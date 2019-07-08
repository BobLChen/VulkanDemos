#include "Common/Common.h"
#include "Common/Log.h"
#include "Launch/Launch.h"

#include "Application/Windows/WinWindow.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <windows.h>

std::vector<std::string> g_CmdLine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_HInstance = hInstance;

	int argc = 0;
	LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
	for (int32 i = 0; i < argc; ++i)
	{
		char* buf = new char[2048];
		wcstombs(buf, argv[i], 2048);
		g_CmdLine.push_back(buf);
	}

	return GuardedMain(g_CmdLine);
}

