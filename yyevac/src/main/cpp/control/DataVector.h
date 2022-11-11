//
// Created by 张保磊 on 2022/9/9.
//

#ifndef AUDIO_NATIVE_DATAVECTOR_H
#define AUDIO_NATIVE_DATAVECTOR_H

#include <vector>
#include <mutex>

template<typename T>
class DataVector {

public:

    void push_back(T data);

    T &indexAt(const int index);

    void set(const T *src, const int32_t size);

    int32_t size();

    void clear();

    void copy(T *dst, int32_t start, int32_t offset);

private:
    std::vector<T> dataVector;
    std::mutex m_mutex;

};

template<typename T>
void DataVector<T>::push_back(T data) {
    std::unique_lock<std::mutex> lock(m_mutex);
    dataVector.push_back(data);
    lock.unlock();
}

template<typename T>
T& DataVector<T>::indexAt(const int index) {
    T data = NULL;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (index >= 0 && index < dataVector.size()) {
        data = dataVector[index];
    }
    lock.unlock();
    return data;
}

template<typename T>
void DataVector<T>::copy(T *dst, int32_t start, int32_t offset) {
    std::unique_lock<std::mutex> lock(m_mutex);
    for (int i = 0; i < offset; ++i) {
        dst[i] = dataVector[start + i];
    }
    lock.unlock();
}

template<typename T>
int32_t DataVector<T>::size() {
    std::unique_lock<std::mutex> lock(m_mutex);
    int32_t size = dataVector.size();
    lock.unlock();
    return size;
}

template<typename T>
void DataVector<T>::clear() {
    std::unique_lock<std::mutex> lock(m_mutex);
    dataVector.clear();
    lock.unlock();
}

template<typename T>
void DataVector<T>::set(const T *src, const int32_t size) {
    if (size <= 0) {
        return;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    for (int i = 0; i < size; ++i) {
        dataVector.push_back(*(src + i));
    }
    lock.unlock();
}

#endif //AUDIO_NATIVE_DATAVECTOR_H
