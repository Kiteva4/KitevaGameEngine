#ifdef _WIN32

#include "graphic/KGEVulkan.h"
#include "graphic/VulkanWindowControl/WindowsWindowControl.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

WindowsWindowControl::WindowsWindowControl(const char* appName)
{
    m_hinstance = GetModuleHandle(nullptr);
    assert(m_hinstance);
    m_hwnd = nullptr;
    std::string s(appName);
    m_appName = std::wstring(s.begin(), s.end());
}

WindowsWindowControl::~WindowsWindowControl()
{

}

void WindowsWindowControl::Init(uint32_t Width, uint32_t Height)
{
    WNDCLASSEX wndcls = {};

    wndcls.cbSize = sizeof(wndcls);
    wndcls.lpfnWndProc = WindowProc;
    wndcls.hInstance = m_hinstance;
    wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndcls.lpszClassName = L"Kiteva Game Engine";

    if (!RegisterClassEx(&wndcls)) {
        DWORD error = GetLastError();
        OGLDEV_ERROR("RegisterClassEx error %d", error);
    }

    m_hwnd = CreateWindowEx(0,
                            L"Kiteva Game Engine",              // class name
                            m_appName.c_str(),
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   // window style
                            100,
                            100,                                // window start
                            static_cast<int>(Width),
                            static_cast<int>(Height) ,
                            nullptr,
                            nullptr,
                            m_hinstance,
                            nullptr);

    if (m_hwnd == nullptr)
    {
        DWORD error = GetLastError();
        OGLDEV_ERROR("CreateWindowEx error %d", error);
    }

    ShowWindow(m_hwnd, SW_SHOW);
}

VkSurfaceKHR WindowsWindowControl::CreateSurface(VkInstance &vkInstance)
{
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr = {};
    win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfoKhr.hwnd = m_hwnd;
    win32SurfaceCreateInfoKhr.hinstance = m_hinstance;
    win32SurfaceCreateInfoKhr.flags = 0;
    win32SurfaceCreateInfoKhr.pNext =  nullptr;

    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
            (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");

     VkSurfaceKHR surface;

    if(vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfoKhr, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Vulkan: Error in vkCreateWin32SurfaceKHR function!");
    }

    std::cout << "Vulkan: Surface successfully created" << std::endl;

    return surface;
}

#endif
