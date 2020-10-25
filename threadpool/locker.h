//
// Created by wonder on 2020/10/24.
// 线程同步机制包装类
//

#ifndef HTTPWEBSERVER_LOCKER_H
#define HTTPWEBSERVER_LOCKER_H

#include <semaphore.h>
#include <exception>
#include <pthread.h>

//信号量封装类
class sem{
public:
    sem(){
       if(sem_init(&m_sem,0,0) != 0){
           throw std::exception();
       }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }
    bool post(){
        return sem_post(&m_sem) == 0;
    }
private:
    sem_t m_sem;
};
//互斥锁类
class locker{
public:
    locker(){
        if(pthread_mutex_init(&m_mutex,NULL) != 0){
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock(){
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock(){
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
private:
    pthread_mutex_t m_mutex;
};
//条件变量类
class cond{
public:
    cond(){
        if(pthread_mutex_init(&m_mutex,NULL) != 0){
            throw std::exception();
        }
        if(pthread_cond_init(&m_cond,NULL) != 0){
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond(){
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    bool wait(){
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond,&m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal(){
        return pthread_cond_signal(&m_cond) == 0;
    }
private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#endif //HTTPWEBSERVER_LOCKER_H
