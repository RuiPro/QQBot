#include <curl.h>
#include <event.h>
#include <evhttp.h>
#include <event2/thread.h>
#include <sqlite3.h>
#include <json.hpp>

#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <chrono>
#include <dirent.h>
#include <sys/stat.h>
#include "thread_pool.hpp"
#include "QQ/QQ.h"
#include "load_plugin.h"
#include "loger.hpp"
using namespace std;
using json = nlohmann::json;
#include "main_process.h"

MainProcess* MainProcess::sm_process;

struct EventData {
	event_base* m_http_base = nullptr;		// http io事件树
	event_base* m_timer_base = nullptr;		// 计时器事件树
	evhttp* m_ev_httpd = nullptr;			// http监测事件
	map<event*, std::function<void()>>* m_timer_task_set = nullptr;	// 保存定时器任务的哈希表
	mutex m_timer_task_set_mutex;			// 定时器任务哈希表锁
};

void HTTPRequestCB(struct evhttp_request* req, void* cb_arg) {
	try {
		auto startTime = chrono::system_clock::now();
		MainProcess* arg = (MainProcess*)cb_arg;
		if (evhttp_request_get_command(req) == EVHTTP_REQ_GET) {
			evbuffer* send_buffer = evhttp_request_get_output_buffer(req);
			evbuffer_add_printf(send_buffer, "{}");
			evhttp_send_reply(req, HTTP_OK, "OK", send_buffer);
			evbuffer_free(send_buffer);
		}
		else {
			evkeyvalq* headers = evhttp_request_get_input_headers(req);
			ssize_t Content_Length = stoi(evhttp_find_header(headers, "Content-Length"));
			evbuffer* recv_buffer = evhttp_request_get_input_buffer(req);
			const ssize_t recv_size = evbuffer_get_length(recv_buffer);
			char* recv_data = new char[recv_size + 1] {};
			ssize_t counter = 0;
			while (recv_size > counter) {
				ssize_t once_size = evbuffer_remove(recv_buffer, recv_data, recv_size);
				if (once_size == -1) {
					loger.error() << "Failed to get data from input buffer!";
					return;
				}
				else {
					counter += once_size;
				}
			}
			evbuffer* send_buffer = evhttp_request_get_output_buffer(req);
			evbuffer_add_printf(send_buffer, "{}");
			evhttp_add_header(req->output_headers, "Content-Type", "application/json");
			evhttp_send_reply(req, HTTP_OK, "OK", send_buffer);
			// evbuffer_free(send_buffer);	无需手动释放，evhttp_send_reply已经自动释放了

			if (Content_Length == recv_size) {
				// 处理数据入口
				json data = json::parse(recv_data);
				do {
					if (data["self_id"] != QQBot.getThisBotID()) {
						loger.warn() << "Received messages from other QQ bots, ignored.";
						break;
					}
					if (data["self_id"] == "meta_event" && data["meta_event_type"] == "heartbeat") {
						break;
					}
					unique_lock<mutex> locker(arg->m_msg_queue_mutex);
					arg->msgQueueAdd(std::move(string(recv_data)));
					locker.unlock();
					arg->m_msg_queue_cv.notify_one();
				} while (false);
			}
			else {
				loger.warn() << "The size of data received doesn't match client report.";
			}
			delete[] recv_data;
			chrono::duration<double> runTime = chrono::system_clock::now() - startTime;
			auto runTimeCount = runTime.count();
			if (runTimeCount > 4.75) {
				loger.warn() << "HTTPRequestCB run time: " << runTimeCount << " is longer than 5s.";
			}
		}
	}
	catch (...) {
		loger.error() << "Libevent's evhttp callback function HTTPRequestCB Exception.";
	}
}
void TimerEventCB(evutil_socket_t fd, short event_t, void* cb_arg) {
	event* ev = *(static_cast<event**>(cb_arg));
	if (cb_arg != nullptr) delete static_cast<event**>(cb_arg);
	auto iter = MainProc.m_event_data->m_timer_task_set->find(ev);
	if (iter != MainProc.m_event_data->m_timer_task_set->end()) {
		MainProc.m_thread_pool->addTask(iter->second);
		unique_lock<mutex> locker(MainProc.m_event_data->m_timer_task_set_mutex);
		MainProc.m_event_data->m_timer_task_set->erase(ev);
	}
}

