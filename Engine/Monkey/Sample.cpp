#include "Sample.h"
#include "Application.h"

NS_MONKEY_BEGIN

Sample::Sample()
    : m_Application(nullptr)
{
    
}

Sample::~Sample()
{
    
}

Application* Sample::GetApplication()
{
    return m_Application;
}

void Sample::SetupApplication(Application *application)
{
    m_Application = application;
}

NS_MONKEY_END
