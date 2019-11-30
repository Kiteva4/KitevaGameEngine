#ifndef MACOSWINDOWCONTROL_H
#define MACOSWINDOWCONTROL_H

#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"

class MacOSWindowControl : public IVulkanWindowControl
{
public:

    MacOSWindowControl(const char* appName);
    virtual ~MacOSWindowControl();
    virtual void Init(unsigned int Width, unsigned int Height) override;

    virtual VkSurfaceKHR CreateSurface(VkInstance& inst) override;
};

#endif // MACOSWINDOWCONTROL_H
