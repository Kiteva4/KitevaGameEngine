#include "graphic/KGEVulkan.h"

#include <string>
#include <stdio.h>
#include <cassert>

//#include <vulkan_core.h>

void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags)
{
    if (flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
        printf("Image usage transfer src is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
        printf("Image usage transfer dest is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        printf("Image usage sampled is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        printf("Image usage color attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        printf("Image usage depth stencil attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
        printf("Image usage transient attachment is supported\n");
    }

    if (flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        printf("Image usage input attachment is supported\n");
    }
}


void VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps)
{
    unsigned int NumExt = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &NumExt, nullptr);
    CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res)

            printf("Found %d extensions\n", NumExt);

    ExtProps.resize(NumExt);

    res = vkEnumerateInstanceExtensionProperties(nullptr, &NumExt, &ExtProps[0]);
    CHECK_VULKAN_ERROR("vkEnumerateInstanceExtensionProperties error %d\n", res)

            for (unsigned int i = 0 ; i < NumExt ; i++) {
        printf("Instance extension %d - %s\n", i, ExtProps[i].extensionName);
    }
}


VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName)
{
    int codeSize = 0;
    char* pShaderCode = ReadBinaryFile(pFileName, codeSize);
    assert(pShaderCode);

    VkShaderModuleCreateInfo shaderCreateInfo = {};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = codeSize;
    shaderCreateInfo.pCode = (const uint32_t*)pShaderCode;

    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule);
    CHECK_VULKAN_ERROR("vkCreateShaderModule error %d\n", res);
    printf("Created shader %s\n", pFileName);
    return shaderModule;
}

/*! @brief
    @brief[in] instance     - экземпляр
    @brief[in] surface      - поверхность
    @brief[in] physDevices  - данные о графических устройствах */
void VulkanGetPhysicalDevices(const VkInstance& instance, const VkSurfaceKHR& surface, VulkanPhysicalDevices& physDevices)
{
    unsigned int NumDevices = 0;

    VkResult res = vkEnumeratePhysicalDevices(instance, &NumDevices, nullptr);
    CHECK_VULKAN_ERROR("vkEnumeratePhysicalDevices error %d\n", res);

    printf("Num physical devices %d\n", NumDevices);

    physDevices.m_devices.resize(NumDevices);
    physDevices.m_devProps.resize(NumDevices);
    physDevices.m_qFamilyProps.resize(NumDevices);
    physDevices.m_qSupportsPresent.resize(NumDevices);
    physDevices.m_surfaceFormats.resize(NumDevices);
    physDevices.m_surfaceCaps.resize(NumDevices);

    res = vkEnumeratePhysicalDevices(instance, &NumDevices, &physDevices.m_devices[0]);
    CHECK_VULKAN_ERROR("vkEnumeratePhysicalDevices error %d\n", res);

    for (unsigned int i = 0 ; i < NumDevices ; i++)
    {
        const VkPhysicalDevice& PhysDev = physDevices.m_devices[i];
        vkGetPhysicalDeviceProperties(PhysDev, &physDevices.m_devProps[i]);

        printf("Device name: %s\n", physDevices.m_devProps[i].deviceName);
        uint32_t apiVer = physDevices.m_devProps[i].apiVersion;
        printf("    API version: %d.%d.%d\n",
               VK_VERSION_MAJOR(apiVer),
               VK_VERSION_MINOR(apiVer),
               VK_VERSION_PATCH(apiVer));

        unsigned int NumQFamily = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, nullptr);

        printf("Num of family queues: %d\n", NumQFamily);

        physDevices.m_qFamilyProps[i].resize(NumQFamily);
        physDevices.m_qSupportsPresent[i].resize(NumQFamily);

        vkGetPhysicalDeviceQueueFamilyProperties(PhysDev, &NumQFamily, &(physDevices.m_qFamilyProps[i][0]));

        for (unsigned int q = 0 ; q < NumQFamily ; q++)
        {
            res = vkGetPhysicalDeviceSurfaceSupportKHR(PhysDev, q, surface, &(physDevices.m_qSupportsPresent[i][q]));
            CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceSupportKHR error %d\n", res);
        }

        unsigned int NumFormats = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, surface, &NumFormats, nullptr);
        assert(NumFormats > 0);

        physDevices.m_surfaceFormats[i].resize(NumFormats);

        res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysDev, surface, &NumFormats, &(physDevices.m_surfaceFormats[i][0]));
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR error %d\n", res);

        for (unsigned int j = 0 ; j < NumFormats ; j++)
        {
            const VkSurfaceFormatKHR& SurfaceFormat = physDevices.m_surfaceFormats[i][j];
            printf("    Format %d color space %d\n", SurfaceFormat.format , SurfaceFormat.colorSpace);
        }

        res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysDev, surface, &(physDevices.m_surfaceCaps[i]));
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR error %d\n", res);

        VulkanPrintImageUsageFlags(physDevices.m_surfaceCaps[i].supportedUsageFlags);

        unsigned int NumPresentModes = 0;

        res = vkGetPhysicalDeviceSurfacePresentModesKHR(PhysDev, surface, &NumPresentModes, nullptr);
        CHECK_VULKAN_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR error %d\n", res);

        assert(NumPresentModes != 0);

        printf("Number of presentation modes %d\n", NumPresentModes);
    }
}
