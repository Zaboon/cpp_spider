//
// Created by rustam_t on 10/6/15.
//

#ifndef PROJECT2_LINUXTHREAD_H
#define PROJECT2_LINUXTHREAD_H

#include <pthread.h>
#include <unistd.h>
#include <exception>
#include "../IThread.hpp"
#include "LinuxMutex.hpp"

template<typename T, typename U>
class LinuxThread : public IThread<T,U>
{

protected:

    pthread_t _thread;

public:

    LinuxThread(T (*func_ptr)(unsigned int, U)) : IThread<T, U>(func_ptr)
    {
        IThread<T, U>::_mutex_vault->push_back(new LinuxMutex());
    }

    virtual ~LinuxThread()
    {
    };

    //overloaded
    virtual bool operator()(U param)
    {
        this->_param = param;

        return (pthread_create(&this->_thread, NULL, IThread<T, U>::entry_point, static_cast<void *>(this)) == 0);
    }

    virtual bool stop()
    {
        if (this->getStatus() == IThread<T, U>::Stopped ||
            this->getStatus() == IThread<T, U>::Ready)
            return (false);

        this->setStatus(IThread<T, U>::Stopped);
        return (pthread_cancel(this->_thread) == 0);
    }

};


#endif //PROJECT2_LINUXTHREAD_H
