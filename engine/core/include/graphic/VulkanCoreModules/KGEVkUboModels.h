#ifndef KGEVKUBOMODELS_H
#define KGEVKUBOMODELS_H

#include <graphic/KGEVulkan.h>

class KGEVkUboModels
{
    kge::vkstructs::UboModelArray* m_uboModels; // Массив матриц (указатель на него) для отдельный объектов (матрицы модели, передаются в буфер формы объектов)
    const kge::vkstructs::Device* m_device;
public:
    KGEVkUboModels(kge::vkstructs::UboModelArray* uboModels,
                   const kge::vkstructs::Device* device,
                   unsigned int maxObjects);
    ~KGEVkUboModels();
};

#endif // KGEVKUBOMODELS_H
