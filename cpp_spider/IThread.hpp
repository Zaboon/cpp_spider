#if defined(_WIN32) && !defined(WIN32)
# define _WINSOCKAPI_
# include <windows.h>
# define WIN32
#endif

#ifndef PROJECT2_ITHREAD_HPP
# define PROJECT2_ITHREAD_HPP

# include <iostream>
# include <string>
# include <vector>

# include "IMutex.hpp"
# include "MutexVault.hpp"

template<typename T, typename U>
class IThread
{
public:

	//status define
	enum Status
	{
		Ready,
		Running,
		Paused,
		Stopped
	};

	static void *(entry_point)(void *_this)
	{
		IThread *self = static_cast<IThread<T, U> *>(_this);

		self->setStatus(Running);
		self->_func_ptr(self->getId(), self->getParameter());
		self->setStatus(Stopped);
		MutexVault::getMutexVault()->remove(self->getId());
		return (NULL);
	}

	//id and status
	unsigned int _id;
	Status _status;

	//mutex vault
	MutexVault *_mutex_vault;

	//target function attributes
	T (*_func_ptr)(unsigned int, U);
	U _param;

public:

	IThread(T (*func_ptr)(unsigned int, U))
	{
		static unsigned int id = 0;

		this->_id = id++;
		this->_func_ptr = func_ptr;
		this->_status = Ready;
		this->_mutex_vault = MutexVault::getMutexVault();
	}

	virtual ~IThread()
	{
		this->_mutex_vault->remove(this->_id);
	}

	Status getStatus() const
	{
		return (this->_status);
	}

	unsigned int getId() const
	{
		return (this->_id);
	}

	void setStatus(Status status)
	{
		this->_status = status;
	}

	virtual U getParameter() const
	{
		return (this->_param);
	}

	virtual bool operator()(U param) = 0;

	bool lock(bool wait)
	{
		IMutex *mutex;

		if ((mutex = (*this->_mutex_vault)[this->_id]) == NULL)
			return (false);
		this->setStatus(Paused);
		return (mutex->lock(wait));
	}

	bool unlock()
	{
		IMutex *mutex;

		if ((mutex = (*this->_mutex_vault)[this->_id]) == NULL)
			return (false);
		this->setStatus(Running);
		return (mutex->unlock());
	}

	virtual bool stop() = 0;
	
};

#endif /* !PROJECT2_ITHREAD_HPP */