#pragma once
#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"
#include "GLFW/glfw3.h"

class GLFWWindowControl : public IVulkanWindowControl
{
public:
    GLFWWindowControl(const char* appName);
    virtual ~GLFWWindowControl() override;
    virtual void Init(uint32_t Width, uint32_t Height) override;
    virtual VkSurfaceKHR CreateSurface(VkInstance& vkInstance) override;

private:
    const char* m_appName;
    GLFWwindow * m_window;
};
