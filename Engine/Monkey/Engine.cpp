#include "Engine.h"
#include "Application/SlateApplication.h"
#include "Vulkan/VulkanDevice.h"

Engine* Engine::g_Instance = nullptr;

Engine::Engine()
    : m_VulkanRHI(nullptr)
	, m_IsRequestingExit(false)
    , m_DeltaTime(0.0f)
{
	Engine::g_Instance = this;
}

Engine::~Engine()
{
	Engine::g_Instance = nullptr;
}

std::shared_ptr<VulkanRHI> Engine::GetVulkanRHI()
{
	return m_VulkanRHI;
}

std::shared_ptr<VulkanDevice> Engine::GetVulkanDevice()
{
	return m_VulkanRHI->GetDevice();
}

VkDevice Engine::GetDeviceHandle()
{
	return GetVulkanDevice()->GetInstanceHandle();
}

const char* Engine::GetTitle()
{
	return m_SlateApplication->GetPlatformApplication()->GetWindow()->GetTitle();
}

std::shared_ptr<SlateApplication> Engine::GetApplication()
{
	return m_SlateApplication;
}

int32 Engine::PreInit(const std::vector<std::string>& cmdLine, int32 width, int32 height, const char* title)
{
    m_AppTitle = title;
    
	m_SlateApplication = std::make_shared<SlateApplication>();
	m_SlateApplication->Init(this);
	m_SlateApplication->MakeWindow(width, height, title);

	m_VulkanRHI = std::make_shared<VulkanRHI>();
	m_VulkanRHI->Init();
    
    InitAssetsPath(cmdLine);
    
	return 0;
}

void Engine::InitAssetsPath(const std::vector<std::string>& cmdLine)
{
    if (cmdLine.size() > 0)
    {
        std::string exePath = cmdLine[0];
        int32 length = 0;
        for (size_t i = 0; i < exePath.size(); ++i) {
            if (exePath[i] == '\\') {
                exePath[i] = '/';
            }
        }
        for (size_t i = exePath.size() - 1; i >= 0; --i) {
            if (exePath[i] == '/') {
                break;
            }
            length += 1;
        }
        m_AssetsPath = exePath.substr(0, exePath.size() - length);
    }
    
    MLOG("AssetsPath:%s", m_AssetsPath.c_str());
}

const std::string& Engine::GetAssetsPath() const
{
	return m_AssetsPath;
}

int32 Engine::Init()
{
	m_VulkanRHI->PostInit();
	return 0;
}

void Engine::Exist()
{
	m_VulkanRHI->Shutdown();
	m_VulkanRHI = nullptr;

	m_SlateApplication->Shutdown(true);
}

void Engine::Tick()
{
	m_SlateApplication->Tick(0.016f);
}

void Engine::PumpMessage()
{
	m_SlateApplication->PumpMessages();
}

bool Engine::IsRequestingExit()
{
	return m_IsRequestingExit;
}

void Engine::RequestExit(bool value)
{
	if (m_IsRequestingExit == value) {
		return;
	}
	m_IsRequestingExit = value;
}

Engine* Engine::Get()
{
	return g_Instance;
}
