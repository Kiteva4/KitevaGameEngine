#pragma once

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"

class WindowsWindowControl : public IVulkanWindowControl
{
public:
    WindowsWindowControl(const char* appName);

    ~WindowsWindowControl();

    virtual void Init(uint32_t Width, uint32_t Height) override;

    virtual VkSurfaceKHR CreateSurface(VkInstance& vkInstance) override;

private:

    HINSTANCE       m_hinstance;
    HWND            m_hwnd;
    std::wstring    m_appName;
};
#endif
