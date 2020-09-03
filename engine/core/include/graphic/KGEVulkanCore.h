#ifndef KGEVULKANCORE_H
#define KGEVULKANCORE_H

#include <string>
#include <vector>
#include <iostream>
#include <graphic/KGEVulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

#include <graphic/VulkanCoreModules/KGEVkInstance.h>
#include <graphic/VulkanCoreModules/KGEVkReportCallBack.h>
#include <graphic/VulkanCoreModules/KGEVkSurface.h>
#include <graphic/VulkanCoreModules/KGEVkDevice.h>
#include <graphic/VulkanCoreModules/KGEVkRenderPass.h>
#include <graphic/VulkanCoreModules/KGEVkSwapChain.h>
#include <graphic/VulkanCoreModules/KGEVkGraphicsPipeline.h>
#include <graphic/VulkanCoreModules/KGEVkPipelineLayout.h>
#include <graphic/VulkanCoreModules/KGEVkCommandPool.h>
#include <graphic/VulkanCoreModules/KGEVkCommandBuffer.h>
#include <graphic/VulkanCoreModules/KGEVkUniformBufferWorld.h>
#include <graphic/VulkanCoreModules/KGEVkUniformBufferModels.h>
#include <graphic/VulkanCoreModules/KGEVkDescriptorPool.h>
#include <graphic/VulkanCoreModules/KGEVkDescriptorSetLayout.h>
#include <graphic/VulkanCoreModules/KGEVkSampler.h>
#include <graphic/VulkanCoreModules/KGEVkDescriptorSet.h>
#include <graphic/VulkanCoreModules/KGEVkUboModels.h>
#include <graphic/VulkanCoreModules/KGEVkSynchronization.h>

// Параметры камеры по умолчанию (угол обзора, границы отсечения)
#define DEFAULT_FOV 60.0f
#define DEFAULT_NEAR 0.1f
#define DEFAULT_FAR 256.0f

// Интервал значений глубины в OpenGL от -1 до 1. В Vulkan - от 0 до 1 (как в DirectX)
// Данный символ "сообщит" GLM что нужно использовать интервал от 0 до 1, что скажется
// на построении матриц проекции, которые используются в шейдере
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class KGEVulkanCore
{
public:
    KGEVulkanCore(uint32_t width,
                  uint32_t heigh,
                  std::string applicationName,
                  IVulkanWindowControl* windowControl,
                  unsigned int primitivesMaxCount,
                  std::vector <const char*> instanceExtensionsRequired,
                  std::vector <const char*> deviceExtensionsRequired,
                  std::vector <const char*> validationLayersRequired);


    /**
    * Приостановка рендеринга с ожиданием завершения выполнения всех команд
    */
    void Pause();

    /**
    * Возврат к состоянию "рендерится"
    */
    void Continue();

    /**
    * Данный метод вызывается при смене разрешения поверхности отображения
    * либо (в дальнейшем) при смене каких-либо ниных настроек графики. В нем происходит
    * пересоздание swap-chain'а и всего того, что зависит от измененных настроек
    */
    void VideoSettingsChanged();

    /**
    * В методе отрисовки происходит отправка подготовленных команд а так-же показ
    * готовых изображение на поверхности показа
    */
    void Draw();

    /**
    * В методе обновления происходит отправка новых данных в UBO буферы, то есть
    * учитываются положения камеры, отдельных примитивов и сцены в целом
    */
    void Update();

    /**
    * Настройка параметров перспективы камеры (угол обзора, грани отсечения)
    * @param float fFOV - угол обзора
    * @param float fNear - ближняя грань отсечения
    * @param float fFar - дальняя гран отсечения
    */
    void SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar);

    /**
    * Настройка положения камеры
    * @param float x - положение по оси X
    * @param float y - положение по оси Y
    * @param float z - положение по оси Z
    */
    void SetCameraPosition(float x, float y, float z);

    /**
    * Настройка поворота камеры
    * @param float x - поворот вокруг оси X
    * @param float y - поворот вокруг оси Y
    * @param float z - поворот вокруг оси Z
    */
    void SetCameraRotation(float x, float y, float z);

    /**
    * Добавление нового примитива
    * @param const std::vector<kge::vkstructs::Vertex> &vertices - массив вершин
    * @param const std::vector<unsigned int> &indices - массив индексов
    * @param glm::vec3 position - положение относительно глобального центра
    * @param glm::vec3 rotaton - вращение вокруг локального центра
    * @param glm::vec3 scale - масштаб
    * @return unsigned int - индекс примитива
    */
    unsigned int AddPrimitive(
            const std::vector<kge::vkstructs::Vertex> &vertices,
            const std::vector<unsigned int> &indices,
            const kge::vkstructs::Texture *texture,
            glm::vec3 position,
            glm::vec3 rotaton,
            glm::vec3 scale = { 1.0f,1.0f,1.0f });

    /**
    * Создание текстуры по данным о пикселях
    * @param const unsigned char* pixels - пиксели загруженные из файла
    * @return kge::vkstructs::Texture - структура с набором хендлов изображения и дескриптора
    *
    * @note - при загрузке используется временный буфер (временное изображение) для перемещения
    * в буфер распологающийся в памяти устройства. Нельзя сразу создать буфер в памяти устройства и переместить
    * в него данные. Это можно сделать только пр помощи команды копирования (из памяти доступной хосту в память
    * доступную только устройству)
    */
    kge::vkstructs::Texture CreateTexture(const unsigned char* pixels,
                                          uint32_t width,
                                          uint32_t height,
                                          uint32_t channels,
                                          uint32_t bpp = 4);
    ~KGEVulkanCore();
