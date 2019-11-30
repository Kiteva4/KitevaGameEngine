#include "graphic/VulkanWindowControl/WindowsWindowControl.h"
#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"

#include "graphic/GKEVulkanApp.h"
#include "graphic/KGEVulkanCore.h"

GKEVulkanApp::GKEVulkanApp(uint32_t width, uint32_t heigh):
    m_appWidth{width},
    m_appHeigh{heigh},
    m_windowControl{nullptr},
    m_KGEVulkanCore{nullptr}
{

}

GKEVulkanApp::~GKEVulkanApp()
{
    delete m_windowControl;
    delete m_KGEVulkanCore;
}


void GKEVulkanApp::Init()
{
//    renderer = new KGEVulkanCore();
//    delete renderer;

#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
// Windows (x64 and x86)
    m_windowControl = new WindowsWindowControl("Window Name");
#elif __unix__ // all unices, not all compilers
// Unix
#elif __linux__
// linux
    m_windowControl = new LinuxXCBWindowControl();
#elif __APPLE__
// Mac OS, not sure if this is covered by __posix__ and/or __unix__ though...

#endif

    m_KGEVulkanCore = new KGEVulkanCore(m_appWidth, m_appHeigh, m_windowControl);
}

void GKEVulkanApp::Run()
{
    std::cout << "app tick" << std::endl;
}
