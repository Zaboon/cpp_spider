#ifndef WINTHREAD_H_
# define WINTHREAD_H_

# include				<winnt.h>
# include				<intsafe.h>
# include				<WinBase.h>
# include				<processthreadsapi.h>
# include				"IThread.hpp"
# include				"WinMutex.hpp"
# include				"WinCriticalSession.hpp"

template<typename T, typename U>
class WinThread :		public IThread<T, U>
{
private:
	HANDLE				_thread_h;
	DWORD				_thread_id;

public:
	WinThread(T(*func_ptr)(unsigned int, U)) : IThread<T, U>(func_ptr)
	{
		IThread<T, U>::_mutex_vault->push_back(new WinCriticalSession());
	}
	virtual ~WinThread() {};

	virtual bool		operator()(U param)
	{
		this->_param = param;
		this->_thread_h = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) IThread<T, U>::entry_point, static_cast<LPVOID>(this), 0, &this->_thread_id);
		return (this->_thread_h != NULL);
	}

	virtual bool		stop()
	{
		if (this->getStatus() == IThread<T, U>::Stopped ||
			this->getStatus() == IThread<T, U>::Ready)
			return (false);

		this->setStatus(IThread<T, U>::Stopped);
		// I'm sorry Dave, I'm afraid I can't do that (terminating threads on windows)
		return (1);
	}
};


#endif /* !WINTHREAD */