event* MainProcess::addTimerTask(const TimeVal& time, const std::function<void()>& task) {
	if (!is_running) return nullptr;
	timeval tv = time;
	event* tev = nullptr;
	if (tv.tv_sec == 0 && tv.tv_usec == 0) return tev;
	event** tev_ptr = new event*(nullptr);
	tev = evtimer_new(m_event_data->m_timer_base, TimerEventCB, tev_ptr);
	if (tev == nullptr) return tev;
	if (evtimer_add(tev, &tv) == -1) {
		loger.error() << "Add timer task error.";
		return nullptr;
	}
	*tev_ptr = tev;
	unique_lock<mutex> locker(m_event_data->m_timer_task_set_mutex);
	if (!is_running) {
		evtimer_del(tev);
		delete tev_ptr;
		return nullptr;
	}
	m_event_data->m_timer_task_set->insert(std::move(std::make_pair(tev, task)));
	return tev;
}
int MainProcess::resetTimerTask(event* timer_ev, const timeval& tv) {
	if (timer_ev == nullptr) return -1;
	if (tv.tv_sec == 0 && tv.tv_usec == 0) return -1;
	unique_lock<mutex> locker(m_event_data->m_timer_task_set_mutex);
	if (m_event_data->m_timer_task_set->find(timer_ev) != m_event_data->m_timer_task_set->end()) {
		event_del(timer_ev);
		evtimer_add(timer_ev, &tv);
	}
	else {
		return -1;
	}
	return 0;
}
int MainProcess::deleteTimerTask(event* timer_ev) {
	if (timer_ev == nullptr) return -1;
	unique_lock<mutex> locker(m_event_data->m_timer_task_set_mutex);
	if (m_event_data->m_timer_task_set->find(timer_ev) != m_event_data->m_timer_task_set->end()) {
		m_event_data->m_timer_task_set->erase(timer_ev);
		event_free(timer_ev);
		timer_ev = nullptr;
	}
	else {
		return -1;
	}
	return 0;
}

