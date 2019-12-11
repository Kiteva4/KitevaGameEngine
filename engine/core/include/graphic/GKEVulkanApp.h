#pragma once

#include "VulkanWindowControl/IVulkanWindowControl.h"
#include <graphic/KGEVulkanCore.h>

#include <vector>
#include <string>

class GKEVulkanApp
{
public:

    GKEVulkanApp(uint32_t width, uint32_t heigh, std::string applicationName);
    ~GKEVulkanApp();

    void Init();
    void Run();

private:

    uint32_t m_appWidth{};
    uint32_t m_appHeigh{};
    std::string m_applicationName;
    IVulkanWindowControl* m_windowControl{};
    KGEVulkanCore* m_KGEVulkanCore{};
    std::vector<const char* > m_instanceExtensions{};
    std::vector<const char* > m_deviceExtensions{};
    std::vector<const char* > m_validationLayersExtensions{};
};
