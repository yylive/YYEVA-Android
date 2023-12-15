//
// Created by zengjiale on 2023/12/14.
//

#ifndef YYEVA_SAFEQUEUE_H
#define YYEVA_SAFEQUEUE_H

#pragma once

#include <mutex>
#include <queue>

using namespace std;
template<typename T>
class SafeQueue {
public:
    int size() {
        unique_lock<mutex> lock(m_mutex); //互斥信号变量加锁，防止m_queue被改变
        return m_queue.size();
    }

    void push(T &t) {
        unique_lock<mutex> lock(m_mutex);
        m_queue.push(t);
    }

    bool peek(T &t) {
        unique_lock<mutex> lock(m_mutex); //队列加锁
        if (m_queue.size() > 0) {
            t = move(m_queue.front()); //取出队首元素，返回队首元素值，并进行右值引用
            return true;
        }
        return false;
    }

    bool pop(T &t) {
        unique_lock<mutex> lock(m_mutex);
        if (m_queue.size() > 0) {
            t = move(m_queue.front());
            m_queue.pop(); //弹出入队的第一个元素
            return true;
        }
        return false;
    }

private:
    mutex m_mutex;//访问互斥信号量
    queue<T> m_queue; //利用模板函数构造队列
};

#endif //YYEVA_SAFEQUEUE_H
