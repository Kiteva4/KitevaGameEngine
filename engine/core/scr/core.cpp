#include <core.h>
#include "graphic/GKEVulkanApp.h"

//#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
//#include "graphic/VulkanWindowControl/WindowsWindowControl.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

Core::Core()
{
    GKEVulkanApp app(WINDOW_WIDTH, WINDOW_HEIGHT);
    app.Init();
    app.Run();
}

Core::~Core() {}
