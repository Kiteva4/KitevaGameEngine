#ifndef KGEVKDESCRIPTORSETLAYOUT_H
#define KGEVKDESCRIPTORSETLAYOUT_H

#include <graphic/KGEVulkan.h>

typedef enum
{
    SetLayoutMain,
    SetLayoutTextures
}SET_LAYOUT_TYPE;

class KGEVkDescriptorSetLayout
{
    const kge::vkstructs::Device* m_device;
    VkDescriptorSetLayout m_descriptorSetLayout;
public:
    KGEVkDescriptorSetLayout(const kge::vkstructs::Device* device,
                             SET_LAYOUT_TYPE setLayoutType);
    ~KGEVkDescriptorSetLayout();
    const VkDescriptorSetLayout &descriptorSetLayout();
};

#endif // KGEVKDESCRIPTORSETLAYOUT_H
