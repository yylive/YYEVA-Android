//
// Created by zengjiale on 2023/12/15.
//

#ifndef YYEVA_THREAD_H
#define YYEVA_THREAD_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <src/main/cpp/util/ELog.h>

#define TAG_THREAD "Thread"

class Thread {
public:
    Thread(const char *name,std::function<void()> task);

    ~Thread();

    void start();

private:
    static void *run(void *arg); //执行入口

private:
    std::mutex mutex;
    std::condition_variable condition;
    std::function<void()> task;
    bool is_stop;
    bool is_start;
    bool is_created;
    pthread_t id;
    const char *name;
};


#endif //YYEVA_THREAD_H
