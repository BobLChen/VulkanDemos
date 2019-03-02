#include "Engine.h"
#include "Application/SlateApplication.h"

Engine* Engine::g_Instance = nullptr;

Engine::Engine()
    : m_VulkanRHI(nullptr)
	, m_IsRequestingExit(false)
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

	std::string assetsPath = cmdLine[0];
	int32 length = 0;
	for (size_t i = 0; i < assetsPath.size(); ++i)
	{
		if (assetsPath[i] == '\\')
		{
			assetsPath[i] = '/';
		}
	}
	for (size_t i = assetsPath.size() - 1; i >= 0; --i)
	{
		if (assetsPath[i] == '/')
		{
			break;
		}
		length += 1;
	}
	m_AssetsPath = assetsPath.substr(0, assetsPath.size() - length);
	
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
	SlateApplication::Get().Tick(0.0069444444444444f);
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