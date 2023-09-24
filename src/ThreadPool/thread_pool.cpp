#include "thread_pool.hpp"

ThreadPool::ThreadPool(int min_thread_num, int max_thread_num, int max_task_num, 
    int wave_range/*5*/, int manager_check_interval/*2000*/) {
    if (max_thread_num < min_thread_num || max_thread_num <= 0 ||
        min_thread_num < 0 || max_task_num < 0 || wave_range < 0 || manager_check_interval < 0)
        return;
    this->m_max_thread_num = max_thread_num;
    this->m_min_thread_num = min_thread_num;
    this->m_max_task_num = max_task_num;
    this->m_wave_range = wave_range;
    this->m_manager_check_interval = manager_check_interval;
    m_alive_thread_num = 0;
    m_working_thread_num.store(0);
    m_exit_thread_num.store(0);
    m_state_code = 0;

    do {
        this->m_to_destroy = new std::vector<std::thread::id>;
        if (this->m_to_destroy == nullptr) break;
        this->m_task_queue = new std::queue<std::function<void()>>;
        if (this->m_task_queue == nullptr) break;
        this->m_worker_list = new std::list<WorkerThread*>;
        if (this->m_worker_list == nullptr) break;
        return;
    } while (false);
    delete this->m_to_destroy;
    delete this->m_task_queue;
    delete this->m_worker_list;
}

ThreadPool::~ThreadPool() {
    if (m_state_code != 2) {
        terminate();
    }
    delete this->m_to_destroy;
    delete this->m_task_queue;
    delete this->m_manager;
    delete this->m_worker_list;
}

int ThreadPool::start() {
    if (m_state_code != 0) {
        return -1;
    }
    m_state_code = 1;
    // 创建管理者线程
    this->m_manager = new std::thread(&ThreadPool::manager_func, this);
    // 创建工作线程
    for (int i = 0; i < m_min_thread_num; ++i) {
        m_worker_list->push_back(new WorkerThread(this));
        ++m_alive_thread_num;
    }
    return 0;
}

