#include "graphic/KGEVulkanCore.h"
#include <cstdlib>
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
    m_width(width),
    m_heigh(heigh),
    // Инициализация экземпляра
    m_kgeVkInstance{applicationName, "KGEngine", instanceExtensionsRequired, validationLayersRequired},
    m_vkInstance(m_kgeVkInstance.vkInstance()),
    // Инициализация поверхности отображения
    m_kgeVkSurface{windowControl, m_vkInstance},
    m_vkSurface{m_kgeVkSurface.vkSurface()},
    // Инициализация устройства
    m_device{},
    m_kgeVkDevice{&m_device, m_vkInstance, m_vkSurface, deviceExtensionsRequired, validationLayersRequired, false},

    m_renderPass(nullptr),
    m_commandPoolDraw(nullptr),
    m_descriptorSetLayoutMain(nullptr),
    m_descriptorSetLayoutTextures(nullptr),
    m_descriptorSetMain(nullptr),
    m_pipelineLayout(nullptr),
    m_pipeline(nullptr),
    m_primitivesMaxCount(primitivesMaxCount),
    m_uboModels(nullptr)
{
    // Присвоить параметры камеры по умолчанию
    m_camera.fFar  = DEFAULT_FOV;
    m_camera.fFar  = DEFAULT_FAR;
    m_camera.fNear = DEFAULT_NEAR;


    // Инициализация устройства
//    m_device     = InitDevice( m_instance, m_vkSurface, deviceExtensionsRequired, validationLayersRequired, false);
    // Инициализация прохода рендеринга
    m_renderPass = InitRenderPass(m_device, m_vkSurface, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);
    // Инициализация swap-chain
    m_swapchain = InitSwapChain(m_device, m_vkSurface, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, m_renderPass, 3, nullptr);
    // Инициализация командного пула
    m_commandPoolDraw = InitCommandPool(m_device, static_cast<unsigned int>(m_device.queueFamilies.graphics));
    // Аллокация командных буферов (получение хендлов)
    m_commandBuffersDraw = InitCommandBuffers(m_device, m_commandPoolDraw, static_cast<unsigned int>(m_swapchain.framebuffers.size()));
    // Аллокация глобального uniform-буфера
    m_uniformBufferWorld = InitUnformBufferWorld(m_device);
    // Аллокация uniform-буфера отдельных объектов (динамический буфер)
    m_uniformBufferModels = InitUniformBufferModels(m_device, m_primitivesMaxCount);
    // Создание дескрипторного пула для выделения основного набора (для unform-буфера)
    m_descriptorPoolMain = InitDescriptorPoolMain(m_device);
    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    m_descriptorPoolTextures = InitDescriptorPoolTextures(m_device);
    // Инициализация размещения основного дескрипторного набора
    m_descriptorSetLayoutMain = InitDescriptorSetLayoutMain(m_device);
    // Инициализация размещения теккстурного набора
    m_descriptorSetLayoutTextures = InitDescriptorSetLayoutTextures(m_device);
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
    // Инициализация размещения графического конвейера
    m_pipelineLayout = InitPipelineLayout(m_device, { m_descriptorSetLayoutMain, m_descriptorSetLayoutTextures});
    // Инициализация графического конвейера
    m_pipeline = InitGraphicsPipeline(m_device, m_pipelineLayout, m_swapchain, m_renderPass);
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

    // В начале деинициализировать компоненты зависимые от swap-chain
    DeinitCommandBuffers(m_device, m_commandPoolDraw, &m_commandBuffersDraw);
    DeinitGraphicsPipeline(m_device, &m_pipeline);

    // Render pass не зависит от swap-chain, но поскольку поверхность могла сменить свои свойства - следует пересоздать
    // по новой, проверив формат цветового вложения
    DeinitRenderPass(m_device, &m_renderPass);
    m_renderPass = InitRenderPass(m_device, m_vkSurface, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

    // Ре-инициализация swap-cahin.
    // В начале получаем старый swap-chain
    kge::vkstructs::Swapchain oldSwapChain = m_swapchain;
    // Инициализируем обновленный
    m_swapchain = InitSwapChain(
                m_device,
                m_vkSurface,
    { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                m_renderPass,
                3,
                &oldSwapChain);

    // Уничтожаем старый
    DeinitSwapchain(m_device, &(oldSwapChain));

    // Инициализация графического конвейера
    m_pipeline = InitGraphicsPipeline(m_device, m_pipelineLayout, m_swapchain, m_renderPass);

    // Аллокация командных буферов (получение хендлов)
    m_commandBuffersDraw = InitCommandBuffers(m_device,
                                              m_commandPoolDraw,
                                              static_cast<unsigned int>(m_swapchain.framebuffers.size()));

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
        memcpy(m_uniformBufferModels.pMapped, m_uboModels, (size_t)(m_uniformBufferModels.size));

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
        tmp = kge::vkutility::CreateBuffer(m_device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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
    DeinitGraphicsPipeline(m_device, &m_pipeline);

    // Деинициализация графического конвейера
    DeinitPipelineLayout(m_device, &m_pipelineLayout);

    // Очистка памяти массива ubo-объектов отдельных примитивов
    FreeUboModels(&(m_uboModels));

    // Деинициализация набора дескрипторов
    DeinitDescriptorSet(m_device, m_descriptorPoolMain, &m_descriptorSetMain);

    // Деинициализация текстурного семплера
    DeinitTextureSampler(m_device, &m_textureSampler);

    // Деинициализация размещения текстурного дескрипторного набора
    DeinitDescriporSetLayout(m_device, &m_descriptorSetLayoutTextures);

    // Деинициализация размещения основоного дескрипторного набора
    DeinitDescriporSetLayout(m_device, &m_descriptorSetLayoutMain);

    // Уничтожение ntrcnehyjuj дескрипторного пула
    DeinitDescriptorPool(m_device, &m_descriptorPoolTextures);

    // Уничтожение основного дескрипторного пула
    DeinitDescriptorPool(m_device, &m_descriptorPoolMain);

    // Деинициализация uniform-буффера объектов
    DeinitUniformBuffer(m_device, &m_uniformBufferModels);

    // Деинициализация глобального uniform-буффера
    DeinitUniformBuffer(m_device, &m_uniformBufferWorld);

    // Деинициализация командных буферов
    DeinitCommandBuffers(m_device, m_commandPoolDraw, &m_commandBuffersDraw);

    // Деинициализация командного пула
    DeinitCommandPool(m_device, &m_commandPoolDraw);

    // Деинициализация swap-chain'а
    DeinitSwapchain(m_device, &m_swapchain);

    // Деинциализация прохода рендеринга
    DeinitRenderPass(m_device, &m_renderPass);

//    // Динициализация устройства
//    DeinitDevice(&m_device);

//    // Деинициализация поверзности
//    DeinitWindowSurface(m_instance, &m_vkSurface);

//    // Деинициализация экземпляра Vulkan
//    DeinitInstance(&m_instance);
}

/**
* Инициализация прохода рендеринга.
* @param const vktoolkit::Device &device - устройство
* @param VkSurfaceKHR surface - хендл поверхности, передается лишь для проверки поддержки запрашиваемого формата
* @param VkFormat colorAttachmentFormat - формат цветовых вложений/изображений, должен поддерживаться поверхностью
* @param VkFormat depthStencilFormat - формат вложений глубины, должен поддерживаться устройством
* @return VkRenderPass - хендл прохода рендеринга
*
* @note - Проход рендеринга можно понимать как некую стадию на которой выполняются все команды рендерига и происходит цикл конвейера
* Проход состоит из под-проходов, и у каждого под-прохода может быть своя конфигурация конвейера. Конфигурация же прохода
* определяет в каком состоянии (размещении памяти) будет вложение (цветовое, глубины и тд)
*/
VkRenderPass KGEVulkanCore::InitRenderPass(const kge::vkstructs::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat)
{
    // Проверка доступности формата вложений (изображений)
    kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(device.physicalDevice, surface);
    if (!si.IsFormatSupported(colorAttachmentFormat)) {
        throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize render-pass");
    }

    // Проверка доступности формата глубины
    if (!device.IsDepthFormatSupported(depthStencilFormat)) {
        throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
    }

    // Массив описаний вложений
    // Пояснение: изображения в которые происходит рендеринг либо которые поступают на вход прохода называются вложениями.
    // Это может быть цветовое вложение (по сути обычное изображение) либо вложение глубины, трафарета и т.д.
    std::vector<VkAttachmentDescription> attachments;

    // Описаниие цветового вложения (изображение)
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = colorAttachmentFormat;                                       // Формат цвета должен соответствовать тому что будет использован при создании своп-чейна
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                                      // Не использовать мультисемплинг (один семпл на пиксел)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                                 // На этапе начала прохода - очищать вложение
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                               // На этапе конца прохода - хранить вложение (для дальнешей презентации)
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                      // Подресурс трафарета (начало прохода) - не используется
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                    // Подресурс трафарета (конце прохода) - не исрользуется
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                            // Размещение памяти в начале (не имеет значения, любое)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                        // Размещение памяти к которому вложение будет приведено после окончания прохода (для представления)
    attachments.push_back(colorAttachment);

    // Описание вложения глубины трафарета (z-буфер)
    VkAttachmentDescription depthStencilAttachment = {};
    depthStencilAttachment.format = depthStencilFormat;
    depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // На этапе начала прохода - очищать вложение
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // На этапе конца прохода - не имеет значение (память не используется для презентации, можно не хранить)
    depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // Подресурс трафарета (начало прохода) - не используется
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Подресурс трафарета (конце прохода) - не исрользуется
    depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Размещение памяти в начале (не имеет значения, любое)
    depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Размещение памяти к которому вложение будет приведено после окончания прохода (глубина-трафарет)
    attachments.push_back(depthStencilAttachment);


    // Массив ссылок на цветовые вложения
    // Для каждой ссылки указывается индекс цветового вложения в массиве вложений и ожидаемое размещение
    std::vector<VkAttachmentReference> colorAttachmentReferences = {
        {
            0,                                                       // Первый элемент массива вложений
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                 // Ожидается что размещение будет VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // Ссылка на вложение глубины-трафарета
    VkAttachmentReference depthStencilAttachemntReference = {
        1,                                                           // Второй элемент массива вложений
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL             // Ожидается что размещение будет VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // Описание единственного под-прохода
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();  // Кол-во цветовых вложений
    subpassDescription.pColorAttachments = colorAttachmentReferences.data();               // Цветовые вложения (вложения для записи)
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         // Глубина-трафарет (не используется)
    subpassDescription.inputAttachmentCount = 0;                                           // Кол-во входных вложений (не используются)
    subpassDescription.pInputAttachments = nullptr;                                        // Входные вложения (вложения для чтения, напр. того что было записано в пред-щем под-проходе)
    subpassDescription.preserveAttachmentCount = 0;                                        // Кол-во хранимых вложений (не используется)
    subpassDescription.pPreserveAttachments = nullptr;                                     // Хранимые вложения могут быть использованы для много-кратного использования в разных под-проходах
    subpassDescription.pResolveAttachments = nullptr;                                      // Resolve-вложения (полезны при работе с мульти-семплингом, не используется)

    // Настройка зависимостей под-проходов
    // Они добавят неявный шаблон перехода вложений
    // Каждая зависимость под-прохода подготовит память во время переходов вложения между под-проходами
    // Пояснение: VK_SUBPASS_EXTERNAL ссылается на внешний, неявный под-проход
    std::vector<VkSubpassDependency> dependencies = {
        // Первая зависимость, начало конвейера
        // Перевод размещения от заключительного (final) к первоначальному (initial)
        {
            VK_SUBPASS_EXTERNAL,                                                       // Передыдущий под-проход - внешний, неявный
            0,                                                                         // Следующий под-проход - первый (и единсвтенный)
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Подразумевается что предыдыщий под-проход (внешний) завершен на этапе завершения конвейера
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Целевой подпроход (первый) начнется на этапе вывода цветовой информации из конвейера
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        // Вторая зависимость, выход - конец конвейера
        // Перевод размещения от первоначального (initial) к заключительному (final)
        {
            0,                                                                         // Предыдщий под-проход - первый (и единсвтенный)
            VK_SUBPASS_EXTERNAL,                                                       // Следующий - внешний, неявный
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Предыдущий под-проход (первый) завершен на этапе вывода цветовой информации
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Следующий под-проход (внешний) начат на этапе завершения конвейера
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
    };

    // Создать проход
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = (unsigned int)attachments.size();              //Кол-во описаний вложений
    renderPassInfo.pAttachments = attachments.data();                               //Описания вложений
    renderPassInfo.subpassCount = 1;                                                //Кол-вл под-проходов
    renderPassInfo.pSubpasses = &subpassDescription;                                //Описание под-проходов
    renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //Кол-во зависимсотей
    renderPassInfo.pDependencies = dependencies.data();                             //Зависимости

    //Создание прохода
    VkRenderPass renderPass;
    if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Failed to create render pass!");
    }

    kge::tools::LogMessage("Vulkan: Render pass successfully initialized");

    return renderPass;
}

void KGEVulkanCore::DeinitRenderPass(const kge::vkstructs::Device &device,
                                     VkRenderPass *renderPass)
{
    // Если создан проход рендера - уничтожить
    if (*renderPass != nullptr) {
        vkDestroyRenderPass(device.logicalDevice, *renderPass, nullptr);
        *renderPass = nullptr;
        kge::tools::LogMessage("Vulkan: Render pass successfully deinitialized");
    }
}

/**
* Swap-chain (список показа, цепочка свопинга) - представляет из себя набор сменяемых изображений
* @param const kge::vkstructs::Device &device - устройство, необходимо для создания
* @param VkSurfaceKHR surface - хкндл поверхоности, необходимо для создания объекта swap-chain и для проверки поддержки формата
* @param VkSurfaceFormatKHR surfaceFormat - формат изображений и цветовое пространство (должен поддерживаться поверхностью)
* @param VkFormat depthStencilFormat - формат вложений глубины (должен поддерживаться устройством)
* @param VkRenderPass renderPass - хендл прохода рендеринга, нужен для создания фрейм-буферов swap-chain
* @param unsigned int bufferCount - кол-во буферов кадра (напр. для тройной буферизации - 3)
* @param kge::vkstructs::Swapchain * oldSwapchain - передыдуший swap-chain (полезно в случае пересоздания свап-чейна, например, сменив размеро поверхности)
* @return kge::vkstructs::Swapchain структура описывающая swap-chain cодержащая необходимые хендлы
* @note - в одно изображение может происходить запись (рендеринг) в то время как другое будет показываться (презентация)
*/
kge::vkstructs::Swapchain KGEVulkanCore::InitSwapChain(
        const kge::vkstructs::Device &device,
        VkSurfaceKHR surface,
        VkSurfaceFormatKHR surfaceFormat,
        VkFormat depthStencilFormat,
        VkRenderPass renderPass,
        unsigned int bufferCount,
        kge::vkstructs::Swapchain * oldSwapchain)
{
    // Возвращаемый результат (структура содержит хендлы свопчейна, изображений, фрейм-буферов и тд)
    kge::vkstructs::Swapchain resultSwapchain;

    // Информация о поверхности
    kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(device.physicalDevice, surface);

    // Проверка доступности формата и цветового пространства изображений
    if (!si.IsSurfaceFormatSupported(surfaceFormat)) {
        throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize swap-chain");
    }

    // Проверка доступности формата глубины
    if (!device.IsDepthFormatSupported(depthStencilFormat)) {
        throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
    }

    // Если кол-во буферов задано
    if (bufferCount > 0) {
        // Проверить - возможно ли использовать запрашиваемое кол-во буферов (и изоображений соответственно)
        if (bufferCount < si.capabilities.minImageCount || bufferCount > si.capabilities.maxImageCount) {
            std::string message = "Vulkan: Surface don't support " + std::to_string(bufferCount) + " images/buffers in swap-chain";
            throw std::runtime_error(message);
        }
    }
    // В противном случае попытаться найти оптимальное кол-во
    else {
        bufferCount = (si.capabilities.minImageCount + 1) > si.capabilities.maxImageCount ? si.capabilities.maxImageCount : (si.capabilities.minImageCount + 1);
    }

    // Выбор режима представления (FIFO_KHR по умолчнию, самый простой)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Если запрашиваемое кол-во буферов больше единицы - есть смысл выбрать более сложный режим,
    // но перед этим необходимо убедиться что поверхность его поддерживает
    if (bufferCount > 1) {
        for (const VkPresentModeKHR& availablePresentMode : si.presentModes) {
            //Если возможно - использовать VK_PRESENT_MODE_MAILBOX_KHR (вертикальная синхронизация)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                break;
            }
        }
    }

    // Информация о создаваемом swap-chain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = m_vkSurface;
    swapchainCreateInfo.minImageCount = bufferCount;                        // Минимальное кол-во изображений
    swapchainCreateInfo.imageFormat = surfaceFormat.format;					// Формат изображения
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;			// Цветовое пространство
    swapchainCreateInfo.imageExtent = si.capabilities.currentExtent;        // Резрешение (расширение) изображений
    swapchainCreateInfo.imageArrayLayers = 1;                               // Слои (1 слой, не стереоскопический рендер)
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // Как используются изображения (как цветовые вложения)

    // Добавить информацию о формате и расширении в результирующий объект swap-chain'а (он будет отдан функцией)
    resultSwapchain.imageFormat = swapchainCreateInfo.imageFormat;
    resultSwapchain.imageExtent = swapchainCreateInfo.imageExtent;

    // Если старый swap-chain был передан - очистить его информацию о формате и расширении
    if (oldSwapchain != nullptr) {
        oldSwapchain->imageExtent = {};
        oldSwapchain->imageFormat = {};
    }

    // Индексы семейств
    std::vector<unsigned int> queueFamilyIndices = {
        static_cast<unsigned int>(device.queueFamilies.graphics),
        static_cast<unsigned int>(device.queueFamilies.present)
    };

    // Если для команд графики и представления используются разные семейства, значит доступ к ресурсу (в данном случае к буферам изображений)
    // должен быть распараллелен (следует использовать VK_SHARING_MODE_CONCURRENT, указав при этом кол-во семейств и их индексы)
    if (device.queueFamilies.graphics != device.queueFamilies.present) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    // В противном случае подходящим будет VK_SHARING_MODE_EXCLUSIVE (с ресурсом работают команды одного семейства)
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = si.capabilities.currentTransform;                                        // Не используем трансформацмию изображения (трансформация поверхности по умолчнию) (???)
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                                     // Смешивание альфа канала с другими окнами в системе (нет смешивания)
    swapchainCreateInfo.presentMode = presentMode;                                                              // Установка режима представления (тот что выбрали ранее)
    swapchainCreateInfo.clipped = VK_TRUE;                                                                      // Не рендерить перекрываемые другими окнами пиксели
    swapchainCreateInfo.oldSwapchain = (oldSwapchain != nullptr ? oldSwapchain->vkSwapchain : nullptr);  // Старый swap-chain (для более эффективного пересоздания можно указывать старый swap-chain)

    // Создание swap-chain (записать хендл в результирующий объект)
    if (vkCreateSwapchainKHR(device.logicalDevice, &swapchainCreateInfo, nullptr, &(resultSwapchain.vkSwapchain)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateSwapchainKHR function. Failed to create swapchain");
    }

    // Уничтожение предыдущего swap-chain, если он был передан
    if (oldSwapchain != nullptr) {
        vkDestroySwapchainKHR(device.logicalDevice, oldSwapchain->vkSwapchain, nullptr);
        oldSwapchain->vkSwapchain = nullptr;
    }

    // Получить хендлы изображений swap-chain
    // Кол-во изображений по сути равно кол-ву буферов (за это отвечает bufferCount при создании swap-chain)
    unsigned int swapChainImageCount = 0;
    vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, nullptr);
    resultSwapchain.images.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, resultSwapchain.images.data());

    // Теперь необходимо создать image-views для каждого изображения (своеобразный интерфейс объектов изображений предостовляющий нужные возможности)
    // Если был передан старый swap-chain - предварительно очистить все его image-views
    if (oldSwapchain != nullptr) {
        if (!oldSwapchain->imageViews.empty()) {
            for (VkImageView const &swapchainImageView : oldSwapchain->imageViews) {
                vkDestroyImageView(device.logicalDevice, swapchainImageView, nullptr);
            }
            oldSwapchain->imageViews.clear();
        }
    }

    // Для каждого изображения (image) swap-chain'а создать свой imageView объект
    for (unsigned int i = 0; i < resultSwapchain.images.size(); i++) {

        // Идентификатор imageView (будт добавлен в массив)
        VkImageView swapChainImageCount;

        // Информация для инициализации
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = resultSwapchain.images[i];                       // Связь с изображением swap-chain
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                        // Тип изображения (2Д текстура)
        createInfo.format = surfaceFormat.format;							// Формат изображения
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// По умолчанию
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Создать и добавить в массив
        if (vkCreateImageView(device.logicalDevice, &createInfo, nullptr, &swapChainImageCount) == VK_SUCCESS) {
            resultSwapchain.imageViews.push_back(swapChainImageCount);
        }
        else {
            throw std::runtime_error("Vulkan: Error in vkCreateImageView function. Failed to create image views");
        }
    }


    // Буфер глубины (Z-буфер)
    // Буфер может быть один для всех фрейм-буферов, даже при двойной/тройной буферизации (в отличии от изображений swap-chain)
    // поскольку он не учавствует в презентации (память из него непосредственно не отображается на экране).

    // Если это пересоздание swap-chain (передан старый) следует очистить компоненты прежнего буфера глубины
    if (oldSwapchain != nullptr) {
        oldSwapchain->depthStencil.Deinit(device.logicalDevice);
    }

    // Создать буфер глубины-трафарета (обычное 2D-изображение с требуемым форматом)
    resultSwapchain.depthStencil = kge::vkutility::CreateImageSingle(
                device,
                VK_IMAGE_TYPE_2D,
                depthStencilFormat,
    { si.capabilities.currentExtent.width, si.capabilities.currentExtent.height, 1 },
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                swapchainCreateInfo.imageSharingMode);


    // Теперь необходимо создать фрейм-буферы привязанные к image-views объектам изображений и буфера глубины (изображения глубины)
    // Перед этим стоит очистить буферы старого swap-сhain (если он был передан)
    if (oldSwapchain != nullptr) {
        if (!oldSwapchain->framebuffers.empty()) {
            for (VkFramebuffer const &frameBuffer : oldSwapchain->framebuffers) {
                vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
            }
            oldSwapchain->framebuffers.clear();
        }
    }

    // Пройтись по всем image views и создать фрейм-буфер для каждого
    for (unsigned int i = 0; i < resultSwapchain.imageViews.size(); i++) {

        // Хендл нового фреймбуфера
        VkFramebuffer framebuffer;

        std::vector<VkImageView> attachments(2);
        attachments[0] = resultSwapchain.imageViews[i];                             // Цветовое вложение (на каждый фрейм-буфер свое)
        attachments[1] = resultSwapchain.depthStencil.vkImageView;                  // Буфер глубины (один на все фрейм-буферы)

        // Описание нового фреймбуфера
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;                          // Указание прохода рендера
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());   // Кол-во вложений
        framebufferInfo.pAttachments = attachments.data();                // Связь с image-views объектом изображения swap-chain'а
        framebufferInfo.width = resultSwapchain.imageExtent.width;        // Разрешение (ширина)
        framebufferInfo.height = resultSwapchain.imageExtent.height;      // Разрешение (высота)
        framebufferInfo.layers = 1;                                       // Один слой

        // В случае успешного создания - добавить в массив
        if (vkCreateFramebuffer(device.logicalDevice, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS) {
            resultSwapchain.framebuffers.push_back(framebuffer);
        }
        else {
            throw std::runtime_error("Vulkan: Error in vkCreateFramebuffer function. Failed to create frame buffers");
        }
    }

    kge::tools::LogMessage("Vulkan: Swap-chain successfully initialized");

    return resultSwapchain;
}

/**
* Деинициализация swap-chain
* @param const kge::vkstructs::Device &device - устройство которое принимало участие в создании swap-chain
* @param kge::vkstructs::Swapchain * swapchain - указатель на объект своп-чейна
*/
void KGEVulkanCore::DeinitSwapchain(const kge::vkstructs::Device &device,
                                    kge::vkstructs::Swapchain *swapchain)
{
    // Очистить фрейм-буферы
    if (!swapchain->framebuffers.empty()) {
        for (VkFramebuffer const &frameBuffer : swapchain->framebuffers) {
            vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
        }
        swapchain->framebuffers.clear();
    }

    // Очистить image-views объекты
    if (!swapchain->imageViews.empty()) {
        for (VkImageView const &imageView : swapchain->imageViews) {
            vkDestroyImageView(device.logicalDevice, imageView, nullptr);
        }
        swapchain->imageViews.clear();
    }

    // Очиска компонентов Z-буфера
    swapchain->depthStencil.Deinit(device.logicalDevice);

    // Очистить swap-chain
    if (swapchain->vkSwapchain != nullptr) {
        vkDestroySwapchainKHR(device.logicalDevice, swapchain->vkSwapchain, nullptr);
        swapchain->vkSwapchain = nullptr;
    }

    // Сбросить расширение и формат
    swapchain->imageExtent = {};
    swapchain->imageFormat = {};

    kge::tools::LogMessage("Vulkan: Swap-chain successfully deinitialized");
}

/**
* Инциализация командного пула
* @param const kge::vkstructs::Device &device - устройство
* @param unsigned int queueFamilyIndex - индекс семейства очередей команды которого будут передаваться в аллоцированных их пуда буферах
* @return VkCommandPool - хендл командного пула
* @note - из командных пулов осуществляется аллокация буферов команд, следует учитывать что для отедльных очередей нужны отдельные пулы
*/
VkCommandPool KGEVulkanCore::InitCommandPool(const kge::vkstructs::Device &device,
                                             unsigned int queueFamilyIndex)
{
    // Результат
    VkCommandPool resultPool = nullptr;

    // Описание пула
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = static_cast<unsigned int>(queueFamilyIndex);
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // Создание пула
    if (vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &resultPool) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateCommandPool function. Failed to create command pool");
    }

    kge::tools::LogMessage("Vulkan: Command pool successfully initialized");

    // Вернуть pool
    return resultPool;
}

/**
* Деинциализация командного пула
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool * commandPool - указатель на хендл командного пула
*/
void KGEVulkanCore::DeinitCommandPool(const kge::vkstructs::Device &device,
                                      VkCommandPool *commandPool)
{
    if (commandPool != nullptr && *commandPool != nullptr) {
        vkDestroyCommandPool(device.logicalDevice, *commandPool, nullptr);
        *commandPool = nullptr;
        kge::tools::LogMessage("Vulkan: Command pool successfully deinitialized");
    }
}

/**
* Аллокация командных буферов
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого будет осуществляться аллокация
* @param unsigned int count - кол-во аллоцируемых буферов
* @return std::vector<VkCommandBuffer> массив хендлов командных буферов
*/
std::vector<VkCommandBuffer> KGEVulkanCore::InitCommandBuffers(const kge::vkstructs::Device &device,
                                                               VkCommandPool commandPool,
                                                               unsigned int count)
{
    // Результат
    std::vector<VkCommandBuffer> resultBuffers(count);

    // Конфигурация аллокации буферов
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;                               // Указание командного пула
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                 // Передается в очередь непосредственно
    allocInfo.commandBufferCount = static_cast<unsigned int>(resultBuffers.size()); // Кол-во командных буферов

    // Аллоцировать буферы команд
    if (vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, resultBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateCommandBuffers function. Failed to allocate command buffers");
    }

    kge::tools::LogMessage("Vulkan: Command buffers successfully allocated");

    // Вернуть массив хендлов
    return resultBuffers;
}

/**
* Деинициализация (очистка) командных буферов
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого были аллоцированы буферы
* @param std::vector<VkCommandBuffer> * buffers - указатель на массив с хендлами буферов (он будет обнулен после очистки)
*/
void KGEVulkanCore::DeinitCommandBuffers(const kge::vkstructs::Device &device,
                                         VkCommandPool commandPool,
                                         std::vector<VkCommandBuffer> *buffers)
{
    // Если массив индентификаторов буферов команд рисования не пуст
    if (device.logicalDevice != nullptr && buffers != nullptr && !buffers->empty()) {
        // Очистисть память
        vkFreeCommandBuffers(device.logicalDevice, commandPool, static_cast<unsigned int>(buffers->size()), buffers->data());
        // Очистить массив
        buffers->clear();

        kge::tools::LogMessage("Vulkan: Command buffers successfully freed");
    }
}

/**
* Создание мирового (глобального) unform-buffer'а
* @param const kge::vkstructs::Device &device - устройство
* @return kge::vkstructs::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - unform-буфер это буфер доступный для шейдера посредством дескриптороа. В нем содержится информация о матрицах используемых
* для преобразования координат вершин сцены. В буфер помещается UBO объект содержащий необходимые матрицы. При каждом обновлении сцены
* можно отправлять объект с новыми данными (например, если сменилось положение камеры, либо угол ее поворота). Таким образом шейдер будет
* использовать для преобразования координат вершины новые данные.
*/
kge::vkstructs::UniformBuffer KGEVulkanCore::InitUnformBufferWorld(const kge::vkstructs::Device &device)
{
    // Результирующий объект
    kge::vkstructs::UniformBuffer resultBuffer = {};

    // Создать буфер, выделить память, привязать память к буферу
    kge::vkstructs::Buffer buffer = kge::vkutility::CreateBuffer(
                device,
                sizeof(kge::vkstructs::UboWorld),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Основная конфиуграция результирущего буфера
    resultBuffer.vkBuffer = buffer.vkBuffer;
    resultBuffer.vkDeviceMemory = buffer.vkDeviceMemory;
    resultBuffer.size = buffer.size;

    // Настройка информации для дескриптора
    resultBuffer.configDescriptorInfo(buffer.size, 0);

    // Разметить буфер (сделать его доступным для копирования информации)
    resultBuffer.map(device.logicalDevice, buffer.size, 0);

    kge::tools::LogMessage("Vulkan: Uniform buffer for world scene successfully allocated");

    return resultBuffer;
}

/**
* Создание буфера для моделей (динамический uniform-bufer)
* @param const kge::vkstructs::Device &device - устройство
* @param unsigned int maxObjects - максимальное кол-во отдельных объектов на сцене
* @return kge::vkstructs::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - в отличии от мирового uniform-буфера, uniform-буфер моделей содержит отдельные матрицы для кадой модели (по сути массив)
* и выделение памяти под передаваемый в такой буфер объект должно использовать выравнивание. У устройства есть определенные лимиты
* на выравнивание памяти, поэтому размер такого буфера вычисляется с учетом допустимого шага выравивания и кол-ва объектов которые
* могут быть на сцене.
*/
kge::vkstructs::UniformBuffer KGEVulkanCore::InitUniformBufferModels(const kge::vkstructs::Device &device,
                                                                     unsigned int maxObjects)
{
    // Результирующий объект
    kge::vkstructs::UniformBuffer resultBuffer = {};

    // Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
    VkDeviceSize bufferSize = device.GetDynamicAlignment<glm::mat4>() * maxObjects;

    kge::vkstructs::Buffer buffer = kge::vkutility::CreateBuffer(
                device,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Настройка результирубщего буфера (uniform-буфер)
    resultBuffer.vkBuffer = buffer.vkBuffer;
    resultBuffer.vkDeviceMemory = buffer.vkDeviceMemory;
    resultBuffer.size = buffer.size;

    // Настройка информации для дескриптора
    resultBuffer.configDescriptorInfo(VK_WHOLE_SIZE);

    // Разметить буфер (сделать его доступным для копирования информации)
    resultBuffer.map(device.logicalDevice, VK_WHOLE_SIZE, 0);

    kge::tools::LogMessage("Vulkan: Uniform buffer for models successfully allocated");

    return resultBuffer;
}

/**
* Деинициализация (очистка) командного буфера
* @param const kge::vkstructs::Device &device - устройство
* @param kge::vkstructs::UniformBuffer * uniformBuffer - указатель на структуру буфера
*/
void KGEVulkanCore::DeinitUniformBuffer(const kge::vkstructs::Device &device,
                                        kge::vkstructs::UniformBuffer *uniformBuffer)
{
    if (uniformBuffer != nullptr) {

        uniformBuffer->unmap(device.logicalDevice);

        if (uniformBuffer->vkBuffer != nullptr) {
            vkDestroyBuffer(device.logicalDevice, uniformBuffer->vkBuffer, nullptr);
            uniformBuffer->vkBuffer = nullptr;
        }

        if (uniformBuffer->vkDeviceMemory != nullptr) {
            vkFreeMemory(device.logicalDevice, uniformBuffer->vkDeviceMemory, nullptr);
            uniformBuffer->vkDeviceMemory = nullptr;
        }

        uniformBuffer->descriptorBufferInfo = {};
        *uniformBuffer = {};

        kge::tools::LogMessage("Vulkan: Uniform buffer successfully deinitialized");
    }
}

/**
* Инициализация основного декскрипторного пула
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
VkDescriptorPool KGEVulkanCore::InitDescriptorPoolMain(const kge::vkstructs::Device &device)
{
    // Хендл нового дескрипторого пула
    VkDescriptorPool descriptorPoolResult = nullptr;

    // Парамтеры размеров пула
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        // Один дескриптор для глобального uniform-буфера
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
        // Один дескриптор для unform-буферов отдельных объектов (динамический)
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
    };


    // Конфигурация пула
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = 1;

    // Создание дескрипторного пула
    if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
    }

    kge::tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");

    // Отдать результат
    return descriptorPoolResult;
}

/**
* Инициализация декскрипторного пула под текстурные наборы дескрипторов
* @param const kge::vkstructs::Device &device - устройство
* @param uint32_t maxDescriptorSets - максимальное кол-во наборов
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
VkDescriptorPool KGEVulkanCore::InitDescriptorPoolTextures(const kge::vkstructs::Device &device,
                                                           uint32_t maxDescriptorSets)
{
    // Хендл нового дескрипторого пула
    VkDescriptorPool descriptorPoolResult = nullptr;

    // Парамтеры размеров пула
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        // Один дескриптор для текстурного семплера
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 },
    };

    // Конфигурация пула
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxDescriptorSets;

    // Создание дескрипторного пула
    if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
    }

    kge::tools::LogMessage("Vulkan: Texture descriptor pool successfully initialized");

    // Отдать результат
    return descriptorPoolResult;
}

/**
* Деинициализация дескрипторного пула
* @param const kge::vkstructs::Device &device - устройство
* @VkDescriptorPool * descriptorPool - указатель на хендл дескрипторного пула
*/
void KGEVulkanCore::DeinitDescriptorPool(const kge::vkstructs::Device &device,
                                         VkDescriptorPool *descriptorPool)
{
    if (descriptorPool != nullptr && *descriptorPool != nullptr) {
        vkDestroyDescriptorPool(device.logicalDevice, *descriptorPool, nullptr);
        *descriptorPool = nullptr;
        kge::tools::LogMessage("Vulkan: Descriptor pool successfully deinitialized");
    }
}

/**
* Инициализация описания размещения дескрипторного пула (под основной дескрипторный набор)
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
VkDescriptorSetLayout KGEVulkanCore::InitDescriptorSetLayoutMain(const kge::vkstructs::Device &device)
{
    // Результирующий хендл
    VkDescriptorSetLayout layoutResult = nullptr;

    // Необходимо описать привязки дескрипторов к этапам конвейера
    // Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
    std::vector<VkDescriptorSetLayoutBinding> bindings =
    {
        {
            0,                                            // Индекс привязки
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // Тип дескриптора (буфер формы, обычный)
            1,                                            // Кол-во дескрипторов
            VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
            nullptr
        },
        {
            1,                                            // Индекс привязки
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // Тип дескриптора (буфер формы, динамический)
            1,                                            // Кол-во дескрипторов
            VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
            nullptr
        },
    };

    // Инициализировать размещение дескрипторного набора
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.pNext = nullptr;
    descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorLayoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
    }

    kge::tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

    return layoutResult;
}

/**
* Инициализация описания размещения дескрипторного пула (под текстурные наборы дескрипторов)
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
VkDescriptorSetLayout KGEVulkanCore::InitDescriptorSetLayoutTextures(const kge::vkstructs::Device &device)
{
    // Результирующий хендл
    VkDescriptorSetLayout layoutResult = nullptr;

    // Необходимо описать привязки дескрипторов к этапам конвейера
    // Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
    std::vector<VkDescriptorSetLayoutBinding> bindings =
    {
        {
            0,                                            // Индекс привязки
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // Тип дескриптора (семплер изображения)
            1,                                            // Кол-во дескрипторов
            VK_SHADER_STAGE_FRAGMENT_BIT,                 // Этап конвейера (вершинный шейдер)
            nullptr
        },
    };

    // Инициализировать размещение дескрипторного набора
    VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
    descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayoutInfo.pNext = nullptr;
    descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorLayoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
    }

    kge::tools::LogMessage("Vulkan: Texture descriptor set layout successfully initialized");

    return layoutResult;
}

/**
* Деинициализация размещения
* @param const kge::vkstructs::Device &device - устройство
* @VkDescriptorSetLayout * descriptorSetLayout - указатель на хендл размещения
*/
void KGEVulkanCore::DeinitDescriporSetLayout(const kge::vkstructs::Device &device,
                                             VkDescriptorSetLayout *descriptorSetLayout)
{
    if (device.logicalDevice != nullptr && descriptorSetLayout != nullptr && *descriptorSetLayout != nullptr) {
        vkDestroyDescriptorSetLayout(device.logicalDevice, *descriptorSetLayout, nullptr);
        *descriptorSetLayout = nullptr;

        kge::tools::LogMessage("Vulkan: Descriptor set layout successfully deinitialized");
    }
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
* Инициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param std::vector<VkDescriptorSetLayout> descriptorSetLayouts - хендлы размещениий дискрипторного набора (дает конвейеру инфу о дескрипторах)
* @return VkPipelineLayout - хендл размещения конвейера
*/
VkPipelineLayout KGEVulkanCore::InitPipelineLayout(const kge::vkstructs::Device &device,
                                                   std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
    VkPipelineLayout resultLayout;

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(device.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &resultLayout) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating pipeline layout");
    }

    kge::tools::LogMessage("Vulkan: Pipeline layout successfully initialized");

    return resultLayout;
}

/**
* Деинициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipelineLayout * pipelineLayout - указатель на хендл размещения
*/
void KGEVulkanCore::DeinitPipelineLayout(const kge::vkstructs::Device &device,
                                         VkPipelineLayout *pipelineLayout)
{
    if (device.logicalDevice != nullptr && pipelineLayout != nullptr && *pipelineLayout != nullptr) {
        vkDestroyPipelineLayout(device.logicalDevice, *pipelineLayout, nullptr);
        *pipelineLayout = nullptr;

        kge::tools::LogMessage("Vulkan: Pipeline layout successfully deinitialized");
    }
}

/**
* Инициализация графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipelineLayout pipelineLayout - хендл размещения конвейера
* @param vktoolkit::Swapchain &swapchain - swap-chain, для получения информации о разрешении
* @param VkRenderPass renderPass - хендл прохода рендеринга (на него ссылается конвейер)
*
* @note - графический конвейер производит рендериннг принимая вершинные данные на вход и выводя пиксели в
* буферы кадров. Конвейер состоит из множества стадий, некоторые из них программируемые (шейдерные). Конвейер
* относится к конкретному проходу рендеринга (и подпроходу). В методе происходит конфигурация всех стадий,
* загрузка шейдеров, настройка конвейера.
*/
VkPipeline KGEVulkanCore::InitGraphicsPipeline(const kge::vkstructs::Device &device,
                                               VkPipelineLayout pipelineLayout,
                                               const kge::vkstructs::Swapchain &swapchain,
                                               VkRenderPass renderPass)
{
    // Результирующий хендл конвейера
    VkPipeline resultPipelineHandle = nullptr;

    // Конфигурация привязок и аттрибутов входных данных (вершинных)
    std::vector<VkVertexInputBindingDescription> bindingDescription = kge::vkutility::GetVertexInputBindingDescriptions(0);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = kge::vkutility::GetVertexInputAttributeDescriptions(0);

    // Конфигурация стадии ввода вершинных данных
    VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
    vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStage.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
    vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
    vertexInputStage.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Описание этапа "сборки" входных данных
    // Конвейер будет "собирать" вершинны в набор треугольников
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
    inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
    inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// Перезагрузка примитивов не используется

    // Прогамируемые (шейдерные) этапы конвейера
    // Используем 2 шейдера - вершинный (для каждой вершины) и фрагментный (пиксельный, для каждого пикселя)
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            kge::vkutility::LoadSPIRVShader("base.vert.spv", device.logicalDevice),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            kge::vkutility::LoadSPIRVShader("base.frag.spv", device.logicalDevice),
            "main",
            nullptr
        }
    };

    // Описываем область просмотра
    // Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.imageExtent.width);
    viewport.height = static_cast<float>(swapchain.imageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.imageExtent;

    // Описываем этап вывода в область просмотра
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Описываем этап растеризации
    VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
    rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
    rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
    rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны
    rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
    rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // Отсечение граней (отсекаются те, что считаются задними)
    rasterizationStage.frontFace = VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке
    rasterizationStage.depthBiasEnable = VK_FALSE;                      // Контроль значений глубины
    rasterizationStage.depthBiasConstantFactor = 0.0f;
    rasterizationStage.depthBiasClamp = 0.0f;
    rasterizationStage.depthBiasSlopeFactor = 0.0f;

    // Описываем этап z-теста (теста глубины)
    // Активировать тест глубины, использовать сравнение "меньше или равно"
    VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
    depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStage.depthTestEnable = VK_TRUE;
    depthStencilStage.depthWriteEnable = VK_TRUE;
    depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStage.depthBoundsTestEnable = VK_FALSE;
    depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStage.stencilTestEnable = VK_FALSE;
    depthStencilStage.front = depthStencilStage.back;

    // Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
    // Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
    // за счет чего возникает эффект более сглаженных линий (в данном примере не используется мульти-семплинг)
    VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
    multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStage.sampleShadingEnable = VK_FALSE;
    multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStage.minSampleShading = 1.0f;
    multisamplingStage.pSampleMask = nullptr;
    multisamplingStage.alphaToCoverageEnable = VK_FALSE;
    multisamplingStage.alphaToOneEnable = VK_FALSE;

    // Этап смешивания цвета (для каждого фрейм-буфера)
    // На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
    // В начлае нужно описать состояния смешивания для цветовых вложений (используем одно состояние на вложение) (???)
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Глобальные настройки смешивания
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;

    // Информация инициализации графического конвейера
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());    // Кол-во шейдерных этапов
    pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
    pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
    pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
    pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
    pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
    pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
    pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
    pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
    pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
    pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
    pipelineInfo.subpass = 0;                                   // Связываем с под-проходом (первый под-проход)

    // Создание графического конвейера
    if (vkCreateGraphicsPipelines(device.logicalDevice, nullptr, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating pipeline");
    }

    kge::tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

    // Шейдерные модули больше не нужны после создания конвейера
    for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
        vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
    }

    return resultPipelineHandle;
}

/**
* Деинициализация графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipeline * pipeline - указатель на хендл конвейера
*/
void KGEVulkanCore::DeinitGraphicsPipeline(const kge::vkstructs::Device &device,
                                           VkPipeline *pipeline)
{
    if (device.logicalDevice != nullptr && pipeline != nullptr && *pipeline != nullptr) {
        vkDestroyPipeline(device.logicalDevice, *pipeline, nullptr);
        *pipeline = nullptr;
        kge::tools::LogMessage("Vulkan: Pipeline sucessfully deinitialized");
    }
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
