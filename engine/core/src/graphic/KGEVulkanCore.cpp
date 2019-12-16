#include "graphic/KGEVulkanCore.h"
#include <cstring>
/**
* Конструктор рендерера
* @param uint32_t width
* @param uint32_t heigh
* @param IVulkanWindowControl* windowControl
* @param int primitivesMaxCount - максимальное кол-во отдельных объектов для отрисовки
* @param std::vector <const char*> instanceExtensionsRequired
* @param std::vector <const char*> deviceExtensionsRequired
* @param std::vector <const char*> validationLayersRequired
* @note - конструктор запистит инициализацию всех необходимых компоненстов Vulkan
*/
KGEVulkanCore::KGEVulkanCore(uint32_t width,
                             uint32_t heigh,
                             std::string applicationName,
                             IVulkanWindowControl* windowControl,
                             unsigned int primitivesMaxCount,
                             std::vector <const char*> instanceExtensionsRequired,
                             std::vector <const char*> deviceExtensionsRequired,
                             std::vector <const char*> validationLayersRequired) :
    m_isReady(false),
    m_isRendering(true),
    m_primitivesMaxCount(primitivesMaxCount),

    // Ширина и высота
    m_width(width),
    m_heigh(heigh),
    // Инициализация экземпляра
    m_vkInstance{},
    m_kgeVkInstance{&m_vkInstance, applicationName, "KGEngine", instanceExtensionsRequired, validationLayersRequired},
    // Инициализация поверхности отображения
    m_vkSurface{},
    m_kgeVkSurface{&m_vkSurface, windowControl, m_vkInstance},
    // Инициализация устройства
    m_device{},
    m_kgeVkDevice{&m_device, m_vkInstance, m_vkSurface, deviceExtensionsRequired, validationLayersRequired, false},
    // Инициализация прохода рендеринга
    m_renderPass{},
    m_kgeRenderPass{new KGEVkRenderPass{&m_renderPass, &m_device, m_vkSurface, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT}},
    // Инициализация swap-chain
    m_swapchain{},
    m_kgeSwapChain{new KGEVkSwapChain{&m_swapchain, &m_device, m_vkSurface, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, m_renderPass, 3}},
    // Инциализация командного пула
    m_commandPoolDraw{},
    m_kgeVkCommandPool{&m_commandPoolDraw, &m_device, static_cast<unsigned int>(m_device.queueFamilies.graphics)},
    // Аллокация командных буферов (получение хендлов)
    m_commandBuffersDraw{},
    m_kgeVkCommandBuffer{new KGEVkCommandBuffer{&m_commandBuffersDraw, &m_device, &m_commandPoolDraw, static_cast<unsigned int>(m_swapchain.framebuffers.size())}},
    //Аллокация глобального uniform-буфера
    m_uniformBufferWorld{},
    m_kgeVkUniformBufferWorld{&m_uniformBufferWorld, &m_device},
    // Аллокация uniform-буфера отдельных объектов (динамический буфер)
    m_uniformBufferModels{},
    m_kgeVkUniformBufferModels{&m_uniformBufferModels, &m_device, m_primitivesMaxCount},
    // Создание дескрипторного пула для выделения основного набора (для unform-буфера)
    m_descriptorPoolMain{},
    m_kgeVkDescriptorPoolMain{&m_descriptorPoolMain, &m_device},
    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    m_descriptorPoolTextures{},
    m_kgeVkDescriptorPoolTextures{&m_descriptorPoolTextures, &m_device, 1000},
    // Инициализация размещения основного дескрипторного набора
    m_descriptorSetLayoutMain{},
    m_kgeVkDescriptorSetLayoutMain{&m_descriptorSetLayoutMain, &m_device, SetLayoutMain},
    // Инициализация размещения теккстурного набора
    m_descriptorSetLayoutTextures{},
    m_kgeVkDescriptorSetLayoutTextures{&m_descriptorSetLayoutTextures, &m_device, SetLayoutTextures},

    m_descriptorSetMain(nullptr),

    // Инициализация размещения графического конвейера
    m_pipelineLayout{},
    m_kgeVkPipelineLayout{&m_pipelineLayout, &m_device, { m_descriptorSetLayoutMain, m_descriptorSetLayoutTextures}},

    // Инициализация графического конвейера
    m_pipeline{},
    m_kgeVkGraphicsPipeline{ new KGEVkGraphicsPipeline{&m_pipeline, &m_device, m_pipelineLayout, m_swapchain, m_renderPass}},

    m_uboModels(nullptr)
{
    // Присвоить параметры камеры по умолчанию
    m_camera.fFar  = DEFAULT_FOV;
    m_camera.fFar  = DEFAULT_FAR;
    m_camera.fNear = DEFAULT_NEAR;

    //m_commandPoolDraw = InitCommandPool(m_device, static_cast<unsigned int>(m_device.queueFamilies.graphics));
    // Аллокация командных буферов (получение хендлов)
    //m_commandBuffersDraw = InitCommandBuffers(m_device, m_commandPoolDraw, static_cast<unsigned int>(m_swapchain.framebuffers.size()));
    // Аллокация глобального uniform-буфера
    //m_uniformBufferWorld = InitUnformBufferWorld(m_device);
    // Аллокация uniform-буфера отдельных объектов (динамический буфер)
   // m_uniformBufferModels = InitUniformBufferModels(m_device, m_primitivesMaxCount);
    // Создание дескрипторного пула для выделения основного набора (для unform-буфера)
    //m_descriptorPoolMain = InitDescriptorPoolMain(m_device);
    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    //m_descriptorPoolTextures = InitDescriptorPoolTextures(m_device);
    // Инициализация размещения основного дескрипторного набора
//    m_descriptorSetLayoutMain = InitDescriptorSetLayoutMain(m_device);
    // Инициализация размещения теккстурного набора
//    m_descriptorSetLayoutTextures = InitDescriptorSetLayoutTextures(m_device);
    // Инициализация текстурного семплера
    m_textureSampler = InitTextureSampler(m_device);
    // Инициализация дескрипторного набора
    m_descriptorSetMain = InitDescriptorSetMain(
                m_device,
                m_descriptorPoolMain,
                m_descriptorSetLayoutMain,
                m_uniformBufferWorld,
                m_uniformBufferModels);

    // Аллокация памяти массива ubo-объектов отдельных примитивов
    m_uboModels = AllocateUboModels(m_device, m_primitivesMaxCount);

    // Примитивы синхронизации
    m_sync = InitSynchronization(m_device);
    // Подготовка базовых комманд
    PrepareDrawCommands(
                m_commandBuffersDraw,
                m_renderPass,
                m_pipelineLayout,
                m_descriptorSetMain,
                m_pipeline,
                m_swapchain,
                m_primitives);

    // Готово к рендерингу
    m_isReady = true;
    // Обновить
    Update();
}

