#ifndef MAINPROCESS_H
#define MAINPROCESS_H

#include <curl.h>
#include <event.h>
#include <evhttp.h>
#include <event2/thread.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <queue>
#include "ThreadPool/ThreadPool.hpp"
#include "json/json.hpp"
#include "QQ/QQ.h"
#include "LoadPlugin.h"
#include "QQBotLog/QQBotLog.h"

using namespace std;
using json = nlohmann::json;

#define VERSION "0.9.2"
#define STATUS_OK true
#define STATUS_BAD false

class MainProcess {
public:
	MainProcess(int argc, char** argv);
	~MainProcess();
	void Exec();								// 执行事件循环
	string GetPath();							// 找到程序所在的路径
	int ConfigFileInit();						// 如果没有配置文件，创建并初始化配置文件
	int AnalysisConfig();						// 解析配置文件
	int MsgQueueAdd(const string& msg);			// 数据队列的添加/读取
	int MsgQueueGet(string& msg);
	int LoadPlugins();							// 加载插件
	static int LoadDir(const string& dir_path);	// 检查文件夹

	// 这些是可供外部访问的变量，我没有对其进行封装，这方便你进行读取。但这不意味着你可以随意修改。当你在修改时最好知道你在做什么
	vector<char*> argv_;					// 程序参数列表
	string app_path_;						// 程序的绝对地址
	unsigned short bind_port_;				// 程序绑定的HTTP端口
	string cqhttp_addr_;					// cqhttp的HTTP地址(包含端口)
	string access_token_;					// 与cqhttp通信的access token
	ThreadPool* thread_pool_ = nullptr;		// 线程池
	QQBot* bot_ = nullptr;					// QQ机器人对象
	vector<LoadedPlugin*>* plugins_list_ = nullptr;			// 加载的插件列表

private:
	int thread_pool_max_thread_num_;		// 线程池最大线程数
	int thread_pool_max_task_num_;			// 线程池最大任务数
	int thread_pool_adjust_range_;			// 线程池每次增加线程/销毁闲线程的数量，单位个，配置默认是5
	int thread_pool_manager_interval_;		// 线程池管理者线程监视的间隔，单位毫秒，配置默认是2000
	bool status_flag_ = STATUS_OK;			// 状态码，默认为STATUS_OK。在初始化失败时会改成STATUS_BAD，禁止事件循环
	int TPS_;								// tick per second，每秒进行多少次事件循环，配置默认是20
	event_base* ev_thread_base_ = nullptr;
	event_base* ev_main_base_ = nullptr;
	evhttp* ev_http_ = nullptr;
	event* ev_timer_ = nullptr;

	queue<string>* msg_queue_ = nullptr;		// 从cqhttp收到的待处理的数据队列
	mutex msg_queue_mutex_;						// 数据队列锁
};

#endif // !MAINPROCESS_H
