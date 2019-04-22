#include "Engine.h"
#include "Application/SlateApplication.h"

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

int32 Engine::PreInit(const std::vector<std::string>& cmdLine, int32 width, int32 height, const char* title)
{
	SlateApplication::Create(this);
	SlateApplication::Get().MakeWindow(width, height, title);

	m_VulkanRHI = std::make_shared<VulkanRHI>();
	m_VulkanRHI->Init();
    
    if (cmdLine.size() > 0)
    {
        std::string exePath = cmdLine[0];
        int32 length = 0;
        
        for (size_t i = 0; i < exePath.size(); ++i)
        {
            if (exePath[i] == '\\')
            {
                exePath[i] = '/';
            }
        }
        
        for (size_t i = exePath.size() - 1; i >= 0; --i)
        {
            if (exePath[i] == '/')
            {
                break;
            }
            length += 1;
        }
        
        m_AssetsPath = exePath.substr(0, exePath.size() - length);
    }
	
    MLOG("AssetsPath:%s", m_AssetsPath.c_str());
	return 0;
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
	SlateApplication::Get().Shutdown();
}

void Engine::Tick()
{
	SlateApplication::Get().Tick(0.016f);
}

void Engine::PumpMessage()
{
	SlateApplication::Get().PumpMessages();
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