void ThreadPool::terminate() {
    if (m_state_code == 0) {
        m_state_code = 2;
        return;
    }
    if (m_state_code == 1) {
        // 如果要放弃剩下的任务，清空任务队列
        if (!m_destroy_with_no_task) {
            m_task_queue_mutex.lock();
            delete m_task_queue;
            m_task_queue = new std::queue<std::function<void()>>;
            m_task_queue_mutex.unlock();
        }
        // 阻止继续添加任务
        m_max_task_num = -1;
        // 如果还有线程在工作，则等待
        while (m_working_thread_num.load() != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // 让管理者线程和工作线程退出循环
        m_state_code = 2;
        // 回收管理者线程
        m_manager->join();
        // 唤醒正在睡眠的子线程
        cond.notify_all();
        // 回收工作线程
        for (auto& thread_element : *m_worker_list) {
            delete thread_element;
        }
        return;
    }
}

void ThreadPool::manager_func() {
    // 当线程池开启时一直工作
    while (m_state_code == 1) {
        // 每2秒监视一次
        std::this_thread::sleep_for(std::chrono::milliseconds(m_manager_check_interval));
        //printf("管理者线程监视一次，当前存活线程数量%d个，正在工作线程%d个，任务%zu个\n", alive_thread_num,
        //       working_thread_num.load(), task_queue->size());
        // 线程不够时创建线程：存活数<最大数 并且 所有线程都在工作
        if (m_alive_thread_num < m_max_thread_num &&
            m_alive_thread_num - m_working_thread_num.load() == 0) {
            // 创建的线程数量 = 调整量 或 最大-当前
            int addNum = m_wave_range < m_max_thread_num - m_alive_thread_num ?
                m_wave_range : m_max_thread_num - m_alive_thread_num;
            for (int i = 0; i < addNum; ++i) {
                m_worker_list->push_back(new WorkerThread(this));
                ++m_alive_thread_num;
            }
            continue;
        }

        // 线程过多时销毁线程：存活数>最小数 并且 一些线程没事干在睡眠
        // 让一些线程主动退出
        m_task_queue_mutex.lock();    // 为了防止在清点闲置线程的时候突然来任务干扰，把任务队列锁起来
        if (m_alive_thread_num > m_min_thread_num &&
            m_working_thread_num.load() + m_wave_range < m_alive_thread_num) {
            // 销毁的线程数量 = 调整量 或 当前 - 最小
            int destroyNum = m_wave_range < m_alive_thread_num - m_min_thread_num ?
                m_wave_range : m_alive_thread_num - m_min_thread_num;
            m_exit_thread_num.store(destroyNum);
            m_task_queue_mutex.unlock();
            // 唤醒对应个睡眠的线程
            // 在此情景下，任务队列应该是空的，添加任务的函数那里不会休眠，所以不会被唤醒
            for (int i = 0; i < destroyNum; ++i) {
                cond.notify_one();
            }
            // 等待要被回收的线程就绪
            while (m_exit_thread_num.load() != 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            // 回收主动退出的线程
            m_to_destroy_mutex.lock();
            for (auto& element : *m_to_destroy) {
                for (auto iter = m_worker_list->begin(); iter != m_worker_list->end(); ++iter) {
                    if (element == (*iter)->m_thread_id) {
                        delete (*iter);
                        m_worker_list->erase(iter);
                        --m_alive_thread_num;
                        break;
                    }
                }
            }
            m_to_destroy->clear();
            m_to_destroy_mutex.unlock();
        }
        else {
            m_task_queue_mutex.unlock();
        }
    }
}

void ThreadPool::setSafelyExit(bool arg) {
    if (m_state_code == 0)
        this->m_destroy_with_no_task = arg;
}

void ThreadPool::setReceiveAllTask(bool arg) {
    if (m_state_code == 0)
        this->m_block_task_when_full = arg;
}

int ThreadPool::getMaxThreadNum() const {
    return this->m_max_thread_num;
}
int ThreadPool::getMinThreadNum() const {
    return this->m_min_thread_num;
}
int ThreadPool::getMaxTaskNum() const {
    return this->m_max_task_num;
}

WorkerThread::WorkerThread(ThreadPool* pool) {
    this->m_pool = pool;
    this->m_thread_ptr = new std::thread(&WorkerThread::worker_func, this);
    this->m_thread_id = m_thread_ptr->get_id();
}

WorkerThread::~WorkerThread() {
    this->m_thread_ptr->join();
    delete this->m_thread_ptr;
}

void WorkerThread::worker_func() {
    // 当线程池开启时一直工作
    while (m_pool->m_state_code == 1) {
        // 给任务队列加锁
        std::unique_lock<std::mutex> uniqueLock(m_pool->m_task_queue_mutex);
        // 当任务队列为空时睡眠等待
        while (m_pool->m_task_queue->empty()) {
            m_pool->cond.wait(uniqueLock);
            // 当线程醒来发现需要有线程退出
            if (m_pool->m_exit_thread_num.load() != 0) {
                --m_pool->m_exit_thread_num;
                uniqueLock.unlock();
                // 在要销毁的线程列表中存入自己的线程ID
                m_pool->m_to_destroy_mutex.lock();
                m_pool->m_to_destroy->push_back(this->m_thread_id);
                m_pool->m_to_destroy_mutex.unlock();
                return;
            }
            // 当线程醒来发现要关闭
            if (m_pool->m_state_code != 1) return;
            // 先判断需不需要退出再判断是不是要关闭线程池
            // 防止关闭线程池时管理者还在等待工作线程退出，造成管理者一直阻塞等待
        }
        // 从任务队列取一个任务运行
        std::function<void()> task = m_pool->m_task_queue->front();
        m_pool->m_task_queue->pop();
        // 释放任务队列锁
        uniqueLock.unlock();
        // 唤醒添加任务的函数
        m_pool->cond.notify_one();
        // 工作线程数量加一
        ++m_pool->m_working_thread_num;
        // 执行任务
        task();
        // 执行完任务，工作线程数量减一
        --m_pool->m_working_thread_num;
    }
}