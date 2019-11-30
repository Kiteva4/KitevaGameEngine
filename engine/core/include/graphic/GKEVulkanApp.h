#pragma once

#include "VulkanWindowControl/IVulkanWindowControl.h"
#include <graphic/KGEVulkanCore.h>

#include <vector>

class GKEVulkanApp
{
public:

    GKEVulkanApp(uint32_t width, uint32_t heigh);
    ~GKEVulkanApp();

    void Init();
    void Run();

private:

    uint32_t m_appWidth{};
    uint32_t m_appHeigh{};
    IVulkanWindowControl* m_windowControl{};
    KGEVulkanCore* m_KGEVulkanCore{};
    std::vector<const char* > m_extensions{};
};
