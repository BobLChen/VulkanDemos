#include "Engine.h"
#include "Application/SlateApplication.h"


Engine::Engine()
    : m_VulkanRHI(nullptr)
	, m_IsRequestingExit(false)
{

}

Engine::~Engine()
{

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
	return 0;
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
