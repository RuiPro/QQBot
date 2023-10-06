#ifndef MAIN_PROCESS_H
#define MAIN_PROCESS_H

#include <curl.h>
#include <event.h>
#include <evhttp.h>
#include <event2/thread.h>
#include <sqlite3.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <queue>
#include "ThreadPool/thread_pool.hpp"
#include <json.hpp>
#include "QQ/QQ.h"
#include "load_plugin.h"
#include "Loger/loger.h"
using namespace std;
using json = nlohmann::json;

#define VERSION "1.0.0"

class MainProcess {
public:
	MainProcess(int argc, char** argv);
	MainProcess(const MainProcess&) = delete;
	MainProcess(MainProcess&&) = delete;
	~MainProcess();
	void exec();								// 执行事件循环
	string getPath();							// 找到程序所在的路径
	int configFileInit();						// 如果没有配置文件，创建并初始化配置文件
	int analysisConfig();						// 解析配置文件
	int msgQueueAdd(const string& msg);			// 数据队列的添加/读取
	int msgQueueGet(string& msg);
	int loadPlugins();							// 加载插件
	static int loadDir(const string& dir_path);	// 检查文件夹

	// 这些是可供外部访问的变量，我没有对其进行封装，这方便你进行读取。但这不意味着可以随意修改。当你在修改时最好知道你在做什么
	vector<char*> m_argv;						// 程序参数列表
	string m_app_path;							// 程序的绝对地址
	unsigned short m_bind_port;					// 程序绑定的HTTP端口
	string m_cqhttp_addr;						// cqhttp的HTTP地址(包含端口)
	string m_access_token;						// 与cqhttp通信的access token
	ThreadPool* m_thread_pool = nullptr;		// 线程池
	vector<LoadedPlugin*>* m_plugins_list = nullptr;			// 加载的插件列表
	bool m_auto_add_friend;
	bool m_auto_join_group;

private:
	int m_thread_pool_max_thread_num;		// 线程池最大线程数
	int m_thread_pool_max_task_num;			// 线程池最大任务数
	int m_thread_pool_adjust_range;			// 线程池每次增加线程/销毁闲线程的数量，单位个，配置默认是5
	int m_thread_pool_manager_interval;		// 线程池管理者线程监视的间隔，单位毫秒，配置默认是2000
	bool m_use_cache;						// 是否使用go-cqhttp的缓存
	int m_TPS;								// tick per second，每秒进行多少次事件循环，配置默认是20
	event_base* m_ev_thread_base = nullptr;
	event_base* m_ev_main_base = nullptr;
	evhttp* m_ev_http = nullptr;
	event* m_ev_timer = nullptr;

	queue<string>* m_msg_queue = nullptr;		// 从cqhttp收到的待处理的数据队列
	mutex m_msg_queue_mutex;					// 数据队列锁
};

#endif // !MAINPROCESS_H
