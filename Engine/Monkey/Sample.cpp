#include "Sample.h"
#include "Application.h"

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


