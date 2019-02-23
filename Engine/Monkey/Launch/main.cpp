#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/PerPlatformCppDefines.h"
#include "Vulkan/VulkanRHI.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/Quat.h"

#include <stdio.h>
#include <string>
#include <vector>

extern int32 GuardedMain(const std::vector<std::string>& cmdLine);

std::vector<std::string> g_CmdLine;

int main(int argc, const char * argv[])
{
	Quat quat;
	Plane plane;
	
    for (int i = 0; i < argc; ++i) {
        g_CmdLine.push_back(argv[i]);
        MLOG("CMDLine:%s", argv[i]);
    }
    
    return GuardedMain(g_CmdLine);
}