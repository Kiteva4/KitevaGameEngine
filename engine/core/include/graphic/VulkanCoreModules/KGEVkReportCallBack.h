#ifndef KGEVKREPORTCALLBACK_H
#define KGEVKREPORTCALLBACK_H

#include <graphic/KGEVulkan.h>

class KGEVkReportCallBack
{
    PFN_vkCreateDebugReportCallbackEXT m_createDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT m_destroyDebugReportCallbackEXT;
    VkDebugReportCallbackEXT debug_report_callback;
    VkInstance m_instance;
public:
    KGEVkReportCallBack(VkInstance instance);
    ~KGEVkReportCallBack();
};

#endif // KGEVKREPORTCALLBACK_H
