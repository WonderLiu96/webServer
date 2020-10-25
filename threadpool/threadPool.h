//
// Created by wonder on 2020/10/22.
//

#ifndef HTTPWEBSERVER_THREADPOOL_H
#define HTTPWEBSERVER_THREADPOOL_H

#include <list>
#include <thread>
#include "locker.h"

template<typename T>
class ThreadPool {
public:
    ThreadPool(int numThreads = std::thread::hardware_concurrency(), int max_tasks = 10000);

    ~ThreadPool();

    bool addTask(T *task);

private:
    //工作线程的运行函数
    static void *worker(void *arg);
    void start(int numThreads);
    void run();

private:
    //线程池中的线程数
    int m_numThreads;
    //请求队列中允许的最大请求数
    int m_maxTasks;
    //描述线程池的数组
    pthread_t *m_threads;
    //请求队列
    std::list<T *> m_queue;
    //互斥锁
    locker m_locker;
    //是否有任务需要处理
    sem m_stat;
    //是否结束线程
    bool m_stop;
};

template<typename T>
ThreadPool<T>::ThreadPool(int numThreads, int max_tasks)
        :m_numThreads(numThreads),
         m_maxTasks(max_tasks),
         m_stop(false), m_threads(NULL){

         if((numThreads <= 0) || max_tasks <= 0){
             throw std::exception();
         }
         start(numThreads);
}
template <typename T>
void ThreadPool<T>::start(int numThreads) {
    //创建线程池
    m_threads = new pthread_t[numThreads];
    if(!m_threads){
        throw std::exception();
    }
    //创建numThreads个线程,并将他们设置为unjoinable状态(子线程结束后，资源自动回收)
    for(int i=0;i<numThreads;i++){
        if(pthread_create(m_threads + i,NULL,worker,this) != 0){
            delete [] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_threads[i])){
            delete [] m_threads;
            throw std::exception();
        }
    }
}
template <typename T>
ThreadPool<T>::~ThreadPool() {
    delete [] m_threads;
    m_stop = true;
}
template <typename T>
bool ThreadPool<T>::addTask(T *task) {
    m_locker.lock();
    if(m_queue.size() > m_maxTasks){
        m_locker.unlock();
        return false;
    }
    m_queue.push_back(task);
    m_locker.unlock();
    m_stat.post();
    return true;
}
template <typename T>
void * ThreadPool<T>::worker(void *arg) {
    ThreadPool * pool = (ThreadPool *)arg;
    pool->run();
    return pool;
}
template <typename T>
void ThreadPool<T>::run() {
    while(!m_stop){
        m_stat.wait();
        m_locker.lock();
        if(m_queue.empty()){
            m_locker.unlock();
            continue;
        }
        T * task = m_queue.front();
        m_queue.pop_front();
        m_locker.unlock();
        if(!task){
            continue;
        }
        task->process();
    }
}
#endif