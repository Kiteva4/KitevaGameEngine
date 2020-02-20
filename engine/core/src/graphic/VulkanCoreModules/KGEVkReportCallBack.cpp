#include "graphic/VulkanCoreModules/KGEVkReportCallBack.h"

KGEVkReportCallBack::KGEVkReportCallBack(VkInstance instance):
    m_instance{instance}
{
    VkResult res;
    m_createDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
    if(!m_createDebugReportCallbackEXT){
        std::cout << "GetInstanceProcAddr: Unable to find "
                     "vkCreateDebugReportCallbackEXT function."
                  << std::endl;
        throw std::runtime_error("Vulkan: Error initialize CreateReportCallBack");
    }
    std::cout << "Got dbgCreateDebugReportCallback function\n";

    m_destroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

    if (!m_destroyDebugReportCallbackEXT) {
        std::cout << "GetInstanceProcAddr: Unable to find "
                     "vkDestroyDebugReportCallbackEXT function."
                  << std::endl;
        throw std::runtime_error("Vulkan: Error initialize DestroyReportCallBack");
    }
    std::cout << "Got m_destroyDebugReportCallbackEXT function\n";

    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    create_info.pNext = nullptr;
    create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    create_info.pfnCallback = kge::vkutility::DebugVulkanCallback;
    create_info.pUserData = nullptr;

    res = m_createDebugReportCallbackEXT(instance, &create_info, nullptr, &debug_report_callback);
    switch (res) {
    case VK_SUCCESS:
        std::cout << "Successfully created debug report callback object\n";
        break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        std::cout << "dbgCreateDebugReportCallback: out of host memory pointer\n" << std::endl;
        break;
    default:
        std::cout << "dbgCreateDebugReportCallback: unknown failure\n" << std::endl;
        break;
    }
}

KGEVkReportCallBack::~KGEVkReportCallBack()
{
    m_destroyDebugReportCallbackEXT(m_instance, debug_report_callback, nullptr);
}
