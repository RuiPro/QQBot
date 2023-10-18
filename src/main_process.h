#ifndef MAIN_PROCESS_H
#define MAIN_PROCESS_H

#include <string>
#include <vector>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <sys/time.h>
class ThreadPool;
class LoadedPlugin;
using namespace std;

struct TimeVal {
	unsigned long second = 0;
	unsigned long microsecond = 0;
	operator timeval () const {
		timeval ret;
		ret.tv_sec = second;
		ret.tv_usec = microsecond;
		return ret;
	}
};

// 存放libevent的相关数据
struct EventData;
struct event;

#ifndef evutil_socket_t
#define evutil_socket_t int
#endif // !evutil_socket_t

class MainProcess {
	friend void HTTPRequestCB(struct evhttp_request* req, void* cb_arg);
	friend void TimerEventCB(evutil_socket_t fd, short event_t, void* cb_arg);
public:
	MainProcess() = delete;
	MainProcess(const MainProcess&) = delete;
	MainProcess(MainProcess&&) = delete;
	MainProcess& operator=(const MainProcess&) = delete;
	~MainProcess();

	void loadPlugins();							// 加载插件
	void exec();								// 执行事件循环

	// 添加定时器任务，成功返回事件指针，失败返回nullptr
	event* addTimerTask(const TimeVal& time, const std::function<void()>& task);
	int resetTimerTask(event* timer_ev, const timeval& tv);
	// 取消定时器任务&释放定时器资源
	int deleteTimerTask(event* timer_ev);

	static void initMainProcessObj(int argc, char** argv) {
		static once_flag init_of;
		call_once(init_of, [&]() { sm_process = new MainProcess(argc, argv); });
	}
	static MainProcess* getMainProcessObj() {
		return sm_process;
	}

private:
	bool is_running = false;
	vector<char*> m_argv;						// 程序参数列表
	string m_app_path;							// 程序的绝对地址
	unsigned short m_bind_port;					// 程序绑定的HTTP端口
	string m_cqhttp_addr;						// cqhttp的HTTP地址(包含端口)
	string m_access_token;						// 与cqhttp通信的access token
	ThreadPool* m_thread_pool = nullptr;		// 线程池
	bool m_auto_add_friend;						// 是否自动加好友
	bool m_auto_join_group;						// 是否自动加群
	int m_thread_pool_max_thread_num;			// 线程池最大线程数
	int m_thread_pool_max_task_num;				// 线程池最大任务数
	int m_thread_pool_adjust_range;				// 线程池每次增加线程/销毁闲线程的数量，单位个，配置默认是5
	int m_thread_pool_manager_interval;			// 线程池管理者线程监视的间隔，单位毫秒，配置默认是2000
	bool m_use_cache;							// 是否使用go-cqhttp的缓存
	vector<LoadedPlugin*>* m_plugins_list = nullptr;			// 加载的插件列表
	mutex m_msg_queue_mutex;					// 消息队列锁
	condition_variable m_msg_queue_cv;			// 消息队列条件变量
	queue<string>* m_msg_queue = nullptr;		// 从cqhttp收到的待处理的消息队列
	EventData* m_event_data = nullptr;			// 与libevent相关的成员

	// 单例模式-主程序构造
	MainProcess(int argc, char** argv);
	static MainProcess * sm_process;

	string getPath();							// 找到程序所在的路径
	int configFileInit();						// 如果没有配置文件，创建并初始化配置文件
	int analysisConfig();						// 解析配置文件
	int msgQueueAdd(const string& msg);			// 数据队列的添加/读取/判空
	int msgQueueAdd(string&& msg);
	int msgQueueGet(string& msg);
	bool msgQueueIsEmpty();

	// 内置的Bot逻辑，比如自动加好友
	void corePlugin(const string& msg);
	// 用于消费消息队列中的信息，并分发给每个插件
	void handOutMsg();

	static int loadDir(const string& dir_path);	// 检查文件夹
};

#define MainProc (*MainProcess::getMainProcessObj())
#define MainProcPtr MainProcess::getMainProcessObj()

#endif // !MAINPROCESS_H
