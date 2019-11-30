#include "graphic/VulkanWindowControl/GLFWWindowControl.h"

GLFWWindowControl::GLFWWindowControl(const char *appName) : m_appName{appName}
{

}

GLFWWindowControl::~GLFWWindowControl()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void GLFWWindowControl::Init(uint32_t Width, uint32_t Height)
{

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);


    m_window = glfwCreateWindow(
                static_cast<int>(Width),
                static_cast<int>(Height),
                m_appName,
                nullptr, nullptr);
}

VkSurfaceKHR GLFWWindowControl::CreateSurface(VkInstance &vkInstance)
{
    VkSurfaceKHR surface;

    if(glfwCreateWindowSurface(vkInstance, m_window, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Vulkan: Failed to create glfw window surface");
    }

    return surface;
}
