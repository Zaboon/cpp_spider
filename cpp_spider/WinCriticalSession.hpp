#ifndef WinCriticalSession_H_
# define WinCriticalSession_H_

# include					<winnt.h>
# include					<intsafe.h>
# include					<WinBase.h>
# include					<Windows.h>
# include					<synchapi.h>
# include					"IMutex.hpp"

class WinCriticalSession :	public IMutex
{
private:
	CRITICAL_SECTION		_criticalSession;
	DWORD					_threadID;

public:
	WinCriticalSession()
	{
		InitializeCriticalSection(&this->_criticalSession);
	}

	~WinCriticalSession()
	{
		DeleteCriticalSection(&this->_criticalSession);
	}

	virtual bool		lock(bool wait = true)
	{
		bool			result= true;

		if (wait) EnterCriticalSection(&this->_criticalSession);
		else result = (TryEnterCriticalSection(&this->_criticalSession) != 0);
		
		this->_threadID = GetCurrentThreadId();
		if (result) this->_status = IMutex::Locked;
		return (result);
	}

	virtual bool		unlock()
	{
		if (this->_status == IMutex::Locked && this->_threadID == GetCurrentThreadId())
			LeaveCriticalSection(&this->_criticalSession);
		return (true);
	}
};

#endif /* !WinCriticalSession_H_ */