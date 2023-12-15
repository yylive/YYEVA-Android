//
// Created by zengjiale on 2023/12/15.
//

#include "thread.h"

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
    return nullptr;
}