/**
* Приостановка рендеринга с ожиданием завершения выполнения всех команд
*/
void KGEVulkanCore::Pause()
{
    // Ожидание завершения всех возможных процессов
    if (m_device.logicalDevice != nullptr) {
        vkDeviceWaitIdle(m_device.logicalDevice);
    }

    m_isRendering = false;
}

/**
* Возврат к состоянию "рендерится"
*/
void KGEVulkanCore::Continue()
{
    m_isRendering = true;
}

/**
* Данный метод вызывается при смене разрешения поверхности отображения
* либо (в дальнейшем) при смене каких-либо ниных настроек графики. В нем происходит
* пересоздание swap-chain'а и всего того, что зависит от измененных настроек
*/
void KGEVulkanCore::VideoSettingsChanged()
{
    // Оставноить выполнение команд
    Pause();
    delete m_kgeVkCommandBuffer;
    delete m_kgeVkGraphicsPipeline;// DeinitGraphicsPipeline(m_device, &m_pipeline);
    delete m_kgeRenderPass;
    m_kgeRenderPass = new KGEVkRenderPass{
            &m_renderPass,
            &m_device,
            m_vkSurface,
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_D32_SFLOAT_S8_UINT};

    // Ре-инициализация swap-cahin.
    // В начале получаем старый swap-chain
    kge::vkstructs::Swapchain oldSwapChain = m_swapchain;
    delete m_kgeSwapChain;

    // Инициализируем обновленный
    m_kgeSwapChain = new KGEVkSwapChain{
            &m_swapchain,
            &m_device,
            m_vkSurface,
    {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            m_renderPass,
            3,
            &oldSwapChain};

    // Инициализация графического конвейера
    m_kgeVkGraphicsPipeline = new KGEVkGraphicsPipeline{&m_pipeline, &m_device, m_pipelineLayout, m_swapchain, m_renderPass};
    // Аллокация командных буферов (получение хендлов)
    m_kgeVkCommandBuffer = new KGEVkCommandBuffer{&m_commandBuffersDraw, &m_device, &m_commandPoolDraw, static_cast<unsigned int>(m_swapchain.framebuffers.size())};

    // Подготовка базовых комманд
    PrepareDrawCommands(
                m_commandBuffersDraw,
                m_renderPass,
                m_pipelineLayout,
                m_descriptorSetMain,
                m_pipeline,
                m_swapchain,
                m_primitives);

    // Снова можно рендерить
    Continue();

    // Обновить
    Update();
}

/**
* В методе отрисовки происходит отправка подготовленных команд а так-же показ
* готовых изображение на поверхности показа
*/
void KGEVulkanCore::Draw()
{
    // Ничего не делать если не готово или приостановлено
    if (!m_isReady || !m_isRendering) {
        return;
    }

    // Индекс доступного изображения
    unsigned int imageIndex;

    // Получить индекс доступного изображения из swap-chain и "включить" семафор сигнализирующий о доступности изображения для рендеринга
    VkResult acquireStatus = vkAcquireNextImageKHR(
                m_device.logicalDevice,
                m_swapchain.vkSwapchain,
                10000,
                m_sync.readyToRender,
                nullptr,
                &imageIndex);

    // Если не получилось получить изображение, вероятно поверхность изменилась или swap-chain более ей не соответствует по каким-либо причинам
    // VK_SUBOPTIMAL_KHR означает что swap-chain еще может быть использован, но в полной мере поверхности не соответствует
    if (acquireStatus != VK_SUCCESS && acquireStatus != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Vulkan: Error. Can't acquire swap-chain image");
    }

    // Данные семафоры будут ожидаться на определенных стадиях ковейера
    std::vector<VkSemaphore> waitSemaphores = { m_sync.readyToRender };

    // Данные семафоры будут "включаться" на определенных стадиях ковейера
    std::vector<VkSemaphore> signalSemaphores = { m_sync.readyToPresent };

    // Стадии конвейера на которых будет происходить одидание семафоров (на i-ой стадии включения i-ого семафора из waitSemaphores)
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    // Информация об отправке команд в буфер
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());       // Кол-во семафоров ожидания
    submitInfo.pWaitSemaphores = waitSemaphores.data();                                 // Семафоры велючение которых будет ожидаться
    submitInfo.pWaitDstStageMask = waitStages;                                          // Стадии на которых конвейер "приостановиться" до включения семафоров
    submitInfo.commandBufferCount = 1;                                                  // Число командных буферов за одну отправку
    submitInfo.pCommandBuffers = &m_commandBuffersDraw[imageIndex];                     // Командный буфер (для текущего изображения в swap-chain)
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());   // Кол-во семафоров сигнала (завершения стадии)
    submitInfo.pSignalSemaphores = signalSemaphores.data();                             // Семафоры которые включатся при завершении

    // Инициировать отправку команд в очередь (на рендеринг)
    VkResult result = vkQueueSubmit(m_device.queues.graphics, 1, &submitInfo, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error. Can't submit commands");
    }

    // Настройка представления (отображение того что отдал конвейер)
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());     // Кол-во ожидаемых семафоров
    presentInfo.pWaitSemaphores = signalSemaphores.data();                  // Cемафоры "включение" которых ожидается перед показом
    presentInfo.swapchainCount = 1;                                         // Кол-во swap-chain'ов
    presentInfo.pSwapchains = &(m_swapchain.vkSwapchain);                   // Указание текущего swap-chain
    presentInfo.pImageIndices = &imageIndex;                                // Индекс текущего изображения, куда осуществляется показ
    presentInfo.pResults = nullptr;

    // Инициировать представление
    VkResult presentStatus = vkQueuePresentKHR(m_device.queues.present, &presentInfo);

    // Представление могло не выполниться если поверхность изменилась или swap-chain более ей не соответствует
    if (presentStatus != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error. Failed to present!");
    }
}

