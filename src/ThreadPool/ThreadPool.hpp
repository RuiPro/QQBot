#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <list>
#include <vector>
#include <functional>
#include <chrono>

class WorkerThread;

class ThreadPool {
    friend WorkerThread;
public:
    ThreadPool(int min_thread_num, int max_thread_num, int max_task_num,
        int wave_range, int manager_check_interval);               // 构造
    ThreadPool(const ThreadPool&) = delete;
    ~ThreadPool();                                                            // 析构
    template<class F, class... Args>
    int AddTask(F func, Args... args);                                        // 【模板】添加任务
    int Start();                                                              // 开始运作
    void Terminate();                                                         // 结束运作
    void SafelyExit(bool);                                                    // 设置强制结束
    void ReceiveAllTask(bool);                                                // 设置队列满时阻塞添加任务
    int GetMaxThreadNum() const;
    int GetMinThreadNum() const;
    int GetMaxTaskNum() const;

private:
    int max_thread_num;                         // 线程池内能达到的最大线程数量
    int min_thread_num;                         // 线程池内至少存在的线程数量
    int max_task_num;                           // 线程池存储的最大任务数量，0代表无上限
    int wave_range;                             // 每次调整加/减多少线程
    int manager_check_interval;                 // 管理者线程每隔多少毫秒检查一次
    int alive_thread_num;                       // 存活的线程数，只有管理者线程修改，不需加锁
    std::atomic<int> working_thread_num{};      // 正在工作的线程数
    std::atomic<int> exit_thread_num{};         // 准备退出的线程数
    std::queue<std::function<void()>> *task_queue = nullptr;    // 任务队列
    std::mutex task_queue_mutex;                // 锁任务队列
    std::list<WorkerThread *> *worker_list = nullptr; // 存放线程的链表
    std::thread *manager = nullptr;             // 管理者线程
    std::condition_variable cond;               // 任务条件变量
    std::vector<std::thread::id> *to_destroy = nullptr;          // 存放需要进行销毁的线程ID
    std::mutex to_destroy_mutex;                // 锁销毁队列
    short state_code;                           // 状态码：0创建但未运行 1正在运行 2结束准备销毁
    bool destroy_with_no_task = true;           // 是否在销毁线程池之前执行完任务队列中剩下的任务，默认是
    bool block_task_when_full = true;           // 在任务队列满的时候是否阻塞添加任务函数，默认是
    // 如果选择否，那么在任务队列满的时候添加任务会返回-1

    void manager_func();                        // 管理者线程函数
};

// 把工作线程包装成一个类，类有构造和析构可以用于创建和销毁
class WorkerThread {
    friend ThreadPool;
    std::thread::id thread_id;              // 线程自己的线程ID
    std::thread *thread_ptr = nullptr;     // 自己要做的事
    ThreadPool *pool = nullptr;            // 自己属于哪个线程池

    void worker_func();
    explicit WorkerThread(ThreadPool *);
    ~WorkerThread();
};

template<class F, class... Args>
int ThreadPool::AddTask(F func, Args... args) {
    // 线程池还没开启的时候不可以添加任务
    if (state_code != 1) return -1;
    std::unique_lock<std::mutex> uniqueLock(task_queue_mutex);
    if (block_task_when_full) {
        // 满的时候休眠
        while (task_queue->size() >= max_task_num) {
            cond.wait(uniqueLock);
            // 当被唤醒之后发现不允许添加任务立即返回
            if (max_task_num == -1) {
                uniqueLock.unlock();
                return -1;
            }
        }
    }
    else {
        // 当不允许添加任务立即返回
        if (max_task_num == -1) {
            uniqueLock.unlock();
            return -1;
        }
        // 当max_task_num不为0并且任务数量抵达上限时返回-1
        if (max_task_num != 0 && task_queue->size() >= max_task_num) {
            uniqueLock.unlock();
            return -1;
        }
    }
    task_queue->push(std::bind(func, args...));
    uniqueLock.unlock();

    cond.notify_all();
    return 0;
}
#endif
