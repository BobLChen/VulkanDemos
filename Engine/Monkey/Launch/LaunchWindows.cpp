#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/Windows/WinWindow.h"

#include <stdio.h>
#include <comdef.h>
#include <string>
#include <vector>
#include <iostream>

#include <windows.h>
#include <io.h>

extern int32 GuardedMain(const std::vector<std::string>& cmdLine);

std::vector<std::string> g_CmdLine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_HInstance = hInstance;

	const char* cmdLine = ::GetCommandLine();
	g_CmdLine.push_back("D:/CPPWorkspace/VulkanTutorials/Build/examples/Debug/Pipelines.exe");

	return GuardedMain(g_CmdLine);
}