/**
* В методе обновления происходит отправка новых данных в UBO буферы, то есть
* учитываются положения камеры, отдельных примитивов и сцены в целом
*/
void KGEVulkanCore::Update()
{
    // Соотношение сторон (используем размеры поверхности определенные при создании swap-chain)
    m_camera.aspectRatio = (float)(m_swapchain.imageExtent.width) / (float)(m_swapchain.imageExtent.height);

    // Настройка матрицы проекции
    // При помощи данной матрицы происходит проекция 3-мерных точек на плоскость
    // Считается что наблюдатель (камера) в центре системы координат
    m_uboWorld.projectionMatrix = m_camera.MakeProjectionMatrix();

    // Настройка матрицы вида
    // Отвечает за положение и поворот камеры (по сути приводит систему координат мира к системе координат наблюдателя)
    m_uboWorld.viewMatrix = m_camera.MakeViewMatrix();

    // Матрица модели мира
    // Позволяет осуществлять глобальные преобразования всей сцены (пока что не используется)
    m_uboWorld.worldMatrix = glm::mat4();

    // Копировать данные в uniform-буфер
    memcpy(m_uniformBufferWorld.pMapped, &m_uboWorld, static_cast<size_t>((m_uniformBufferWorld.size)));

    // Теперь необходимо обновить динамический буфер формы объектов (если они есть)
    if (!m_primitives.empty()) {

        // Динамическое выравнивание для одного элемента массива
        VkDeviceSize dynamicAlignment = m_device.GetDynamicAlignment<glm::mat4>();

        // Пройтись по всем объектам
        for (unsigned int i = 0; i < m_primitives.size(); i++) {

            // Используя выравнивание получить указатель на нужный элемент массива
            glm::mat4* modelMat = (glm::mat4*)(((uint64_t)(m_uboModels) + (i * dynamicAlignment)));

            // Вписать данные матрицы в элемент
            *modelMat = glm::translate(glm::mat4(), m_primitives[i].position);
            *modelMat = glm::rotate(*modelMat, glm::radians(m_primitives[i].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            *modelMat = glm::rotate(*modelMat, glm::radians(m_primitives[i].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            *modelMat = glm::rotate(*modelMat, glm::radians(m_primitives[i].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // Копировать данные в uniform-буфер
        memcpy(m_uniformBufferModels.pMapped, m_uboModels, static_cast<size_t>(m_uniformBufferModels.size));

        // Гарантировать видимость обновленной памяти устройством
        VkMappedMemoryRange memoryRange = {};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = m_uniformBufferModels.vkDeviceMemory;
        memoryRange.size = m_uniformBufferModels.size;
        vkFlushMappedMemoryRanges(m_device.logicalDevice, 1, &memoryRange);
    }
}

/**
* Настройка параметров перспективы камеры (угол обзора, грани отсечения)
* @param float fFOV - угол обзора
* @param float fNear - ближняя грань отсечения
* @param float fFar - дальняя гран отсечения
*/
void KGEVulkanCore::SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar)
{
    m_camera.fFOV = fFOV;
    m_camera.fNear = fNear;
    m_camera.fFar = fFar;
}

/**
* Настройка положения камеры
* @param float x - положение по оси X
* @param float y - положение по оси Y
* @param float z - положение по оси Z
*/
void KGEVulkanCore::SetCameraPosition(float x, float y, float z)
{
    m_camera.position = glm::vec3(x, y, z);
}

/**
* Настройка поворота камеры
* @param float x - поворот вокруг оси X
* @param float y - поворот вокруг оси Y
* @param float z - поворот вокруг оси Z
*/
void KGEVulkanCore::SetCameraRotation(float x, float y, float z)
{
    m_camera.rotation = glm::vec3(x, y, z);
}

/**
* Добавление нового примитива
* @param const std::vector<vktoolkit::Vertex> &vertices - массив вершин
* @param const std::vector<unsigned int> &indices - массив индексов
* @param glm::vec3 position - положение относительно глобального центра
* @param glm::vec3 rotaton - вращение вокруг локального центра
* @param glm::vec3 scale - масштаб
* @return unsigned int - индекс примитива
*/
unsigned int KGEVulkanCore::AddPrimitive(const std::vector<kge::vkstructs::Vertex> &vertices,
                                         const std::vector<unsigned int> &indices,
                                         const kge::vkstructs::Texture *texture,
                                         glm::vec3 position,
                                         glm::vec3 rotaton,
                                         glm::vec3 scale)
{
    // Новый примитив
    kge::vkstructs::Primitive primitive;
    primitive.position = position;
    primitive.rotation = rotaton;
    primitive.scale = scale;
    primitive.texture = texture;
    primitive.drawIndexed = !indices.empty();

    // Буфер вершин (временный)
    std::vector<kge::vkstructs::Vertex> vertexBuffer = vertices;
    VkDeviceSize vertexBufferSize = (static_cast<unsigned int>(vertexBuffer.size())) * sizeof(kge::vkstructs::Vertex);
    unsigned int vertexCount = static_cast<unsigned int>(vertexBuffer.size());

    // Создать буфер вершин в памяти хоста
    kge::vkstructs::Buffer tmp = kge::vkutility::CreateBuffer(m_device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    primitive.vertexBuffer.vkBuffer = tmp.vkBuffer;
    primitive.vertexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
    primitive.vertexBuffer.size = tmp.size;
    primitive.vertexBuffer.count = vertexCount;

    // Разметить память буфера вершин и скопировать в него данные, после чего убрать разметку
    void * verticesMemPtr;
    vkMapMemory(m_device.logicalDevice, primitive.vertexBuffer.vkDeviceMemory, 0, vertexBufferSize, 0, &verticesMemPtr);
    memcpy(verticesMemPtr, vertexBuffer.data(), static_cast<size_t>(vertexBufferSize));
    vkUnmapMemory(m_device.logicalDevice, primitive.vertexBuffer.vkDeviceMemory);

    // Если необходимо рисовать индексированную геометрию
    if (primitive.drawIndexed) {

        // Буфер индексов (временный)
        std::vector<unsigned int> indexBuffer = indices;
        VkDeviceSize indexBufferSize = (static_cast<unsigned int>(indexBuffer.size())) * sizeof(unsigned int);
        unsigned int indexCount = static_cast<unsigned int>(indexBuffer.size());

        // Cоздать буфер индексов в памяти хоста
        tmp = kge::vkutility::CreateBuffer(m_device,
                                           indexBufferSize,
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        primitive.indexBuffer.vkBuffer = tmp.vkBuffer;
        primitive.indexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
        primitive.indexBuffer.size = tmp.size;
        primitive.indexBuffer.count = indexCount;

        // Разметить память буфера индексов и скопировать в него данные, после чего убрать разметку
        void * indicesMemPtr;
        vkMapMemory(m_device.logicalDevice, primitive.indexBuffer.vkDeviceMemory, 0, indexBufferSize, 0, &indicesMemPtr);
        memcpy(indicesMemPtr, indexBuffer.data(), static_cast<size_t>(indexBufferSize));
        vkUnmapMemory(m_device.logicalDevice, primitive.indexBuffer.vkDeviceMemory);
    }


    // Впихнуть новый примитив в массив
    m_primitives.push_back(primitive);

    // Обновить командный буфер
    ResetCommandBuffers(m_device, m_commandBuffersDraw);
    PrepareDrawCommands(m_commandBuffersDraw,
                        m_renderPass, m_pipelineLayout,
                        m_descriptorSetMain, m_pipeline,
                        m_swapchain,
                        m_primitives);

    // Вернуть индекс
    return static_cast<unsigned int>(m_primitives.size() - 1);
}

/**
* Создание текстуры по данным о пикселях
* @param const unsigned char* pixels - пиксели загруженные из файла
* @return vktoolkit::Texture - структура с набором хендлов изображения и дескриптора
*
* @note - при загрузке используется временный буфер (временное изображение) для перемещения
* в буфер распологающийся в памяти устройства. Нельзя сразу создать буфер в памяти устройства и переместить
* в него данные. Это можно сделать только пр помощи команды копирования (из памяти доступной хосту в память
* доступную только устройству)
*/
kge::vkstructs::Texture KGEVulkanCore::CreateTexture(const unsigned char *pixels,
                                                     uint32_t width,
                                                     uint32_t height,
                                                     uint32_t channels,
                                                     uint32_t bpp)
{
    // Приостановить выполнение основных команд (если какие-либо в процессе)
    this->Pause();

    // Результат
    kge::vkstructs::Texture resultTexture = {};

    // Размер изображения (ожидаем по умолчанию 4 байта на пиксель, в режиме RGBA)
    VkDeviceSize size = static_cast<VkDeviceSize>(width * height * bpp);

    // Если данных не обнаружено
    if (!pixels) {
        throw std::runtime_error("Vulkan: Error while creating texture. Empty pixel buffer recieved");
    }

    // Создать промежуточное изображение
    kge::vkstructs::Image stagingImage = kge::vkutility::CreateImageSingle(
                m_device,
                VK_IMAGE_TYPE_2D,
                VK_FORMAT_R8G8B8A8_UNORM,
    { width,height,1 },
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_PREINITIALIZED,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                VK_IMAGE_TILING_LINEAR);

    // Выбрать подресурс изображения (мип-уровень 0, слой - 0)
    VkImageSubresource subresource = {};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.mipLevel = 0;
    subresource.arrayLayer = 0;

    // Размещение байт в подресурсе
    VkSubresourceLayout stagingImageSubresourceLayout = {};
    vkGetImageSubresourceLayout(m_device.logicalDevice, stagingImage.vkImage, &subresource, &stagingImageSubresourceLayout);

    // Разметить память под изображение
    void* data;
    vkMapMemory(m_device.logicalDevice, stagingImage.vkDeviceMemory, 0, size, 0, &data);

    // Если "ширина строки" равна кол-ву пиксилей по ширине помноженному на bpp - можно исользовать обычный memcpy
    if (stagingImageSubresourceLayout.rowPitch == width * bpp) {
        memcpy(data, pixels, static_cast<unsigned int>(size));
    }
    // Если нет (например размер изображения не кратен степени двойки) - перебираем байты со смещением и копируем каждую стороку
    else {
        unsigned char* dataBytes = reinterpret_cast<unsigned char*>(data);
        for (unsigned int y = 0; y < height; y++) {
            memcpy(&dataBytes[y * (stagingImageSubresourceLayout.rowPitch)],
                    &pixels[y * width * bpp],
                    width * bpp
                    );
        }
    }

    // Убрать разметку памяти
    vkUnmapMemory(m_device.logicalDevice, stagingImage.vkDeviceMemory);

    // Создать финальное изображение (в памяти устройства)
    resultTexture.image = kge::vkutility::CreateImageSingle(
                m_device,
                VK_IMAGE_TYPE_2D,
                VK_FORMAT_R8G8B8A8_UNORM,
    { width,height, 1 },
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_PREINITIALIZED,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_TILING_OPTIMAL);


    // Создать командный буфер для команд перевода размещения изображений
    VkCommandBuffer transitionCmdBuffer = kge::vkutility::CreateSingleTimeCommandBuffer(m_device, m_commandPoolDraw);

    // Подресурс подвергающийся смере размещения в изображениях (описываем его)
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    // Сменить размещение памяти промежуточного изображения в VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    kge::vkutility::CmdImageLayoutTransition(transitionCmdBuffer, stagingImage.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

    // Сменить размещение памяти целевого изображения в VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    kge::vkutility::CmdImageLayoutTransition(transitionCmdBuffer, resultTexture.image.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

    // Выполнить команды перевода размещения
    kge::vkutility::FlushSingleTimeCommandBuffer(m_device, m_commandPoolDraw, transitionCmdBuffer, m_device.queues.graphics);

    // Создать командный буфер для копирования изображения
    VkCommandBuffer copyCmdBuffer = kge::vkutility::CreateSingleTimeCommandBuffer(m_device, m_commandPoolDraw);

    // Копирование из промежуточной картинки в основную
    kge::vkutility::CmdImageCopy(copyCmdBuffer, stagingImage.vkImage, resultTexture.image.vkImage, width, height);

    // Выполнить команды копирования
    kge::vkutility::FlushSingleTimeCommandBuffer(m_device, m_commandPoolDraw, copyCmdBuffer, m_device.queues.graphics);

    // Очистить промежуточное изображение
    stagingImage.Deinit(m_device.logicalDevice);


    // Получить новый набор дескрипторов из дескриптороного пула
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_descriptorPoolTextures;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &(m_descriptorSetLayoutTextures);

    if (vkAllocateDescriptorSets(m_device.logicalDevice, &descriptorSetAllocInfo, &(resultTexture.descriptorSet)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set for texture");
    }

    // Информация о передаваемом изображении
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = resultTexture.image.vkImageView;
    imageInfo.sampler = m_textureSampler;

    // Конфигурация добавляемых в набор дескрипторов
    std::vector<VkWriteDescriptorSet> writes =
    {
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
            nullptr,                                     // pNext
            resultTexture.descriptorSet,                 // Целевой набор дескрипторов
            0,                                           // Точка привязки (у шейдера)
            0,                                           // Элемент массив (массив не используется)
            1,                                           // Кол-во дескрипторов
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   // Тип дескриптора
            &imageInfo,                                  // Информация о параметрах изображения
            nullptr,
            nullptr
        }
    };

    // Обновить наборы дескрипторов
    vkUpdateDescriptorSets(m_device.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    // Исполнение основых команд снова возможно
    this->Continue();

    // Вернуть результат
    return resultTexture;
}

KGEVulkanCore::~KGEVulkanCore()
{
    Pause();
    m_isReady = false;
    // Сброс буферов команд
    ResetCommandBuffers(m_device, m_commandBuffersDraw);

    // Деинициализация примитивов синхронизации
    DeinitSynchronization(m_device, &m_sync);

    // Деинициализация графического конвейера
    //DeinitGraphicsPipeline(m_device, &m_pipeline);

    // Деинициализация графического конвейера
    //DeinitPipelineLayout(&m_device, &m_pipelineLayout);

    // Очистка памяти массива ubo-объектов отдельных примитивов
    FreeUboModels(&(m_uboModels));

    // Деинициализация набора дескрипторов
    DeinitDescriptorSet(m_device, m_descriptorPoolMain, &m_descriptorSetMain);

    // Деинициализация текстурного семплера
    DeinitTextureSampler(m_device, &m_textureSampler);

    // Деинициализация размещения текстурного дескрипторного набора
    //DeinitDescriporSetLayout(m_device, &m_descriptorSetLayoutTextures);

    // Деинициализация размещения основоного дескрипторного набора
   // DeinitDescriporSetLayout(m_device, &m_descriptorSetLayoutMain);

    // Уничтожение ntrcnehyjuj дескрипторного пула
    //DeinitDescriptorPool(m_device, &m_descriptorPoolTextures);

    // Уничтожение основного дескрипторного пула
    //DeinitDescriptorPool(m_device, &m_descriptorPoolMain);

    // Деинициализация uniform-буффера объектов
    //DeinitUniformBuffer(m_device, &m_uniformBufferModels);

    // Деинициализация глобального uniform-буффера
    //DeinitUniformBuffer(m_device, &m_uniformBufferWorld);

    // Деинициализация командных буферов
    //DeinitCommandBuffers(m_device, m_commandPoolDraw, &m_commandBuffersDraw);

    // Деинициализация командного пула
    // DeinitCommandPool(m_device, &m_commandPoolDraw);

    //    // Деинициализация swap-chain'а
    //    DeinitSwapchain(m_device, &m_swapchain);

    // Деинциализация прохода рендеринга
    //DeinitRenderPass(m_device, &m_renderPass);

    //    // Динициализация устройства
    //    DeinitDevice(&m_device);

    //    // Деинициализация поверзности
    //    DeinitWindowSurface(m_instance, &m_vkSurface);

    //    // Деинициализация экземпляра Vulkan
    //    DeinitInstance(&m_instance);
}


/**
* Инициализация текстурного семплера
* @param const kge::vkstructs::Device &device - устройство
* @note - описывает как данные текстуры подаются в шейдер и как интерпретируются координаты
*/
VkSampler KGEVulkanCore::InitTextureSampler(const kge::vkstructs::Device &device)
{
    // Результирующий хендл семплера
    VkSampler resultSampler;

    // Настройка семплера
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели больше фрагментов
    samplerInfo.minFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели меньше фрагментов
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;     // Повторять при выходе за пределы
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;                        // Включть анизотропную фильтрацию
    samplerInfo.maxAnisotropy = 4;                                 // уровень фильтрации
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // Цвет грани
    samplerInfo.unnormalizedCoordinates = VK_FALSE;                // Использовать нормальзованные координаты (не пиксельные)
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    // Создание семплера
    if (vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr, &resultSampler) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating texture sampler");
    }

    kge::tools::LogMessage("Vulkan: Texture sampler successfully initialized");


    return resultSampler;
}

/**
* Деинициализация текстурного семплера
* @param const kge::vkstructs::Device &device - устройство
* @param VkSampler * sampler - деинициализация текстурного семплера
*/
void KGEVulkanCore::DeinitTextureSampler(const kge::vkstructs::Device &device,
                                         VkSampler *sampler)
{
    if (sampler != nullptr && *sampler != nullptr) {
        vkDestroySampler(device.logicalDevice, *sampler, nullptr);
        *sampler = nullptr;
    }
}

/**
* Инициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSetLayout descriptorSetLayout - хендл размещения дескрипторно набора
* @param const vktoolkit::UniformBuffer &uniformBufferWorld - буфер содержит необходимую для создания дескриптора информацию
* @param const vktoolkit::UniformBuffer &uniformBufferModels - буфер содержит необходимую для создания дескриптора информацию
*/
VkDescriptorSet KGEVulkanCore::InitDescriptorSetMain(const kge::vkstructs::Device &device,
                                                     VkDescriptorPool descriptorPool,
                                                     VkDescriptorSetLayout descriptorSetLayout,
                                                     const kge::vkstructs::UniformBuffer &uniformBufferWorld,
                                                     const kge::vkstructs::UniformBuffer &uniformBufferModels)
{
    // Результирующий хендл дескриптора
    VkDescriptorSet descriptorSetResult = nullptr;

    // Получить новый набор дескрипторов из дескриптороного пула
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
    }

    // Конфигурация добавляемых в набор дескрипторов
    std::vector<VkWriteDescriptorSet> writes =
    {
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
            nullptr,                                     // pNext
            descriptorSetResult,                         // Целевой набор дескрипторов
            0,                                           // Точка привязки (у шейдера)
            0,                                           // Элемент массив (массив не используется)
            1,                                           // Кол-во дескрипторов
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // Тип дескриптора
            nullptr,
            &(uniformBufferWorld.descriptorBufferInfo),  // Информация о параметрах буфера
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
            nullptr,                                     // pNext
            descriptorSetResult,                         // Целевой набор дескрипторов
            1,                                           // Точка привязки (у шейдера)
            0,                                           // Элемент массив (массив не используется)
            1,                                           // Кол-во дескрипторов
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   // Тип дескриптора
            nullptr,
            &(uniformBufferModels.descriptorBufferInfo), // Информация о параметрах буфера
            nullptr,
        },
    };

    // Обновить наборы дескрипторов
    vkUpdateDescriptorSets(device.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    kge::tools::LogMessage("Vulkan: Descriptor set successfully initialized");

    // Вернуть хендл набора
    return descriptorSetResult;
}
/**
* Деинициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSet * descriptorSet - указатель на хендл набора дескрипторов
*/
void KGEVulkanCore::DeinitDescriptorSet(const kge::vkstructs::Device &device,
                                        VkDescriptorPool descriptorPool,
                                        VkDescriptorSet *descriptorSet)
{
    if (device.logicalDevice    != nullptr
            && descriptorPool   != nullptr
            && descriptorSet    != nullptr
            && *descriptorSet   != nullptr)
    {
        if (vkFreeDescriptorSets(device.logicalDevice, descriptorPool, 1, descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Vulkan: Error while destroying descriptor set");
        }
        *descriptorSet = nullptr;

        kge::tools::LogMessage("Vulkan: Descriptor set successfully deinitialized");
    }
}

/**
* Аллокация памяти под объект динамического UBO буфера
* @param const vktoolkit::Device &device - устройство (необходимо для получения выравнивания)
* @param unsigned int maxObjects - максимальное кол-вл объектов (для выяснения размера аллоцируемой памяти)
* @return vktoolkit::UboModelArray - указатель на аллоцированный массив матриц
*/
kge::vkstructs::UboModelArray KGEVulkanCore::AllocateUboModels(const kge::vkstructs::Device &device,
                                                               unsigned int maxObjects)
{
    // Получить оптимальное выравнивание для типа glm::mat4
    std::size_t dynamicAlignment = static_cast<size_t>(device.GetDynamicAlignment<glm::mat4>());

    // Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
    std::size_t bufferSize = static_cast<size_t>(dynamicAlignment * maxObjects);

    // Аллоцировать память с учетом выравнивания
    kge::vkstructs::UboModelArray result = static_cast<kge::vkstructs::UboModelArray>(aligned_alloc(bufferSize, dynamicAlignment));

    kge::tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully allocated");

    return result;
}

/**
* Освобождение памяти объекта динамического UBO буфера
* @param vktoolkit::UboModelArray * uboModels - указатель на массив матриц, память которого будет очищена
*/
void KGEVulkanCore::FreeUboModels(kge::vkstructs::UboModelArray *uboModels)
{
    free(*uboModels);
    *uboModels = nullptr;
    kge::tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully freed");
}

/**
* Инициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @return vktoolkit::Synchronization - структура с набором хендлов семафоров
* @note - семафоры синхронизации позволяют отслеживать состояние рендеринга и в нужный момент показывать изображение
*/
kge::vkstructs::Synchronization KGEVulkanCore::InitSynchronization(const kge::vkstructs::Device &device)
{
    // Результат
    kge::vkstructs::Synchronization syncResult = {};

    // Информация о создаваемом семафоре (ничего не нужно указывать)
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Создать примитивы синхронизации
    if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToRender)) != VK_SUCCESS ||
            vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToPresent)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating synchronization primitives");
    }

    kge::tools::LogMessage("Vulkan: Synchronization primitives sucessfully initialized");

    return syncResult;
}

/**
* Деинициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @param vktoolkit::Synchronization * sync - указатель на структуру с хендлами семафоров
*/
void KGEVulkanCore::DeinitSynchronization(const kge::vkstructs::Device &device,
                                          kge::vkstructs::Synchronization *sync)
{
    if (sync != nullptr) {
        if (sync->readyToRender != nullptr) {
            vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
            sync->readyToRender = nullptr;
        }

        if (sync->readyToRender != nullptr) {
            vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
            sync->readyToRender = nullptr;
        }

        kge::tools::LogMessage("Vulkan: Synchronization primitives sucessfully deinitialized");
    }
}

/**
* Подготовка команд, заполнение командных буферов
* @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов
* @param VkRenderPass renderPass - хендл прохода рендеринга, используется при привязке прохода
* @param VkPipelineLayout pipelineLayout - хендл размещения конвейра, исппользуется при привязке дескрипторов
* @param VkDescriptorSet descriptorSet - хендл набор дескрипторов, исппользуется при привязке дескрипторов
* @param VkPipeline pipeline - хендл конвейера, используется при привязке конвейера
* @param const vktoolkit::Swapchain &swapchain - свопчейн, используется при конфигурации начала прохода (в.ч. для указания фрейм-буфера)
* @param const std::vector<vktoolkit::Primitive> &primitives - массив примитивов (привязка буферов вершин, буферов индексов для каждого и т.д)
*
* @note - данную операцию нет нужды выполнять при каждом обновлении кадра, набор команд как правило относительно неизменный. Метод лишь заполняет
* буферы команд, а сама отправка происходть в draw. При изменении кол-ва примитивов следует сбросить командные буферы и заново их заполнить
* при помощи данного метода
*/
void KGEVulkanCore::PrepareDrawCommands(std::vector<VkCommandBuffer> commandBuffers, VkRenderPass renderPass, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSetMain, VkPipeline pipeline, const kge::vkstructs::Swapchain &swapchain, const std::vector<kge::vkstructs::Primitive> &primitives)
{
    // Информация начала командного буфера
    VkCommandBufferBeginInfo cmdBufInfo = {};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    cmdBufInfo.pNext = nullptr;

    // Параметры очистки вложений в начале прохода
    std::vector<VkClearValue> clearValues(2);
    // Очистка первого вложения (цветового)
    clearValues[0].color = { {0.6f, 0.8f, 0.8f, 1.0f} };
    // Очиска второго вложения (вложения глубины-трфарета)
    clearValues[1].depthStencil.depth = 1.0f;
    clearValues[1].depthStencil.stencil = 0;

    // Информация о начале прохода
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = swapchain.imageExtent.width;
    renderPassBeginInfo.renderArea.extent.height = swapchain.imageExtent.height;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();


    // Пройтись по всем буферам
    for (unsigned int i = 0; i < commandBuffers.size(); ++i)
    {
        // Начать запись команд в командный буфер i
        vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

        // Установить целевой фрейм-буфер (поскольку их кол-во равно кол-ву командных буферов, индексы соответствуют)
        renderPassBeginInfo.framebuffer = swapchain.framebuffers[i];

        // Начать первый под-проход основного прохода, это очистит цветоые вложения
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Привязать графический конвейер
        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // Пройтись по всем примитивам
        if (!primitives.empty()) {
            for (unsigned int primitiveIndex = 0; primitiveIndex < primitives.size(); primitiveIndex++)
            {
                // Спиок динамических смещений для динамических UBO буферов в наборах дескрипторов
                // При помощи выравниваниях получаем необходимое смещение для дескрипторов, чтобы была осуществлена привязка
                // нужного буфера UBO (с матрицей модели) для конкретного примитива
                std::vector<uint32_t> dynamicOffsets = {
                    primitiveIndex * static_cast<uint32_t>(m_device.GetDynamicAlignment<glm::mat4>())
                };

                // Наборы дескрипторов (массив наборов)
                // По умолчанию в нем только основной
                std::vector<VkDescriptorSet> descriptorSets = {
                    descriptorSetMain
                };

                // Если у примитива есть текстура
                // Добавить в список дескрипторов еще один (отвечающий за подачу текстуры и параметров семплинга в шейдер)
                if (primitives[primitiveIndex].texture != nullptr) {
                    descriptorSets.push_back(primitives[primitiveIndex].texture->descriptorSet);
                }

                // Привязать наборы дескрипторов
                vkCmdBindDescriptorSets(
                            commandBuffers[i],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            0,
                            static_cast<uint32_t>(descriptorSets.size()),
                            descriptorSets.data(),
                            static_cast<uint32_t>(dynamicOffsets.size()),
                            dynamicOffsets.data());

                // Если нужно рисовать индексированную геометрию
                if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
                    // Привязать буфер вершин
                    VkDeviceSize offsets[1] = { 0 };
                    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

                    // Привязать буфер индексов
                    vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

                    // Отрисовка геометрии
                    vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
                }
                // Если индексация вершин не используется
                else {
                    // Привязать буфер вершин
                    VkDeviceSize offsets[1] = { 0 };
                    vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

                    // Отрисовка
                    vkCmdDraw(commandBuffers[i], primitives[primitiveIndex].vertexBuffer.count, 1, 0, 0);
                }

            }
        }

        // Завершение прохода
        vkCmdEndRenderPass(commandBuffers[i]);

        // Завершение прохода добавит неявное преобразование памяти фрейм-буфера в
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR для представления содержимого

        // Завершение записи команд
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Vulkan: Error while preparing commands");
        }
    }
}

/**
* Сброс командных буферов (для перезаписи)
* @param const vktoolkit::Device &device - устройство, для получения хендлов очередей
* @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов, которые необходимо сбросить
*/
void KGEVulkanCore::ResetCommandBuffers(const kge::vkstructs::Device &device, std::vector<VkCommandBuffer> commandBuffers)
{
    // Приостановить рендеринг
    m_isReady = false;

    // Подождать завершения всех команд в очередях
    if (device.queues.graphics != nullptr && device.queues.present != nullptr) {
        vkQueueWaitIdle(device.queues.graphics);
        vkQueueWaitIdle(device.queues.present);
    }

    // Сбросить буферы команд
    if (!commandBuffers.empty()) {
        for (const VkCommandBuffer &buffer : commandBuffers) {
            if (vkResetCommandBuffer(buffer, 0) != VK_SUCCESS) {
                throw std::runtime_error("Vulkan: Error while resetting commad buffers");
            }
        }
    }

    // Снова готово к рендерингу
    m_isReady = true;
}
