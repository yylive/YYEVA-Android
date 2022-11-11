//
// Created by zengjiale on 2022/6/23.
//

#ifndef YYEVA_THREADPOOL_H
#define YYEVA_THREADPOOL_H

#pragma once
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include "safequeue.h"

class ThreadPool {
private:
    class ThreadWorker {//内置线程工作类

    private:
        int m_id; //工作id

        ThreadPool * m_pool;//所属线程池

    public:
        //构造函数

        ThreadWorker(ThreadPool * pool, const int id)
                : m_pool(pool), m_id(id) {
        }
        //重载`()`操作

        void operator()() {
            std::function<void()> func; //定义基础函数类func

            bool dequeued; //是否正在取出队列中元素

            //判断线程池是否关闭，没有关闭，循环提取

            while (!m_pool->m_shutdown) {
                {
                    //为线程环境锁加锁，互访问工作线程的休眠和唤醒

                    std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
                    //如果任务队列为空，阻塞当前线程

                    if (m_pool->m_queue.empty()) {
                        m_pool->m_conditional_lock.wait(lock); //等待条件变量通知，开启线程

                    }
                    //取出任务队列中的元素

                    dequeued = m_pool->m_queue.dequeue(func);
                }
                //如果成功取出，执行工作函数

                if (dequeued) {
                    func();
                }
            }
        }
    };

    bool m_shutdown; //线程池是否关闭

    SafeQueue<std::function<void()>> m_queue;//执行函数安全队列，即任务队列

    std::vector<std::thread> m_threads; //工作线程队列

    std::mutex m_conditional_mutex;//线程休眠锁互斥变量

    std::condition_variable m_conditional_lock; //线程环境锁，让线程可以处于休眠或者唤醒状态

public:
    //线程池构造函数

    ThreadPool(const int n_threads)
            : m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {
    }

    ThreadPool(const ThreadPool &) = delete; //拷贝构造函数，并且取消默认父类构造函数

    ThreadPool(ThreadPool &&) = delete; // 拷贝构造函数，允许右值引用

    ThreadPool & operator=(const ThreadPool &) = delete; // 赋值操作

    ThreadPool & operator=(ThreadPool &&) = delete; //赋值操作

    // Inits thread pool

    void init() {
        for (int i = 0; i < m_threads.size(); ++i) {
            m_threads[i] = std::thread(ThreadWorker(this, i));//分配工作线程

        }
    }

    // Waits until threads finish their current task and shutdowns the pool

    void shutdown() {
        m_shutdown = true;
        m_conditional_lock.notify_all(); //通知 唤醒所有工作线程

        for (int i = 0; i < m_threads.size(); ++i) {
            if(m_threads[i].joinable()) { //判断线程是否正在等待

                m_threads[i].join();  //将线程加入等待队列

            }
        }
    }

    // Submit a function to be executed asynchronously by the pool
    //线程的主要工作函数，使用了后置返回类型，自动判断函数返回值

    template<typename F, typename...Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Create a function with bounded parameters ready to execute
        //

        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);//连接函数和参数定义，特殊函数类型,避免左、右值错误

        // Encapsulate it into a shared ptr in order to be able to copy construct // assign
        //封装获取任务对象，方便另外一个线程查看结果

        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // Wrap packaged task into void function
        //利用正则表达式，返回一个函数对象

        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };

        // 队列通用安全封包函数，并压入安全队列

        m_queue.enqueue(wrapper_func);

        // 唤醒一个等待中的线程

        m_conditional_lock.notify_one();

        // 返回先前注册的任务指针

        return task_ptr->get_future();
    }
};


#endif //YYEVA_THREADPOOL_H
