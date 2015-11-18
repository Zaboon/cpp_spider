#include "Working.h"

int WINAPI WinMain(HINSTANCE thisinstance, HINSTANCE previnstance,
	LPSTR cmdline, int ncmdshow)
{
	std::cout << "yolo" << std::endl;
	Working		*doYourJob = new Working(thisinstance);
	doYourJob->Work();
	return (0);
}