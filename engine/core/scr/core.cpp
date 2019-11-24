#include "core.h"
#include <graphic/KGEVulkanCore.h>

#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
#include "graphic/VulkanWindowControl/WindowsWindowControl.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

Core::Core()
{

    IVulkanWindowControl* pWindowControl = nullptr;

#ifdef __linux__
    pWindowControl = new LinuxXCBWindowControl();
#elif _WIN32
    pWindowControl = new WindowsWindowControl("KitevaGameEngine");
#else
    //TODO
#endif

    pWindowControl->Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    KGEVulkanCore core("KitevaGameEngine");
    core.Init(pWindowControl);
}

Core::~Core() {}
