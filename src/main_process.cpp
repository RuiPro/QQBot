#include <unistd.h>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <curl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "main_process.h"
#include "functions.h"
using namespace std;
using json = nlohmann::json;

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
	if (analysisConfig() != 0) exit(0);
	// 初始化数据
	m_thread_pool = new ThreadPool(0, m_thread_pool_max_thread_num, m_thread_pool_max_task_num,
		m_thread_pool_adjust_range, m_thread_pool_manager_interval);
	m_thread_pool->start();
	m_msg_queue = new queue<string>;
	m_plugins_list = new vector<LoadedPlugin*>;
	// 实例化Bot对象
	createBot(m_cqhttp_addr, m_access_token, m_use_cache);
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
	// 加载插件
	if (loadDir(m_app_path + "plugins/") != 0) exit(0);
	if (loadPlugins() != 0) exit(0);
	loger.info() << "Load plugins: " << m_plugins_list->size();
}

MainProcess::~MainProcess() {
	cout << "\033[31m\033[1m" << "\nProgram exiting..." << "\033[0m" << endl;
	event_base_loopbreak(m_ev_thread_base);
	event_base_loopbreak(m_ev_main_base);
	evhttp_free(m_ev_http);
	event_free(m_ev_timer);
	event_base_free(m_ev_main_base);
	event_base_free(m_ev_thread_base);
	if (m_thread_pool != nullptr) delete m_thread_pool;
	if (m_msg_queue != nullptr) delete m_msg_queue;
	if (m_plugins_list != nullptr) {
		for (auto& plugin : *m_plugins_list) {
			delete plugin;
		}
		delete m_plugins_list;
	}
}

void MainProcess::exec() {
	evthread_use_pthreads();
	// 在子线程中执行HTTP请求事件循环，把收到的数据放在数据队列中
	m_ev_thread_base = event_base_new();
	m_ev_http = evhttp_new(m_ev_thread_base);
	evhttp_bind_socket(m_ev_http, "0.0.0.0", m_bind_port);
	evhttp_set_cb(m_ev_http, "/", HTTPRequestCB, this);
	m_thread_pool->addTask(event_base_dispatch, m_ev_thread_base);

	// 在主线程中执数据处理事件循环
	m_ev_main_base = event_base_new();
	while (true) {
		m_ev_timer = evtimer_new(m_ev_main_base, TickEventCB, this);
		struct timeval tv = { 0, 100000 / m_TPS };
		evtimer_add(m_ev_timer, &tv);
		event_base_dispatch(m_ev_main_base);
		event_free(m_ev_timer);
	}
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
	"TPS": 20,
	"use_cache": true,
	"bind_port": 12345,
	"go-cqhttp_IP": "127.0.0.1",
	"go-cqhttp_port": 5700,
	"access_token": "",
	"thread_pool":{
		"max_thread_num": 8,
		"max_task_num": 32,
		"adjust_range": 5,
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
		exit(0);
	}
	try {
		json config;
		config_file >> config;
		if (!config.is_object()) {
			loger.error() << "Cann't get json from config.json.";
			exit(0);
		}
		if (config.find("TPS") == config.end() ||
			config.find("use_cache") == config.end() ||
			config.find("bind_port") == config.end() ||
			config.find("go-cqhttp_IP") == config.end() ||
			config.find("go-cqhttp_port") == config.end() ||
			config.find("thread_pool") == config.end() ||
			config["thread_pool"].find("max_thread_num") == config["thread_pool"].end() ||
			config["thread_pool"].find("max_task_num") == config["thread_pool"].end() ||
			config["thread_pool"].find("adjust_range") == config["thread_pool"].end() ||
			config["thread_pool"].find("manager_interval") == config["thread_pool"].end() ||
			config.find("CorePlugin") == config.end() ||
			config["CorePlugin"].find("auto_add_friend") == config["CorePlugin"].end() ||
			config["CorePlugin"].find("auto_join_group") == config["CorePlugin"].end()) {
			loger.error() << "Required value not found in config.json";
			loger.error() << "You may be able to delete config.json and reconfigure it.";
			return -1;
		}
		m_TPS = config["TPS"];
		m_use_cache = config["use_cache"];
		m_bind_port = config["bind_port"];
		string cqhttp_IP = config["go-cqhttp_IP"];
		unsigned short cqhttp_port = config["go-cqhttp_port"];
		m_thread_pool_max_thread_num = config["thread_pool"]["max_thread_num"];
		m_thread_pool_max_task_num = config["thread_pool"]["max_task_num"];
		m_thread_pool_adjust_range = config["thread_pool"]["adjust_range"];
		m_thread_pool_manager_interval = config["thread_pool"]["manager_interval"];
		m_auto_add_friend = config["CorePlugin"]["auto_add_friend"];
		m_auto_join_group = config["CorePlugin"]["auto_join_group"];
		if (m_TPS <= 0 || cqhttp_IP.empty() || cqhttp_port == 0 || m_bind_port == 0 || m_thread_pool_max_thread_num < 0 ||
			m_thread_pool_max_task_num <= 0 || m_thread_pool_adjust_range <= 0 || m_thread_pool_manager_interval < 0) {
			loger.error() << "Invalid config value found! Please check the config.json.";
			exit(0);
		}
		m_cqhttp_addr = cqhttp_IP + ":" + to_string(cqhttp_port);
		cout << "\033[32m" << "-------- Use HTTP to communicate with go-cqhttp --------" << "\033[0m" << endl;
		cout << "\033[32m" << "> Bind HTTP server port: \033[1m" << m_bind_port << "\033[0m" << endl;
		cout << "\033[32m" << "> The (HTTP) address of go-cqhttp is: \033[1m" << m_cqhttp_addr << "\033[0m" << endl;
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
		return -1;
	}
}

int MainProcess::msgQueueAdd(const string& msg) {
	if (m_msg_queue == nullptr) return -1;
	m_msg_queue_mutex.lock();
	m_msg_queue->push(msg);
	m_msg_queue_mutex.unlock();
	return 0;
}

int MainProcess::msgQueueGet(string& msg) {
	if (m_msg_queue == nullptr) return -1;
	m_msg_queue_mutex.lock();
	if (m_msg_queue->empty()) {
		m_msg_queue_mutex.unlock();
		return -1;
	}
	msg = m_msg_queue->front();
	m_msg_queue->pop();
	m_msg_queue_mutex.unlock();
	return 0;
}

int MainProcess::loadPlugins() {
	// 加载plugins文件夹下的插件
	string plugins_dir_path = m_app_path + "plugins/";
	DIR* dir = opendir(plugins_dir_path.c_str());
	if (dir == nullptr) {
		loger.error() << "Faild to open diretory: " << plugins_dir_path;
		return -1;
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
	return 0;
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
		}
		else {
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
