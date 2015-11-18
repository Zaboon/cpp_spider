//
// Created by rustam_t on 10/6/15.
//

#ifndef PROJECT2_LINUXMUTEX_HPP
#define PROJECT2_LINUXMUTEX_HPP

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

class LinuxMutex : public IMutex
{
public:

    LinuxMutex()
    {
        _tid = 0;
        pthread_mutex_init(&this->_mutex, NULL);
    }

    virtual bool lock(bool wait = true)
    {
        if (!(pthread_equal(static_cast<pid_t>(syscall(SYS_gettid)), this->_tid)))
        {
            pthread_mutex_lock(&this->_mutex);
            this->_status = IMutex::Locked;
            this->_tid = static_cast<pid_t>(syscall(SYS_gettid));
            return (true);
        }
        return (false);
    };

    virtual bool unlock()
    {
        pthread_mutex_unlock(&this->_mutex);
        this->_status = IMutex::Unlocked;
        this->_tid = 0;
        return (true);
    }

    ~LinuxMutex()
    {
        pthread_mutex_destroy(&this->_mutex);
    };

protected:

    pthread_mutex_t _mutex;
    pid_t           _tid;
};

#endif //PROJECT2_LINUXMUTEX_HPP
