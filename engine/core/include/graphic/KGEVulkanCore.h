#ifndef KGEVULKANCORE_H
#define KGEVULKANCORE_H

#include <string>
#include <vector>
#include <iostream>
#include <graphic/KGEVulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

#include <graphic/VulkanCoreModules/KGEVkInstance.h>
#include <graphic/VulkanCoreModules/KGEVkSurface.h>
#include <graphic/VulkanCoreModules/KGEVkDevice.h>
//#include <graphic/VulkanCoreModules/KGEVkRenderPass.h>
#include <graphic/VulkanCoreModules/KGEVkSwapChain.h>

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

    uint32_t m_width;
    uint32_t m_heigh;

    /* Instance*/
    VkInstance m_vkInstance{}; // Хендл instance'а vulkan'а
    KGEVkInstance m_kgeVkInstance;

    /* Surface */
    VkSurfaceKHR m_vkSurface{};// Хендл поверхности отображения
    KGEVkSurface m_kgeVkSurface;

    /* Device */
    kge::vkstructs::Device m_device{};// Устройство (структура с хендлами физ-го и лог-го ус-ва, очередей)
    KGEVkDevice m_kgeVkDevice;

    /* RenderPass */
    VkRenderPass                    m_renderPass;                   // Основной проход рендеринга
    VkRenderPass InitRenderPass(const kge::vkstructs::Device &device,
                                VkSurfaceKHR surface,
                                VkFormat colorAttachmentFormat,
                                VkFormat depthStencilFormat);
    void DeinitRenderPass(const kge::vkstructs::Device &device,
                          VkRenderPass * renderPass);

    /* Swap chain */
    kge::vkstructs::Swapchain m_swapchain{};
    KGEVkSwapChain* m_kgeSwapChain;

    /* Command Pool */
    VkCommandPool m_commandPoolDraw;              // Командный пул (для выделения командных буферов)
    VkCommandPool InitCommandPool(const kge::vkstructs::Device &device,
                                  unsigned int queueFamilyIndex);
    void DeinitCommandPool(const kge::vkstructs::Device &device,
                           VkCommandPool * commandPool);


    /*Command Buffer*/
    std::vector<VkCommandBuffer> m_commandBuffersDraw;           // Командные буферы (свой на каждое изобр. swap-chain, с набором команд что и в других, в целях синхронизации)
    std::vector<VkCommandBuffer> InitCommandBuffers(const kge::vkstructs::Device &device,
                                                    VkCommandPool commandPool,
                                                    unsigned int count);
    void DeinitCommandBuffers(const kge::vkstructs::Device &device,
                              VkCommandPool commandPool,
                              std::vector<VkCommandBuffer> * buffers);



    /* Uniform Buffers */
    kge::vkstructs::UniformBuffer m_uniformBufferWorld;           // Буфер формы сцены (содержит хендлы буфера, памяти, инфо для дескриптора)
    kge::vkstructs::UniformBuffer InitUnformBufferWorld(const kge::vkstructs::Device &device);
    kge::vkstructs::UniformBuffer m_uniformBufferModels;          // Буфер формы объектов (содержит хендлы буфера, памяти, инфо для дескриптора)
    kge::vkstructs::UniformBuffer InitUniformBufferModels(const kge::vkstructs::Device &device,
                                                          unsigned int maxObjects);
    void DeinitUniformBuffer(const kge::vkstructs::Device &device,
                             kge::vkstructs::UniformBuffer * uniformBuffer);



    /* Descriptor Pool  */
    VkDescriptorPool m_descriptorPoolMain; // Пул дескрипторов (для основного набора)
    VkDescriptorPool InitDescriptorPoolMain(const kge::vkstructs::Device &device);
    VkDescriptorPool m_descriptorPoolTextures;// Пул дескрипторов (для наборов под текстуры)
    VkDescriptorPool InitDescriptorPoolTextures(const kge::vkstructs::Device &device,
                                                uint32_t maxDescriptorSets = 1000);
    void DeinitDescriptorPool(const kge::vkstructs::Device &device,
                              VkDescriptorPool * descriptorPool);


    /* Descriptor set layout*/
    VkDescriptorSetLayout m_descriptorSetLayoutMain;// Размещение набора дескрипторов (для основного набора)
    VkDescriptorSetLayout InitDescriptorSetLayoutMain(const kge::vkstructs::Device &device);
    VkDescriptorSetLayout m_descriptorSetLayoutTextures;// Размещение набора дескрипторов (для наборов под текстуры)
    VkDescriptorSetLayout InitDescriptorSetLayoutTextures(const kge::vkstructs::Device &device);
    void DeinitDescriporSetLayout(const kge::vkstructs::Device &device,
                                  VkDescriptorSetLayout * descriptorSetLayout);

    /* Texture Sampler */
    VkSampler m_textureSampler; // Текстурный семплер (описывает как данные подаются в шейдер и как интерпретируются координаты)
    VkSampler InitTextureSampler(const kge::vkstructs::Device &device);
    void DeinitTextureSampler(const kge::vkstructs::Device &device,
                              VkSampler * sampler);


    /* Descriptor Set*/
    VkDescriptorSet m_descriptorSetMain; // Набор дескрипторов (основной)
    VkDescriptorSet InitDescriptorSetMain(const kge::vkstructs::Device &device,
                                          VkDescriptorPool descriptorPool,
                                          VkDescriptorSetLayout descriptorSetLayout,
                                          const kge::vkstructs::UniformBuffer &uniformBufferWorld,
                                          const kge::vkstructs::UniformBuffer &uniformBufferModels);
    void DeinitDescriptorSet(const kge::vkstructs::Device &device,
                             VkDescriptorPool descriptorPool,
                             VkDescriptorSet * descriptorSet);





    /* Pipeline Layout */
    VkPipelineLayout m_pipelineLayout; // Размещение конвейера
    VkPipelineLayout InitPipelineLayout(const kge::vkstructs::Device &device,
                                        std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    void DeinitPipelineLayout(const kge::vkstructs::Device &device,
                              VkPipelineLayout * pipelineLayout);



    /* Pipeline */
    VkPipeline m_pipeline; // Основной графический конвейер
    VkPipeline InitGraphicsPipeline(const kge::vkstructs::Device &device,
                                    VkPipelineLayout pipelineLayout,
                                    const kge::vkstructs::Swapchain &swapchain,
                                    VkRenderPass renderPass);
    void DeinitGraphicsPipeline(const kge::vkstructs::Device &device,
                                VkPipeline * pipeline);

    unsigned int m_primitivesMaxCount;           // Максимальное кол-во примитивов (необходимо для аллокации динамического UBO буфера)

    /* Ubo */
    kge::vkstructs::UboModelArray m_uboModels; // Массив матриц (указатель на него) для отдельный объектов (матрицы модели, передаются в буфер формы объектов)
    kge::vkstructs::UboModelArray AllocateUboModels(const kge::vkstructs::Device &device,
                                                    unsigned int maxObjects);
    void FreeUboModels(kge::vkstructs::UboModelArray * uboModels);

    /* Synchronization */
    kge::vkstructs::Synchronization m_sync;                         // Примитивы синхронизации
    kge::vkstructs::Synchronization InitSynchronization(const kge::vkstructs::Device &device);
    void DeinitSynchronization(const kge::vkstructs::Device &device, kge::vkstructs::Synchronization * sync);


    std::vector<kge::vkstructs::Primitive> m_primitives;            // Набор геометр. примитивов для отображения
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