MainProcess::MainProcess(int argc, char** argv) {
	// 取出参数
	for (int i = 0; i < argc; ++i) {
		m_argv.push_back(argv[i]);
	}
	// 打印信息
	cout << "\033[36m" << "~ Build by Ruimix ~" << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << left << "Using libcurl version: " << setw(20) << curl_version() << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << left << "Using libevent version: " << setw(20) << LIBEVENT_VERSION << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << left << "Using json for Modern C++ version: " << setw(20)
		<< to_string(NLOHMANN_JSON_VERSION_MAJOR) + "." + to_string(NLOHMANN_JSON_VERSION_MINOR) + "." + to_string(NLOHMANN_JSON_VERSION_PATCH)
		<< "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << left << "Using SQLite3 version: " << setw(20) << sqlite3_version << "\033[0m" << endl;

	// 获取程序路径
	m_app_path = getPath();
	if (m_app_path.empty()) exit(0);
	// 检查配置文件是否存在
	if (configFileInit() != 0) exit(0);
	// 解析配置文件
	loger.info() << "Checking config file...";
	if (analysisConfig() != 0) exit(0);
	// 初始化数据
	loger.info() << "Initialize ThreadPool...";
	m_thread_pool = new ThreadPool(1, m_thread_pool_max_thread_num, m_thread_pool_max_task_num,
		m_thread_pool_adjust_range, m_thread_pool_manager_interval);
	m_thread_pool->start();
	loger.info() << "Initialize data...";
	m_msg_queue = new queue<string>;
	m_plugins_list = new vector<LoadedPlugin*>;
	m_event_data = new EventData;
	m_event_data->m_timer_task_set = new map<event*, std::function<void()>>;
	evthread_use_pthreads();
	m_event_data->m_http_base = event_base_new();
	m_event_data->m_ev_httpd = evhttp_new(m_event_data->m_http_base);
	m_event_data->m_timer_base = event_base_new();
	// 实例化Bot对象
	ThisBot::createBot(m_onebot_addr, m_access_token, m_use_cache);
	// 获取Cqhttp和Bot对象的信息并输出
	loger.info() << "Using go-cqhttp version: " << QQBot.queryCqhttpVersion();
	if (QQBot.fetchThisBotBasicInfo() != 0) exit(0);
	loger.info() << "ThisBot has logged in with " << QQBot.getThisBotNickname() << "(" << QQBot.getThisBotID() << ")";
	if (QQBot.fetchThisBotFriendList() != 0) exit(0);
	QQBot.printFriendList();
	if (QQBot.fetchThisBotUFriendList() != 0) exit(0);
	QQBot.printUFriendList();
	if (QQBot.fetchThisBotGroupList() != 0) exit(0);
	QQBot.printGroupList();
	vector<unsigned int> group_id_list = std::move(QQBot.getThisBotGroupIDList());
	for (auto& element : group_id_list) {
		QQBot.fetchThisBotGroupMemberList(element);
	}
	for (auto& element : m_admin_list) {
		QQBot.setAddThisBotAdmin(element);
	}
	loger.info() << "Add " << m_admin_list.size() << " administrator to ThisBot.";
}
MainProcess::~MainProcess() {
	loger.info() << "\033[31m\033[1mProgram exiting...\033[0m";
	is_running = false;
	loger.info() << "\033[31m\033[1mExiting event loop...\033[0m";
	if (event_base_loopbreak(m_event_data->m_http_base) != 0) {
		loger.error() << "\033[31m\033[1mExiting event http loop falid.\033[0m";
	}
	if (event_base_loopbreak(m_event_data->m_timer_base) != 0) {
		loger.error() << "\033[31m\033[1mExiting event timer loop falid.\033[0m";
	}
	evhttp_free(m_event_data->m_ev_httpd);
	event_base_free(m_event_data->m_http_base);
	event_base_free(m_event_data->m_timer_base);
	if (m_event_data->m_timer_task_set != nullptr) {
		unique_lock<mutex> locker(m_event_data->m_timer_task_set_mutex);
		for (auto iter = m_event_data->m_timer_task_set->begin(); iter != m_event_data->m_timer_task_set->end(); iter++) {
			if (iter->first != nullptr) {
				event_del(iter->first);
			}
		}
		delete m_event_data->m_timer_task_set;
	}
	loger.info() << "\033[31m\033[1mClosing ThreadPool...\033[0m";
	std::queue<string> empty;
	m_msg_queue->swap(empty);
	m_msg_queue_cv.notify_all();
	if (m_thread_pool != nullptr) delete m_thread_pool;
	loger.info() << "\033[31m\033[1mRelease resources...\033[0m";
	if (m_msg_queue != nullptr) delete m_msg_queue;
	if (m_event_data != nullptr) delete m_event_data;
	loger.info() << "\033[31m\033[1mUninstalling plugins...\033[0m";
	if (m_plugins_list != nullptr) {
		for (auto& plugin : *m_plugins_list) {
			delete plugin;
		}
		delete m_plugins_list;
	}
}
void MainProcess::exec() {
	if (is_running) return;
	is_running = true;
	// 在子线程中执行消息分发
	m_thread_pool->addTask(&MainProcess::handOutMsg, this);
	// 在子线程中执行HTTP请求事件循环，把收到的数据放在数据队列中
	evhttp_bind_socket(m_event_data->m_ev_httpd, m_bind_addr.c_str(), m_bind_port);
	evhttp_set_cb(m_event_data->m_ev_httpd, "/", HTTPRequestCB, this);
	m_thread_pool->addTask(event_base_dispatch, m_event_data->m_http_base);
	loger.info() << "\033[36mBot is now running!\033[0m";
	// 在主线程中执数据处理计时器事件
	event_base_loop(m_event_data->m_timer_base, EVLOOP_NO_EXIT_ON_EMPTY);
}
void MainProcess::loadPlugins() {
	loger.info() << "Loading plugins...";
	if (loadDir(m_app_path + "plugins/") != 0) {
		loger.error() << "Load plugins error: falid to load plugin dir.";
		exit(0);
	}
	// 加载plugins文件夹下的插件
	string plugins_dir_path = m_app_path + "plugins/";
	DIR* dir = opendir(plugins_dir_path.c_str());
	if (dir == nullptr) {
		loger.error() << "Faild to open diretory: " << plugins_dir_path;
		exit(0);
	}
	dirent* file;
	while ((file = readdir(dir)) != NULL) {
		if (file->d_type == DT_REG) {
			string file_name(file->d_name);
			string suffix = ".plg";
			if (file_name.length() <= 4 || file_name.substr(file_name.length() - suffix.length()) != suffix) continue;
			LoadedPlugin* load_plugin = new LoadedPlugin(plugins_dir_path + file_name, m_app_path);
			if (!load_plugin->isGood()) {
				loger.warn() << "Plugin " << file_name << " faild to load: bad plugin.";
				delete load_plugin;
				continue;
			}
			loger.info() << "Loading plugin " << load_plugin->getPluginName() << " " << load_plugin->getPluginVersion() << "...";
			// 加载插件后，为插件创建一个插件专用的目录
			if (loadDir(plugins_dir_path + load_plugin->getPluginName() + "/") != 0) {
				loger.warn() << "Faild to create diretory for plugin: " << load_plugin->getPluginName();
				delete load_plugin;
				continue;
			}
			load_plugin->loading();
			m_plugins_list->push_back(load_plugin);
		}
	}
	closedir(dir);
	loger.info() << "Load plugins: " << m_plugins_list->size();
	return;
}

string MainProcess::getPath() {
	char* argv0 = m_argv[0];
	string env_path;
	if (argv0[0] == '.' && argv0[1] == '/') {       // 如果程序位置在当前路径下的子路径
		char path[1024]{ 0 };
		if (getcwd(path, 1024) == nullptr) {        // 程序路径 = 当前工作路径 拼接 程序执行路径
			loger.error() << "Failed to get app path!";
			return env_path;
		}
		env_path = path;
		env_path += argv0 + 1;
	}
	else if (argv0[0] == '.' && argv0[1] == '.') {  // 如果程序位置在父路径下的子路径下
		char path1[1024]{ 0 };
		char path2[1024]{ 0 };
		if (getcwd(path1, 1023) == nullptr) {       // 程序路径 = 求绝对路径(当前工作路径 拼接 程序执行路径)
			loger.error() << "Failed to get app path!";
			return env_path;
		}
		env_path = path1;
		env_path.push_back('/');
		env_path += argv0;
		realpath(env_path.c_str(), path2);
		env_path.clear();
		env_path = path2;
	}
	else {                                          // 如果程序位置是绝对路径，直接转换
		env_path = argv0;
	}
	while (env_path.back() != '/') {                 // 去掉程序名称，得到程序所在的目录
		env_path.pop_back();
	}
	return env_path;
}
int MainProcess::configFileInit() {
	string config_path = m_app_path + "config.json";
	ifstream f;
	f.open(config_path);
	if (!f.is_open()) {
		loger.info() << "File config.json does not exist, creating...";
		ofstream new_file;
		new_file.open(config_path, ios::out | ios::app);
		if (!new_file.is_open()) {
			loger.error() << "File config.json filed to create!";
			exit(0);
		}
		new_file << R"({
	"bind_addr": "0.0.0.0",
	"bind_port": 12345,
	"OneBot_addr": "127.0.0.1",
	"OneBot_port": 5700,
	"use_cache": true,
	"access_token": "",
	"admin_list": [],
	"thread_pool":{
		"max_thread_num": 64,
		"max_task_num": 512,
		"adjust_range": 2,
		"manager_interval": 2000
	},
	"CorePlugin":{
		"auto_add_friend": true,
		"auto_join_group": true
	}
}
		)";
		new_file.close();
		loger.info() << "File config.json created successfully.";
	}
	f.close();
	return 0;
}
int MainProcess::analysisConfig() {
	string config_path = m_app_path + "config.json";
	ifstream config_file(config_path);
	if (!config_file.is_open()) {
		loger.error() << "Failed to open file config.json.";
		return -1;
	}
	try {
		json config;
		config_file >> config;
		if (!config.is_object()) {
			loger.error() << "Cann't get json from config.json.";
			return -1;
		}
		m_bind_addr = config["bind_addr"];
		m_bind_port = config["bind_port"];
		m_onebot_addr = config["OneBot_addr"].get<string>() + ":" + to_string(config["OneBot_port"]);
		m_use_cache = config["use_cache"];
		for (auto& element : config["admin_list"]) {
			m_admin_list.push_back(element);
		}
		m_thread_pool_max_thread_num = config["thread_pool"]["max_thread_num"];
		m_thread_pool_max_task_num = config["thread_pool"]["max_task_num"];
		m_thread_pool_adjust_range = config["thread_pool"]["adjust_range"];
		m_thread_pool_manager_interval = config["thread_pool"]["manager_interval"];
		m_auto_add_friend = config["CorePlugin"]["auto_add_friend"];
		m_auto_join_group = config["CorePlugin"]["auto_join_group"];

		cout << "\033[32m" << "-------- Use HTTP to communicate with go-cqhttp --------" << "\033[0m" << endl;
		cout << "\033[32m" << "> Bind HTTP server addr: \033[1m" << m_bind_addr << ":" << m_bind_port << "\033[0m" << endl;
		cout << "\033[32m" << "> The (HTTP) address of OneBot is: \033[1m" << m_onebot_addr << "\033[0m" << endl;
		if (config.find("access_token") != config.end() && !string(config["access_token"]).empty()) {
			m_access_token = config["access_token"];
			string str(m_access_token.size(), '*');
			str[0] = m_access_token[0];
			str[m_access_token.size() - 1] = m_access_token[m_access_token.size() - 1];
			cout << "\033[32m" << "> Access token is \033[1m" << str << "\033[0m" << endl;
		}
		else {
			cout << "\033[32m" << "> Access token not found." << "\033[0m" << endl;
		}
		cout << "\033[32m" << "--------------------------------------------------------" << "\033[0m" << endl;
		return 0;
	}
	catch (const exception& e) {
		loger.error() << "Failed to analysis config json:" << e.what();
		loger.error() << "You may be able to delete config.json and reconfigure it.";
		return -1;
	}
}
int MainProcess::msgQueueAdd(const string& msg) {
	if (m_msg_queue == nullptr) return -1;
	m_msg_queue->push(msg);
	return 0;
}
int MainProcess::msgQueueAdd(string&& msg) {
	if (m_msg_queue == nullptr) return -1;
	m_msg_queue->push(std::move(msg));
	return 0;
}
int MainProcess::msgQueueGet(string& msg) {
	if (m_msg_queue == nullptr) return -1;
	if (m_msg_queue->empty()) {
		return -1;
	}
	msg = m_msg_queue->front();
	m_msg_queue->pop();
	return 0;
}
bool MainProcess::msgQueueIsEmpty() {
	return m_msg_queue->empty();
}
void MainProcess::corePlugin(const string& msg) {
	try {
		json QQevent = json::parse(msg, NULL, false);
		if (QQevent["post_type"] == "notice") {
			// 群变动时更新群列表，包括群内人数改变（包括Bot加群退群）
			if (QQevent["notice_type"] == "group_decrease" || QQevent["notice_type"] == "group_increase") {
				if (QQevent["user_id"] == QQBot.getThisBotID()) {	// 如果是Bot加群退群，更新bot群列表
					loger.info() << "QQBot group list changed.";
					QQBot.fetchThisBotGroupList();
					QQBot.printGroupList();
				}
				else {
					if (QQBot.fetchThisBotGroupMemberList(QQevent["group_id"]) == 0) {	// 如果是其他群，更新对应群即可
						loger.info() << "QQBot group " << QQevent["group_id"] << " updated.";
					}
				}
			}
			// 好友变动时更新好友列表
			if (QQevent["notice_type"] == "friend_add") {
				loger.info() << "QQBot friend list changed.";
				QQBot.fetchThisBotFriendList();
				QQBot.fetchThisBotUFriendList();
				QQBot.printFriendList();
				QQBot.printUFriendList();
			}
			// 群成员信息变动
			if (QQevent["notice_type"] == "group_admin") {
				if (QQBot.fetchThisBotGroupMemberList(QQevent["group_id"]) == 0) {
					loger.info() << "Group admin in group " << QQevent["group_id"] << " updated.";
				}
			}
			// 群内名片、头衔改变
			if (QQevent["notice_type"] == "group_card") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated.";
				}
				else {
					loger.warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!";
				}
			}
			if (QQevent["notice_type"] == "notify" && QQevent["sub_type"] == "title") {
				if (QQBot.fetchThisBotGroupMemberInfo(QQevent["group_id"], QQevent["user_id"]) == 0) {
					loger.info() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information updated.";
				}
				else {
					loger.warn() << "Member " << QQevent["user_id"] << " in group " << QQevent["group_id"] << " information update failed!";
				}
			}
		}
		// 加群、加好友
		if (QQevent["post_type"] == "request") {
			if (QQevent["request_type"] == "friend") {
				loger.info() << QQevent["user_id"] << " requests to add bot as a friend: " << QQevent["comment"];
				if (m_auto_add_friend) {
					QQBot.applyAddFriendRequest(QQevent["flag"], true, "");
					loger.info() << "Automatically added friend " << QQevent["user_id"];
				}
			}
			if (QQevent["request_type"] == "group" && QQevent["sub_type"] == "invite") {
				loger.info() << "bot is invited to join group " << QQevent["group_id"];
				if (m_auto_join_group) {
					QQBot.applyAddGroupRequest(QQevent["flag"], QQevent["sub_type"], true, "");
					loger.info() << "Automatically join group " << QQevent["group_id"];
				}
			}
		}
	}
	catch (...) {
		loger.error() << "corePlugin Exception.";
	}
}
void MainProcess::handOutMsg() {
	while (true) {
		unique_lock<mutex> locker(m_msg_queue_mutex);
		while (msgQueueIsEmpty()) {
			m_msg_queue_cv.wait(locker);
			if (!is_running) {
				return;
			}
		}
		string a_msg;
		if (msgQueueGet(a_msg) != 0) return;
		m_thread_pool->addTask(&MainProcess::corePlugin, this, a_msg);
		for (auto& plugin : *m_plugins_list) {
			m_thread_pool->addTask((void(BasicPlugin::*)(const string&)) & BasicPlugin::pluginMain, plugin->getBasicPlugin(), a_msg);
		}
	}
}

int MainProcess::loadDir(const string& dir_path) {
	// 判断文件夹是否存在
	struct stat plugins_dir_info;
	// 如果不存在，则创建
	if (stat(dir_path.c_str(), &plugins_dir_info) != 0) {
		if (errno == ENOENT) {
			if (mkdir(dir_path.c_str(), 0755) != 0) {
				loger.error() << "Failed to create diretory: " << dir_path;
				return -1;
			}
			return 0;
		} else {
			loger.error() << "Failed to check plugins diretory. Error code: " << errno;
			return -1;
		}
	}
	// 如果存在，判断是否可读
	if ((plugins_dir_info.st_mode & S_IFDIR) == 0) {
		loger.error() << dir_path << " is not a diretory.";
		return -1;
	}
	if (access(dir_path.c_str(), F_OK | R_OK | X_OK) != 0) {
		loger.error() << "Unable to access " << dir_path << ", folder does not exist or permission deny.";
		return -1;
	}
	return 0;
}