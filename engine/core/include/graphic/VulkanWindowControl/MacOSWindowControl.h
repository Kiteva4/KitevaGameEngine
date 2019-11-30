#ifdef __APPLE__
#ifndef MACOSWINDOWCONTROL_H
#define MACOSWINDOWCONTROL_H

#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"

class MacOSWindowControl : public IVulkanWindowControl
{
public:

    MacOSWindowControl(const char* appName);
    virtual ~MacOSWindowControl() override;
    virtual void Init(unsigned int Width, unsigned int Height) override;

    virtual VkSurfaceKHR CreateSurface(VkInstance& vkInstance) override;
};

#endif // MACOSWINDOWCONTROL_H

#endif
