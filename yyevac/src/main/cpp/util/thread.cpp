//
// Created by zengjiale on 2023/12/15.
//

#include "thread.h"
#include <src/main/cpp/player/common/easy.h>

void free_thread(Thread **thread) {
    SAFE_DELETE(*thread);
}

Thread::Thread(const char *name, std::function<void()> task):
    task(task),
    mutex(),
    condition(),
    is_stop(false),
    is_start(false),
    is_created(false),
    name(name),
    id(-1) {
    int ret = pthread_create(&id, nullptr, run, this);
    if (ret == 0 && id > 0) {
        if (strlen(name) > 0) {
            pthread_setname_np(id, name);
        }
        is_created = true;
    }
    ELOGD(TAG_THREAD, "create thread, id:%ld, name:%s, error:%d", id, name, ret);
}

Thread::~Thread() {
    ELOGD(TAG_THREAD, "~Thread, id:%ld, name:%s", id, name);
}

void Thread::start() {
    ELOGD(TAG_THREAD, "start, id:%ld, name:%s", id, name);
    std::unique_lock<std::mutex> lock(mutex);
    if (is_created && !is_start) {
        is_start = true;
        condition.notify_all();
    }
}

void *Thread::run(void *arg) {
    Thread *thread = (Thread*) arg;
    std::function<void()> task;
    std::unique_lock<std::mutex> lock(thread->mutex);
    thread->condition.wait(lock, [thread] {
        ELOGD(TAG_THREAD, "wait start, id:%ld, name:%s", thread->id, thread->name);
        return (thread->is_created && thread->is_start) || thread->is_stop; //等待start
    });

    if (thread->is_stop || !thread->task) {
        pthread_exit(nullptr); //退出线程
    }
    task = std::move(thread->task);
    if (task) {
        ELOGD(TAG_THREAD, "run task, id:%ld, name:%s", thread->id, thread->name);
        task(); //开始执行任务
    }
    thread->is_stop = true;
    ELOGD(TAG_THREAD, "run finished, id:%ld, name:%s", thread->id, thread->name);
    pthread_exit(nullptr); //退出线程
}
