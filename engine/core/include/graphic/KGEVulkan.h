#pragma once

#define ENABLE_DEBUG_LAYERS

#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace kge
{
    namespace vkutility
    {
        //Проверка на наличие запрашиваемых расширений у инстанса
        bool checkInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames);

        //Проверка на наличие запрашиваемых слолев валидации
        bool checkValidationLayersSupported(std::vector<const char*> instanceExtensionsNames);

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugVulkanCallback(
                VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objType,
                uint64_t obj,
                size_t location,
                int32_t code,
                const char* layerPrefix,
                const char* msg,
                void* userData);
    }
}
