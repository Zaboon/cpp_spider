//
// Created by rustam_t on 10/6/15.
//

#if defined(_WIN32) && !defined(WIN32)
# define _WINSOCKAPI_
# include <windows.h>
# define WIN32
#endif

#ifndef PROJECT2_IMUTEX_HPP
#define PROJECT2_IMUTEX_HPP

class IMutex
{
public:

    enum Status
    {
        Unlocked,
        Locked
    };

    IMutex()
    {
        this->_status = Unlocked;
    };

    virtual Status getStatus() const
    {
        return (this->_status);
    }

    virtual bool lock(bool wait = true) = 0;

    virtual bool unlock() = 0;

    virtual ~IMutex() {};

    virtual void setStatus(Status status)
    {
        this->_status = status;
    }

protected:

    Status _status;

};

#endif //PROJECT2_IMUTEX_HPP
