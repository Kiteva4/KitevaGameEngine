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

    virtual void Init(unsigned int Width, unsigned int Height);

    virtual VkSurfaceKHR CreateSurface(VkInstance& inst);

private:

    HINSTANCE       m_hinstance;
    HWND            m_hwnd;
    std::wstring    m_appName;
};
#endif
