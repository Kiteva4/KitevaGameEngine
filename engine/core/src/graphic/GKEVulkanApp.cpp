#include "graphic/VulkanWindowControl/WindowsWindowControl.h"
#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
#include "graphic/VulkanWindowControl/MacOSWindowControl.h"
#include "graphic/VulkanWindowControl/GLFWWindowControl.h"

#include "graphic/GKEVulkanApp.h"
#include "graphic/KGEVulkanCore.h"

// Переменная IS_VK_DEBUG будет true если используется debug конфиуграция
// В зависимости от данной переменной некоторое поведение может меняться
#ifdef DEBUG
const bool IS_VK_DEBUG = false;
#else
const bool IS_VK_DEBUG = true;
#endif

GKEVulkanApp::GKEVulkanApp(uint32_t width, uint32_t heigh, std::string applicationName):
    m_appWidth{width},
    m_appHeigh{heigh},
    m_applicationName{applicationName},
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
#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
    // Windows (x64 and x86)
    m_windowControl = new GLFWWindowControl("Window Name");
    //    m_windowControl = new WindowsWindowControl("Window Name");
    //    m_extensions.insert(
    //                m_extensions.end(),
    //                {VK_KHR_SURFACE_EXTENSION_NAME,
    //                 VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    //                 VK_EXT_DEBUG_REPORT_EXTENSION_NAME});
#elif __linux__
    // linux
    m_windowControl = new GLFWWindowControl("Window Name");
#elif __APPLE__
    // Mac OS, not sure if this is covered by __posix__ and/or __unix__ though...
//#define VK_USE_PLATFORM_MACOS_MVK
    m_windowControl = new GLFWWindowControl("Window Name");
#else

#endif

    m_windowControl->Init(m_appWidth,m_appHeigh);
    //Запрос необходимых glfw расширений
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions{};
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> instanceExtensions (glfwExtensions, glfwExtensions+glfwExtensionCount);

    m_instanceExtensions         = std::move(instanceExtensions);
    m_deviceExtensions           = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    m_validationLayersExtensions = {"VK_LAYER_LUNARG_standard_validation"};

    // Если это DEBUG конфигурация - запросить еще расширения и слои для валидации
    if(IS_VK_DEBUG){
        std::cout << "DEBUG: debug extensions included!" << std::endl;
        m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        m_validationLayersExtensions.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    // Enable surface extensions depending on os
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
    instanceExtensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
    instanceExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
    m_instanceExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    m_KGEVulkanCore = new KGEVulkanCore(m_appWidth,
                                        m_appHeigh,
                                        m_applicationName,
                                        m_windowControl,
                                        1000,
                                        m_instanceExtensions,
                                        m_deviceExtensions,
                                        m_validationLayersExtensions);
}

void GKEVulkanApp::Run()
{
    std::cout << "app tick" << std::endl;
}
