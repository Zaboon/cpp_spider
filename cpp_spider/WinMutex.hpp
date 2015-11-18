#ifndef WINMUTEX_H_
# define WINMUTEX_H_

# include				<winnt.h>
# include				<intsafe.h>
# include				<WinBase.h>
# include				<Windows.h>
# include				<processthreadsapi.h>
# include				"IMutex.hpp"

class WinMutex :		public IMutex
{
private:
	HANDLE				_mutex;

public:
	WinMutex()
	{
		this->_mutex = CreateMutex(NULL, FALSE, NULL);
	}

	~WinMutex()
	{
		CloseHandle(this->_mutex);
	}

	virtual bool		lock(bool wait = true)
	{
		DWORD			waitResult;

		if ((waitResult = WaitForSingleObject(this->_mutex, (wait) ? INFINITE : 0)) == WAIT_OBJECT_0)
		{
			this->_status = IMutex::Locked;
			return (true);
		}
		return (false);
	}

	virtual bool		unlock()
	{
		if (this->_status == IMutex::Locked)
		{
			ReleaseMutex(this->_mutex);
			this->_status = IMutex::Unlocked;
		}
		return (true);
	}
};

#endif /* !WINMUTEX_H_ */