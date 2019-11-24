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

void WindowsWindowControl::Init(unsigned int Width, unsigned int Height)
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

#endif