private:

    bool m_isReady;                      // Состояние готовности к рендерингу
    bool m_isRendering;                  // В процессе ли рендеринг
    unsigned int m_primitivesMaxCount;   // Максимальное кол-во примитивов (необходимо для аллокации динамического UBO буфера)

    uint32_t m_width;
    uint32_t m_heigh;

    /* Instance*/
    //VkInstance m_vkInstance{}; // Хендл instance'а vulkan'а
    KGEVkInstance m_kgeVkInstance;

    /* Report CallBack*/
    //KGEVkReportCallBack m_kgeVkReportCallBack;

    /* Surface */
    KGEVkSurface m_kgeVkSurface;

    /* Device */
    KGEVkDevice m_kgeVkDevice;

    /* RenderPass */
    KGEVkRenderPass m_kgeRenderPass;

    /* Swap chain */
    KGEVkSwapChain m_kgeSwapChain;

    /* Command Pool */
    KGEVkCommandPool m_kgeVkCommandPool;

    /*Command Buffer*/
    KGEVkCommandBuffer m_kgeVkCommandBuffer;

    //Аллокация глобального uniform-буфера
    KGEVkUniformBufferWorld m_kgeVkUniformBufferWorld;

    // Аллокация uniform-буфера отдельных объектов (динамический буфер)
    KGEVkUniformBufferModels m_kgeVkUniformBufferModels;

    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    KGEVkDescriptorPool m_kgeVkDescriptorPoolMain;

    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    KGEVkDescriptorPool m_kgeVkDescriptorPoolTextures;

    /* Descriptor set layout*/
    KGEVkDescriptorSetLayout m_kgeVkDescriptorSetLayoutMain;

    KGEVkDescriptorSetLayout m_kgeVkDescriptorSetLayoutTextures;

    /* Texture Sampler */
    KGEVkSampler m_kgeVkSampler;

    /* Descriptor Set*/
    KGEVkDescriptorSet m_kgeVkDescriptorSet;

    /* Pipeline Layout */
    KGEVkPipelineLayout m_kgeVkPipelineLayout;

    /* Pipeline */
    KGEVkGraphicsPipeline m_kgeVkGraphicsPipeline;

    /* Ubo */
    kge::vkstructs::UboModelArray m_uboModels;              // Массив матриц (указатель на него) для отдельный объектов (матрицы модели, передаются в буфер формы объектов)
    KGEVkUboModels m_kgeUboModels;

    /* Synchronization */
    kge::vkstructs::Synchronization m_sync;                 // Примитивы синхронизации
    KGEVkSynchronization m_kgeVkSynchronization;

    std::vector<kge::vkstructs::Primitive> m_primitives;     // Набор геометр. примитивов для отображения
    kge::vkstructs::UboWorld m_uboWorld;                     // Структура с матрицами для общих преобразований сцены (данный объект буедт передаваться в буфер формы сцены)

    /**
    * Подготовка команд, заполнение командных буферов
    * @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов
    * @param VkRenderPass renderPass - хендл прохода рендеринга, используется при привязке прохода
    * @param VkPipelineLayout pipelineLayout - хендл размещения конвейра, исппользуется при привязке дескрипторов
    * @param VkDescriptorSet descriptorSet - хендл набор дескрипторов, исппользуется при привязке дескрипторов
    * @param VkPipeline pipeline - хендл конвейера, используется при привязке конвейера
    * @param const kge::vkstructs::Swapchain &swapchain - свопчейн, используется при конфигурации начала прохода (в.ч. для указания фрейм-буфера)
    * @param const std::vector<kge::vkstructs::Primitive> &primitives - массив примитивов (привязка буферов вершин, буферов индексов для каждого и т.д)
    *
    * @note - данную операцию нет нужды выполнять при каждом обновлении кадра, набор команд как правило относительно неизменный. Метод лишь заполняет
    * буферы команд, а сама отправка происходть в draw. При изменении кол-ва примитивов следует сбросить командные буферы и заново их заполнить
    * при помощи данного метода
    */
    void PrepareDrawCommands(std::vector<VkCommandBuffer> commandBuffers,
                             VkRenderPass renderPass,
                             VkPipelineLayout pipelineLayout,
                             VkDescriptorSet descriptorSetMain,
                             VkPipeline pipeline,
                             const kge::vkstructs::Swapchain &swapchain,
                             const std::vector<kge::vkstructs::Primitive> &primitives);

    /**
    * Сброс командных буферов (для перезаписи)
    * @param const kge::vkstructs::Device &device - устройство, для получения хендлов очередей
    * @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов, которые необходимо сбросить
    */
    void ResetCommandBuffers(const kge::vkstructs::Device &device,
                             std::vector<VkCommandBuffer> commandBuffers);

    kge::vkstructs::CameraSettings m_camera;
};


#endif // KGEVULKANCORE_H
