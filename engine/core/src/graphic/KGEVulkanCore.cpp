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
    ////m_vkInstance{},
    m_kgeVkInstance{applicationName, "KGEngine", instanceExtensionsRequired, validationLayersRequired},
    // Report callBack
    //m_kgeVkReportCallBack{m_kgeVkInstance.instance()},
    // Инициализация поверхности отображения
    ////m_vkSurface{},
    m_kgeVkSurface{windowControl, m_kgeVkInstance.instance()},
    // Инициализация устройства
    ////m_device{},
    m_kgeVkDevice{m_kgeVkInstance.instance(), m_kgeVkSurface.surface(), deviceExtensionsRequired, validationLayersRequired, false},
    // Инициализация прохода рендеринга
    ////m_renderPass{},
    m_kgeRenderPass{m_kgeVkDevice.device(), m_kgeVkSurface.surface(), VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT},
    // Инициализация swap-chain
    ////m_swapchain{},
    m_kgeSwapChain{m_kgeVkDevice.device(), m_kgeVkSurface.surface(), { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, m_kgeRenderPass.renderPass(), 3},
    // Инциализация командного пула
    ////m_commandPoolDraw{},
    m_kgeVkCommandPool{m_kgeVkDevice.device(), static_cast<unsigned int>(m_kgeVkDevice.device()->queueFamilies.graphics)},
    // Аллокация командных буферов (получение хендлов)
    ////m_commandBuffersDraw{},
    m_kgeVkCommandBuffer{m_kgeVkDevice.device(), &m_kgeVkCommandPool.commandPool(), static_cast<unsigned int>(m_kgeSwapChain.swapchain().framebuffers.size())},
    //Аллокация глобального uniform-буфера
    ////m_uniformBufferWorld{},
    m_kgeVkUniformBufferWorld{m_kgeVkDevice.device()},
    // Аллокация uniform-буфера отдельных объектов (динамический буфер)
    ////m_uniformBufferModels{},
    m_kgeVkUniformBufferModels{m_kgeVkDevice.device(), m_primitivesMaxCount},
    // Создание дескрипторного пула для выделения основного набора (для unform-буфера)
    ////m_descriptorPoolMain{},
    m_kgeVkDescriptorPoolMain{m_kgeVkDevice.device()},
    // Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
    //m_descriptorPoolTextures{},
    m_kgeVkDescriptorPoolTextures{m_kgeVkDevice.device(), 5},
    // Инициализация размещения основного дескрипторного набора
    //m_descriptorSetLayoutMain{},
    m_kgeVkDescriptorSetLayoutMain{m_kgeVkDevice.device(), SetLayoutMain},
    // Инициализация размещения теккстурного набора
    //m_descriptorSetLayoutTextures{},
    m_kgeVkDescriptorSetLayoutTextures{m_kgeVkDevice.device(), SetLayoutTextures},
    // Инициализация текстурного семплера
    //m_textureSampler{},
    m_kgeVkSampler{m_kgeVkDevice.device()},
    // Инициализация дескрипторного набора
    //m_descriptorSetMain{},
    m_kgeVkDescriptorSet{m_kgeVkDevice.device(), &m_kgeVkDescriptorPoolMain.descriptorPool(), &m_kgeVkDescriptorSetLayoutMain.descriptorSetLayout(), m_kgeVkUniformBufferWorld.uniformBufferWorld(), &m_kgeVkUniformBufferModels.m_uniformBufferModels},
    // Инициализация размещения графического конвейера
    //m_pipelineLayout{},
    m_kgeVkPipelineLayout{m_kgeVkDevice.device(), { m_kgeVkDescriptorSetLayoutMain.descriptorSetLayout(), m_kgeVkDescriptorSetLayoutTextures.descriptorSetLayout()}},
    // Инициализация графического конвейера
    //m_pipeline{},
    m_kgeVkGraphicsPipeline{m_kgeVkDevice.device(), m_kgeVkPipelineLayout.pipelineLayout(), m_kgeSwapChain.swapchain(), m_kgeRenderPass.renderPass()},
    // Аллокация памяти массива ubo-объектов отдельных примитивов
    //m_uboModels{},
    m_kgeUboModels{&m_uboModels, m_kgeVkDevice.device(), m_primitivesMaxCount},
    // Примитивы синхронизации
    //m_sync{},
    m_kgeVkSynchronization{&m_sync, m_kgeVkDevice.device()}
{
    // Присвоить параметры камеры по умолчанию
    m_camera.fFar  = DEFAULT_FOV;
    m_camera.fFar  = DEFAULT_FAR;
    m_camera.fNear = DEFAULT_NEAR;

    PrepareDrawCommands(
                m_kgeVkCommandBuffer.commandBuffersDraw(),
                m_kgeRenderPass.renderPass(),
                m_kgeVkPipelineLayout.pipelineLayout(),
                m_kgeVkDescriptorSet.descriptorSet(),
                m_kgeVkGraphicsPipeline.pipeline(),
                m_kgeSwapChain.swapchain(),
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
    if (m_kgeVkDevice.device()->logicalDevice != nullptr) {
        vkDeviceWaitIdle(m_kgeVkDevice.device()->logicalDevice);
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
    m_kgeVkCommandBuffer.~KGEVkCommandBuffer();
    m_kgeVkGraphicsPipeline.~KGEVkGraphicsPipeline();// DeinitGraphicsPipeline(m_device, &m_pipeline);
    m_kgeRenderPass.~KGEVkRenderPass();
    m_kgeRenderPass = { m_kgeVkDevice.device(), m_kgeVkSurface.surface(), VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT};

    // Ре-инициализация swap-cahin.
    // В начале получаем старый swap-chain
    kge::vkstructs::Swapchain oldSwapChain = m_kgeSwapChain.swapchain();
    m_kgeSwapChain.~KGEVkSwapChain();

    // Инициализируем обновленный
    m_kgeSwapChain = { m_kgeVkDevice.device(), m_kgeVkSurface.surface(), {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, m_kgeRenderPass.renderPass(), 3, &oldSwapChain};

    // Инициализация графического конвейера
    m_kgeVkGraphicsPipeline = {m_kgeVkDevice.device(), m_kgeVkPipelineLayout.pipelineLayout(), m_kgeSwapChain.swapchain(), m_kgeRenderPass.renderPass()};
    // Аллокация командных буферов (получение хендлов)
    m_kgeVkCommandBuffer = {m_kgeVkDevice.device(), &m_kgeVkCommandPool.commandPool(), static_cast<unsigned int>(m_kgeSwapChain.swapchain().framebuffers.size())};

    // Подготовка базовых комманд
    PrepareDrawCommands(
                m_kgeVkCommandBuffer.commandBuffersDraw(),
                m_kgeRenderPass.renderPass(),
                m_kgeVkPipelineLayout.pipelineLayout(),
                m_kgeVkDescriptorSet.descriptorSet(),
                m_kgeVkGraphicsPipeline.pipeline(),
                m_kgeSwapChain.swapchain(),
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
    std::cout << "--DRAW--" << std::endl;
    // Ничего не делать если не готово или приостановлено
    if (!m_isReady || !m_isRendering) {
        return;
    }

    // Индекс доступного изображения
    unsigned int imageIndex;

    // Получить индекс доступного изображения из swap-chain и "включить" семафор сигнализирующий о доступности изображения для рендеринга
    VkResult acquireStatus = vkAcquireNextImageKHR(
                m_kgeVkDevice.device()->logicalDevice,
                m_kgeSwapChain.swapchain().vkSwapchain,
                1000,
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

    VkFenceCreateInfo fenceInfo;
    VkFence drawFence;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = nullptr;
    fenceInfo.flags = 0;
    vkCreateFence(m_kgeVkDevice.device()->logicalDevice, &fenceInfo, nullptr, &drawFence);

    // Информация об отправке команд в буфер
    VkSubmitInfo submitInfo[1] = {};
    submitInfo[0].pNext = nullptr;
    submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo[0].waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());       // Кол-во семафоров ожидания
    submitInfo[0].pWaitSemaphores = waitSemaphores.data();                                 // Семафоры велючение которых будет ожидаться
    submitInfo[0].pWaitDstStageMask = waitStages;                                          // Стадии на которых конвейер "приостановиться" до включения семафоров
    submitInfo[0].commandBufferCount = 1;                                                  // Число командных буферов за одну отправку
    submitInfo[0].pCommandBuffers = &m_kgeVkCommandBuffer.commandBuffersDraw()[imageIndex];// Командный буфер (для текущего изображения в swap-chain)
    submitInfo[0].signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());   // Кол-во семафоров сигнала (завершения стадии)
    submitInfo[0].pSignalSemaphores = signalSemaphores.data();                             // Семафоры которые включатся при завершении


    // Инициировать отправку команд в очередь (на рендеринг)
    VkResult result = vkQueueSubmit(m_kgeVkDevice.device()->queues.graphics, 1, submitInfo, drawFence);
    std::cout << "----" << std::endl;
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error. Can't submit commands");
    }

    // Настройка представления (отображение того что отдал конвейер)
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());    // Кол-во ожидаемых семафоров
    presentInfo.pWaitSemaphores = signalSemaphores.data();                              // Cемафоры "включение" которых ожидается перед показом
    presentInfo.swapchainCount = 1;                                                     // Кол-во swap-chain'ов
    presentInfo.pSwapchains = &m_kgeSwapChain.swapchain().vkSwapchain;                  // Указание текущего swap-chain
    presentInfo.pImageIndices = &imageIndex;                                            // Индекс текущего изображения, куда осуществляется показ
    presentInfo.pResults = nullptr;

    // Инициировать представление
    VkResult presentStatus = vkQueuePresentKHR(m_kgeVkDevice.device()->queues.present, &presentInfo);

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
    std::cout  << "--UPDATE--" << std::endl;
    // Соотношение сторон (используем размеры поверхности определенные при создании swap-chain)
    m_camera.aspectRatio = static_cast<float>(m_kgeSwapChain.swapchain().imageExtent.width) /m_kgeSwapChain.swapchain().imageExtent.height;

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
    memcpy(m_kgeVkUniformBufferWorld.uniformBufferWorld()->pMapped,
           &m_uboWorld,
           static_cast<size_t>(m_kgeVkUniformBufferWorld.uniformBufferWorld()->size));

    // Теперь необходимо обновить динамический буфер формы объектов (если они есть)
    if (!m_primitives.empty()) {

        // Динамическое выравнивание для одного элемента массива
        VkDeviceSize dynamicAlignment = m_kgeVkDevice.device()->GetDynamicAlignment<glm::mat4>();

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
        memcpy(m_kgeVkUniformBufferModels.m_uniformBufferModels.pMapped, m_uboModels, m_kgeVkUniformBufferModels.m_uniformBufferModels.size);

        // Гарантировать видимость обновленной памяти устройством
        VkMappedMemoryRange memoryRange = {};
        memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        memoryRange.memory = m_kgeVkUniformBufferModels.m_uniformBufferModels.vkDeviceMemory;
        memoryRange.size = m_kgeVkUniformBufferModels.m_uniformBufferModels.size;
        vkFlushMappedMemoryRanges(m_kgeVkDevice.device()->logicalDevice, 1, &memoryRange);
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
    kge::vkstructs::Buffer tmp = kge::vkutility::CreateBuffer(*m_kgeVkDevice.device(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    primitive.vertexBuffer.vkBuffer = tmp.vkBuffer;
    primitive.vertexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
    primitive.vertexBuffer.size = tmp.size;
    primitive.vertexBuffer.count = vertexCount;

    // Разметить память буфера вершин и скопировать в него данные, после чего убрать разметку
    void * verticesMemPtr;
    vkMapMemory(m_kgeVkDevice.device()->logicalDevice, primitive.vertexBuffer.vkDeviceMemory, 0, vertexBufferSize, 0, &verticesMemPtr);
    memcpy(verticesMemPtr, vertexBuffer.data(), static_cast<size_t>(vertexBufferSize));
    vkUnmapMemory(m_kgeVkDevice.device()->logicalDevice, primitive.vertexBuffer.vkDeviceMemory);

    // Если необходимо рисовать индексированную геометрию
    if (primitive.drawIndexed) {

        // Буфер индексов (временный)
        std::vector<unsigned int> indexBuffer = indices;
        VkDeviceSize indexBufferSize = (static_cast<unsigned int>(indexBuffer.size())) * sizeof(unsigned int);
        unsigned int indexCount = static_cast<unsigned int>(indexBuffer.size());

        // Cоздать буфер индексов в памяти хоста
        tmp = kge::vkutility::CreateBuffer(*m_kgeVkDevice.device(),
                                           indexBufferSize,
                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        primitive.indexBuffer.vkBuffer = tmp.vkBuffer;
        primitive.indexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
        primitive.indexBuffer.size = tmp.size;
        primitive.indexBuffer.count = indexCount;

        // Разметить память буфера индексов и скопировать в него данные, после чего убрать разметку
        void * indicesMemPtr;
        vkMapMemory(m_kgeVkDevice.device()->logicalDevice, primitive.indexBuffer.vkDeviceMemory, 0, indexBufferSize, 0, &indicesMemPtr);
        memcpy(indicesMemPtr, indexBuffer.data(), static_cast<size_t>(indexBufferSize));
        vkUnmapMemory(m_kgeVkDevice.device()->logicalDevice, primitive.indexBuffer.vkDeviceMemory);
    }


    // Впихнуть новый примитив в массив
    m_primitives.push_back(primitive);

    // Обновить командный буфер
    ResetCommandBuffers(*m_kgeVkDevice.device(),
                        m_kgeVkCommandBuffer.commandBuffersDraw());

    PrepareDrawCommands(m_kgeVkCommandBuffer.commandBuffersDraw(),
                        m_kgeRenderPass.renderPass(),
                        m_kgeVkPipelineLayout.pipelineLayout(),
                        m_kgeVkDescriptorSet.descriptorSet(),
                        m_kgeVkGraphicsPipeline.pipeline(),
                        m_kgeSwapChain.swapchain(),
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
    kge::vkstructs::Image stagingImage = kge::vkutility::CreateImageSingle(*m_kgeVkDevice.device(),
                                                                           VK_IMAGE_TYPE_2D,
                                                                           VK_FORMAT_R8G8B8A8_UNORM,
                                                                           { width,height,1 },
                                                                           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
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
    vkGetImageSubresourceLayout(m_kgeVkDevice.device()->logicalDevice, stagingImage.vkImage, &subresource, &stagingImageSubresourceLayout);

    // Разметить память под изображение
    void* data;
    vkMapMemory(m_kgeVkDevice.device()->logicalDevice, stagingImage.vkDeviceMemory, 0, size, 0, &data);

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
    vkUnmapMemory(m_kgeVkDevice.device()->logicalDevice, stagingImage.vkDeviceMemory);

    // Создать финальное изображение (в памяти устройства)
    resultTexture.image = kge::vkutility::CreateImageSingle(
                *m_kgeVkDevice.device(),
                VK_IMAGE_TYPE_2D,
                VK_FORMAT_R8G8B8A8_UNORM,
    { width,height, 1 },
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_PREINITIALIZED,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_TILING_OPTIMAL);


    // Создать командный буфер для команд перевода размещения изображений
    VkCommandBuffer transitionCmdBuffer = kge::vkutility::CreateSingleTimeCommandBuffer(*m_kgeVkDevice.device(), m_kgeVkCommandPool.commandPool());

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
    kge::vkutility::FlushSingleTimeCommandBuffer(*m_kgeVkDevice.device(), m_kgeVkCommandPool.commandPool(), transitionCmdBuffer, m_kgeVkDevice.device()->queues.graphics);

    // Создать командный буфер для копирования изображения
    VkCommandBuffer copyCmdBuffer = kge::vkutility::CreateSingleTimeCommandBuffer(*m_kgeVkDevice.device(), m_kgeVkCommandPool.commandPool());

    // Копирование из промежуточной картинки в основную
    kge::vkutility::CmdImageCopy(copyCmdBuffer, stagingImage.vkImage, resultTexture.image.vkImage, width, height);

    // Выполнить команды копирования
    kge::vkutility::FlushSingleTimeCommandBuffer(*m_kgeVkDevice.device(), m_kgeVkCommandPool.commandPool(), copyCmdBuffer, m_kgeVkDevice.device()->queues.graphics);

    // Очистить промежуточное изображение
    stagingImage.Deinit(m_kgeVkDevice.device()->logicalDevice);


    // Получить новый набор дескрипторов из дескриптороного пула
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_kgeVkDescriptorPoolTextures.descriptorPool();
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &(m_kgeVkDescriptorSetLayoutTextures.descriptorSetLayout());

    if (vkAllocateDescriptorSets(m_kgeVkDevice.device()->logicalDevice, &descriptorSetAllocInfo, &(resultTexture.descriptorSet)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set for texture");
    }

    // Информация о передаваемом изображении
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = resultTexture.image.vkImageView;
    imageInfo.sampler = m_kgeVkSampler.sampler();

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
    vkUpdateDescriptorSets(m_kgeVkDevice.device()->logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

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
    ResetCommandBuffers(*m_kgeVkDevice.device(), m_kgeVkCommandBuffer.commandBuffersDraw());
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
void KGEVulkanCore::PrepareDrawCommands(std::vector<VkCommandBuffer> commandBuffers,
                                        VkRenderPass renderPass,
                                        VkPipelineLayout pipelineLayout,
                                        VkDescriptorSet descriptorSetMain,
                                        VkPipeline pipeline,
                                        const kge::vkstructs::Swapchain &swapchain,
                                        const std::vector<kge::vkstructs::Primitive> &primitives)
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
                    primitiveIndex * static_cast<uint32_t>(m_kgeVkDevice.device()->GetDynamicAlignment<glm::mat4>())
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
