#include <unistd.h>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <curl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "json/json.hpp"
#include "MainProcess.h"
#include "Functions.h"
using json = nlohmann::json;

MainProcess::MainProcess(int argc, char** argv) {
	// 取出参数
	for (int i = 0; i < argc; ++i) {
		argv_.push_back(argv[i]);
	}
	// 打印信息
	cout << "\033[36m" << "~ Build by Ruimix ~" << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << std::left << "Using libcurl version: " << setw(20) << curl_version() << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << std::left << "Using libevent version: " << setw(20) << LIBEVENT_VERSION << "\033[0m" << endl;
	cout << "\033[33m" << setw(36) << std::left << "Using json for Modern C++ version: " << setw(20)
		<< std::to_string(NLOHMANN_JSON_VERSION_MAJOR) + "." + std::to_string(NLOHMANN_JSON_VERSION_MINOR) + "." + std::to_string(NLOHMANN_JSON_VERSION_PATCH)
		<< "\033[0m" << endl;
	// 获取程序路径
	app_path_ = GetPath();
	// 检查配置文件是否存在
	if (ConfigFileInit() != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
	// 解析配置文件
	if (AnalysisConfig() != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
	// 初始化数据
	thread_pool_ = new ThreadPool(0, thread_pool_max_thread_num_, thread_pool_max_task_num_,
		thread_pool_adjust_range_, thread_pool_manager_interval_);
	thread_pool_->Start();
	msg_queue_ = new queue<string>;
	plugins_list_ = new vector<LoadedPlugin*>;
	// 实例化QQBot对象
	bot_ = new QQBot(cqhttp_addr_, access_token_);
	// 加载插件
	if (LoadDir(app_path_ + "plugins/") != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
	if (LoadPlugins() != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
	std::cout << "\033[34m[Info]\033[0m\t" << "Load plugins: " << plugins_list_->size() << std::endl;
	// 获取Cqhttp和QQBot对象的信息
	if (bot_->PrintCqhttpVersion() != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
	if (bot_->GetAllinfo() != 0) {
		status_flag_ = STATUS_BAD;
		return;
	}
}

MainProcess::~MainProcess() {
	cout << "\033[31m\033[1m" << "Program exiting..." << "\033[0m" << endl;
	if (status_flag_ != STATUS_BAD) {
		event_base_loopbreak(ev_thread_base_);
		event_base_loopbreak(ev_main_base_);
		evhttp_free(ev_http_);
		event_free(ev_timer_);
		event_base_free(ev_main_base_);
		event_base_free(ev_thread_base_);
	}
	if (thread_pool_ != nullptr) delete thread_pool_;
	if (msg_queue_ != nullptr) delete msg_queue_;
	if (plugins_list_ != nullptr) {
		for (auto& plugin : *plugins_list_) {
			delete plugin;
		}
		delete plugins_list_;
	}
	if (bot_ != nullptr) delete bot_;
}

void MainProcess::Exec() {
	if (status_flag_ == STATUS_BAD) return;
	evthread_use_pthreads();
	// 在子线程中执行HTTP请求事件循环，把收到的数据放在数据队列中
	ev_thread_base_ = event_base_new();
	ev_http_ = evhttp_new(ev_thread_base_);
	evhttp_bind_socket(ev_http_, "0.0.0.0", bind_port_);
	evhttp_set_cb(ev_http_, "/", HTTPRequestCB, this);
	thread_pool_->AddTask(event_base_dispatch, ev_thread_base_);

	// 在主线程中执数据处理事件循环，每秒循环20次
	ev_main_base_ = event_base_new();
	while (true) {
		ev_timer_ = evtimer_new(ev_main_base_, TickEventCB, this);
		struct timeval tv = { 0, 100000 / TPS_ };
		evtimer_add(ev_timer_, &tv);
		event_base_dispatch(ev_main_base_);
		event_free(ev_timer_);
	}
}

string MainProcess::GetPath() {
	char* argv0 = argv_[0];
	string env_path;
	if (argv0[0] == '.' && argv0[1] == '/') {       // 如果程序位置在当前路径下的子路径
		char path[1024]{ 0 };
		if (getcwd(path, 1024) == nullptr) {        // 程序路径 = 当前工作路径 拼接 程序执行路径
			std::cout << "\033[31m[Error]\033[0m\t" << "Failed to get app path!" << std::endl;
			return env_path;
		}
		env_path = path;
		env_path += argv0 + 1;
	}
	else if (argv0[0] == '.' && argv0[1] == '.') {  // 如果程序位置在父路径下的子路径下
		char path1[1024]{ 0 };
		char path2[1024]{ 0 };
		if (getcwd(path1, 1023) == nullptr) {       // 程序路径 = 求绝对路径(当前工作路径 拼接 程序执行路径)
			std::cout << "\033[31m[Error]\033[0m\t" << "Failed to get app path!" << std::endl;
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

int MainProcess::ConfigFileInit() {
	string config_path = app_path_ + "config.json";
	ifstream f;
	f.open(config_path);
	if (!f.is_open()) {
		std::cout << "\033[34m[Info]\033[0m\t" << "File config.json does not exist, creating..." << std::endl;
		ofstream new_file;
		new_file.open(config_path, std::ios::out | std::ios::app);
		if (!new_file.is_open()) {
			std::cout << "\033[31m[Error]\033[0m\t" << "File config.json filed to create!" << std::endl;
			std::cout << "\033[31m" << "Program exiting..." << "\033[0m" << std::endl;
			return -1;
		}
		new_file << R"({
	"TPS": 20,
	"bind_port": 12345,
	"go-cqhttp_IP": "127.0.0.1",
	"go-cqhttp_port": 5700,
	"access_token": "",
	"thread_pool":{
		"max_thread_num": 8,
		"max_task_num": 32,
		"adjust_range": 5,
		"manager_interval": 2000
	}
}
		)";
		new_file.close();
		std::cout << "\033[34m[Info]\033[0m\t" << "File config.json created successfully." << std::endl;
	}
	f.close();
	return 0;
}

int MainProcess::AnalysisConfig() {
	string config_path = app_path_ + "config.json";
	ifstream config_file(config_path);
	if (!config_file.is_open()) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Failed to open file config.json." << std::endl;
		std::cout << "\033[31m" << "Program exiting..." << "\033[0m" << std::endl;
		return -1;
	}
	try {
		json config;
		config_file >> config;
		if (!config.is_object()) {
			std::cout << "\033[31m[Error]\033[0m\t" << "Cann't get json from config.json." << std::endl;
			std::cout << "\033[31m" << "Program exiting..." << "\033[0m" << std::endl;
			return -1;
		}
		if (config.find("TPS") == config.end() ||
			config.find("bind_port") == config.end() ||
			config.find("go-cqhttp_IP") == config.end() ||
			config.find("go-cqhttp_port") == config.end() ||
			config.find("thread_pool") == config.end() ||
			config["thread_pool"].find("max_thread_num") == config["thread_pool"].end() ||
			config["thread_pool"].find("max_task_num") == config["thread_pool"].end() ||
			config["thread_pool"].find("adjust_range") == config["thread_pool"].end() ||
			config["thread_pool"].find("manager_interval") == config["thread_pool"].end()) {
			std::cout << "\033[31m[Error]\033[0m\t" << "Required value not found in config.json" << std::endl;
			std::cout << "\033[31m[Tips]\033[0m\t" << "You may be able to delete config.json and reconfigure it." << std::endl;
			return -1;
		}
		TPS_ = config["TPS"];
		bind_port_ = config["bind_port"];
		string cqhttp_IP = config["go-cqhttp_IP"];
		unsigned short cqhttp_port = config["go-cqhttp_port"];
		thread_pool_max_thread_num_ = config["thread_pool"]["max_thread_num"];
		thread_pool_max_task_num_ = config["thread_pool"]["max_task_num"];
		thread_pool_adjust_range_ = config["thread_pool"]["adjust_range"];
		thread_pool_manager_interval_ = config["thread_pool"]["manager_interval"];
		if (TPS_ <= 0 || cqhttp_IP.empty() || cqhttp_port == 0 || bind_port_ == 0 || thread_pool_max_thread_num_ < 0 ||
			thread_pool_max_task_num_ <= 0 || thread_pool_adjust_range_ <= 0 || thread_pool_manager_interval_ < 0) {
			std::cout << "\033[31m[Error]\033[0m\t" << "Invalid config value found! Please check the config.json." << std::endl;
			std::cout << "\033[31m" << "Program exiting..." << "\033[0m" << std::endl;
			return -1;
		}
		cqhttp_addr_ = cqhttp_IP + ":" + to_string(cqhttp_port);
		cout << "\033[32m" << "-------- Use HTTP to communicate with go-cqhttp --------" << "\033[0m" << endl;
		cout << "\033[32m" << "> Bind HTTP server port: \033[1m" << bind_port_ << "\033[0m" << endl;
		cout << "\033[32m" << "> The (HTTP) address of go-cqhttp is: \033[1m" << cqhttp_addr_ << "\033[0m" << endl;
		if (config.find("access_token") != config.end() && !string(config["access_token"]).empty()) {
			access_token_ = config["access_token"];
			string str(access_token_.size(), '*');
			str[0] = access_token_[0];
			str[access_token_.size() - 1] = access_token_[access_token_.size() - 1];
			cout << "\033[32m" << "> Access token is \033[1m" << str << "\033[0m" << endl;
		}
		else {
			cout << "\033[32m" << "> Access token not found." << "\033[0m" << endl;
		}
		cout << "\033[32m" << "--------------------------------------------------------" << "\033[0m" << endl;
		return 0;
	}
	catch (const std::exception& e) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Failed to analysis config json:" << e.what() << std::endl;
		return -1;
	}
}

int MainProcess::MsgQueueAdd(const string& msg) {
	if (msg_queue_ == nullptr) return -1;
	msg_queue_mutex_.lock();
	msg_queue_->push(msg);
	msg_queue_mutex_.unlock();
	return 0;
}

int MainProcess::MsgQueueGet(string& msg) {
	if (msg_queue_ == nullptr) return -1;
	msg_queue_mutex_.lock();
	if (msg_queue_->empty()) {
		msg_queue_mutex_.unlock();
		return -1;
	}
	msg = msg_queue_->front();
	msg_queue_->pop();
	msg_queue_mutex_.unlock();
	return 0;
}

int MainProcess::LoadPlugins() {
	// 加载plugins文件夹下的插件
	string plugins_dir_path = app_path_ + "plugins/";
	DIR* dir = opendir(plugins_dir_path.c_str());
	if (dir == nullptr) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Faild to open diretory: " << plugins_dir_path << std::endl;
		return -1;
	}
	dirent* file;
	while ((file = readdir(dir)) != NULL) {
		if (file->d_type == DT_REG) {
			string file_name(file->d_name);
			string suffix = ".plg";
			if (file_name.length() <= 4 || file_name.substr(file_name.length() - suffix.length()) != suffix) continue;
			LoadedPlugin* load_plugin = new LoadedPlugin(plugins_dir_path + file_name, bot_, app_path_);
			if (!load_plugin->isGood()) {
				cout << "\033[33m[Warn]\033[0m\t" << "Plugin " << file_name << " faild to load: bad plugin." << endl;
				delete load_plugin;
				continue;
			}
			// 加载插件后，为插件创建一个插件专用的目录
			if (LoadDir(plugins_dir_path + load_plugin->GetName() + "/") != 0) {
				cout << "\033[33m[Warn]\033[0m\t" << "Faild to create diretory for plugin: " << load_plugin->GetName() << endl;
				delete load_plugin;
				continue;
			}
			load_plugin->Loading();
			plugins_list_->push_back(load_plugin);
		}
	}
	closedir(dir);
	return 0;
}

int MainProcess::LoadDir(const string& dir_path) {
	// 判断文件夹是否存在
	struct stat plugins_dir_info;
	// 如果不存在，则创建
	if (stat(dir_path.c_str(), &plugins_dir_info) != 0) {
		if (errno == ENOENT) {
			if (mkdir(dir_path.c_str(), 0755) != 0) {
				std::cout << "\033[31m[Error]\033[0m\t" << "Failed to create diretory: " << dir_path << std::endl;
				return -1;
			}
			return 0;
		}
		else {
			std::cout << "\033[31m[Error]\033[0m\t" << "Failed to check plugins diretory. Error code: " << errno << std::endl;
			return -1;
		}
	}
	// 如果存在，判断是否可读
	if ((plugins_dir_info.st_mode & S_IFDIR) == 0) {
		std::cout << "\033[31m[Error]\033[0m\t" << dir_path << " is not a diretory." << std::endl;
		return -1;
	}
	if (access(dir_path.c_str(), F_OK | R_OK | X_OK) != 0) {
		std::cout << "\033[31m[Error]\033[0m\t" << "Unable to access " << dir_path << ", folder does not exist or permission deny." << std::endl;
		return -1;
	}
	return 0;
}
