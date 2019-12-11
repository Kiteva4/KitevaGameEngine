#ifndef KGEVKDEVICE_H
#define KGEVKDEVICE_H

#include <graphic/KGEVulkan.h>

class KGEVkDevice
{

kge::vkstructs::Device* m_device;

public:
    KGEVkDevice(
            kge::vkstructs::Device* device,
            VkInstance vkInstance,
            VkSurfaceKHR surface,
            std::vector<const char *> deviceExtensionsRequired,
            std::vector<const char *> validationLayersRequired,
            bool uniqueQueueFamilies
            );
    ~KGEVkDevice();
};

#endif // KGEVKDEVICE_H
