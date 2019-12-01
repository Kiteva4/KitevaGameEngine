#pragma once

#define ENABLE_DEBUG_LAYERS

#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>

#include <string>
#include <fstream>
#include <ctime>
#include <time.h>
#include <chrono>

// Необходимо для функций получения путей к файлу
//#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

// Наименования лог-файла
#ifndef LOG_FILENAME
#define LOG_FILENAME "log.txt"
#endif

namespace kge
{
    namespace vkstructs
    {
        /**
        * Структура описывающая ресурс изображения состоит из 3-ех составляющих:
        * - Хендл изображения (объект самого ресурса изображения)
        * - Хендл памяти (для использования изображения, как и в случае с буфером, нужно аллоцировать и привязывать память)
        * - Хендл view объекта (используется для доступа к самом ресурсу либо подресурсам)
        */
        struct Image
        {
            VkImage vkImage = nullptr;
            VkDeviceMemory vkDeviceMemory = nullptr;
            VkImageView vkImageView = nullptr;
            VkFormat format = {};
            VkExtent3D extent = {};

            // Деинициализация (очистка памяти)
            void Deinit(VkDevice logicalDevice) {

                this->format = {};
                this->extent = {};

                if (this->vkImageView != nullptr) {
                    vkDestroyImageView(logicalDevice, this->vkImageView, nullptr);
                    this->vkImageView = nullptr;
                }

                if (this->vkImage != nullptr) {
                    vkDestroyImage(logicalDevice, this->vkImage, nullptr);
                    this->vkImage = nullptr;
                }

                if (this->vkDeviceMemory != nullptr) {
                    vkFreeMemory(logicalDevice, this->vkDeviceMemory, nullptr);
                    this->vkDeviceMemory = nullptr;
                }
            }
        };

        /**
        * Стурктура описывающая объект с информацией о семействах очередей (их индексы, проверка - доступен ли рендеринг)
        *
        * Команды, которые программа будет отдавать устройству, разделены на семейства, например:
        * - Семейство команд рисования (графические команды)
        * - Семейство команд прдеставления (показ того что нарисовано)
        * - Семейство команд вычисления (подсчеты) и прочие
        *
        * Семейство может поддерживать команды сразу нескольких типов, потому индекс семейства, например, для граифческих
        * команд и команд представления, может совпадать. Объект данной структуры можно будет получить при помощи функции GetQueueFamilyInfo
        * которая вернет информацию о семействах поддерживаемых конкретным устройством для конкретной поверхности
        */
        struct QueueFamilyInfo {
            int graphics    = -1; /*! графические моманды */
            int present     = -1; /*! представления */
            int compute     = -1; /*! вычисления */
            int transfer    = -1; /*! перемещения */

            //Метод проверки поддерживает ли данный набор рендеринг
            bool IsRenderingCompatible() const {
                return graphics >=0 && present >=0;
            }
        };

        /**
        * Структура описывает объект устройства. Данный объект содержит хендлы физического и логического устройства,
        * информацию о доступных семействах очередей, хендлы самих очередей, позволяет получить необходимую
        * информацию об устройстве
        */
        struct Device{
            VkPhysicalDevice physicalDevice = nullptr;
            VkDevice logicalDevice = nullptr;

            kge::vkstructs::QueueFamilyInfo queueFamilies = {};

            struct {
                VkQueue graphics = nullptr;
                VkQueue present = nullptr;
            } queues;

            VkPhysicalDeviceProperties GetProperties() const {

                VkPhysicalDeviceProperties properties = {};
                if(physicalDevice != nullptr){
                    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
                }

                return properties;
            }

            bool IsReady() const {
                return  physicalDevice  != nullptr &&
                        logicalDevice   != nullptr &&
                        queues.graphics != nullptr &&
                        queues.present  != nullptr &&
                        queueFamilies.IsRenderingCompatible();
            }

            void Deinit(){
                if(logicalDevice != nullptr){
                    vkDestroyDevice(logicalDevice, nullptr);
                    logicalDevice = nullptr;
                }

                physicalDevice  = nullptr;
                queues.graphics = nullptr;
                queues.present  = nullptr;
                queueFamilies   = {};
            }

            // Получить выравнивание памяти для конкретного типа даныз учитывая аппаратные лимиты физического устройства
            // Иногда необходимо аллоцировать именно выравненную память, этот метод возвращает размер одного фрагмента
            // выравненной памяти с учетом размера типа и лимитов устройства
            template <typename T>
            VkDeviceSize GetDynamicAlignment() const {
                VkDeviceSize minUboAlignment = this->GetProperties().limits.minUniformBufferOffsetAlignment;
                VkDeviceSize dynamicAlignment = static_cast<VkDeviceSize>(sizeof(T));

                if (minUboAlignment > 0) {
                    dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
                }

                return dynamicAlignment;
            }

            // Проверить поддерживает ли устройство вложения глубины-трафарета для конкретного формата
            bool IsDepthFormatSupported(VkFormat format) const{

                VkFormatProperties formatProps;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

                // Формат должен поддерживать вложения глубины-трафарета
                if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT){
                    return true;
                }

                return false;
            }

        };

        /**
        * Структура объекта с информацией о возможностях поверхности, о ее форматах и доступных режимах отображения
        * Пояснение: То, где будет показываться отрендереная картинка, называется поверхностью. Например,
        * поверхность окна Windows или какая либо иная поверхность (если, например, показ происходит вне окна, сразу на экран).
        * Каждая поверхность обладает своими ограничениями и свойствами (цвета, частота обновления, и прочее).
        */
        struct SurfaceInfo {
            VkSurfaceCapabilitiesKHR capabilities = {}; /*! Структура с набором возможностей  */
            std::vector<VkSurfaceFormatKHR> formats;    /*! Массив форматов  */
            std::vector<VkPresentModeKHR> presentModes; /*! Масисв режимов представлений  */

            /*! Метод для проверки поддержки формата 'VkFormat' у структуры 'VkSurfaceFormatKHR'.*/
            bool IsFormatSupported(VkFormat format) const {
                if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
                    return true;
                }
                else if(formats.size() > 1){
                    for(const auto& formatEntry : formats) {
                        if(formatEntry.format == format){
                            return true;
                        }
                    }
                }
                return false;
            }

            /*! Проверка цветового пространства */
            bool IsColorSpaceSupported(VkColorSpaceKHR colorSpace) const {
                if(formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED){
                    return true;
                }
                else if(formats.size() > 1){
                    for(const auto& formatEntry : formats) {
                        if(formatEntry.colorSpace == colorSpace){
                            return true;
                        }
                    }
                }
                return false;
            }

            //TODO
            /*! Проверка поддержки формата поверхности (с учетом всех составляющих формата поверхности)*/
            bool IsSurfaceFormatSupported(VkSurfaceFormatKHR surfaceFormat) const {
                return  IsFormatSupported(surfaceFormat.format) &&
                        IsColorSpaceSupported(surfaceFormat.colorSpace);
            }
        };

        /**
        * Структура описывающая своп-чейн (список показа). Список показа представляет из себя набор сменяемых изображений.
        * В одно изображение может производится запись (рендеринг), в то время как другое будет показываться (презентация).
        * Как правило изображений в списке от 1 до 3. Если изображение одно - рендеринг будет происходить в показанное изображение
        * Структура содержит :
        * - Хендл своп-чейна (списка показа)
        * - Массив изоражений своп-чейна (хендлы изображений в которые будет производится показ)
        * - Массив видов изображений (своеобразные ссылки на изображения предостовляющие необходимый доступ к ресурсу)
        * - Массив буферов кадров
        */
        struct Swapchain {
            // Хендл swap-chain
            VkSwapchainKHR vkSwapchain = nullptr;

            // Хендлы изображений и видов для цветовых вложений фрейм-буферов
            // а так же формат и расширение
            std::vector<VkImage> images;
            std::vector<VkImageView> imageViews;
            VkFormat imageFormat = {};
            VkExtent2D imageExtent = {};

            // Изображение буфера глубины-трафорета (Z-буфер)
            // К данным изображениям следует создавать так же и память, в отличии
            // от изображений swap-chain, которые аллоцируются. Структура vktoolkit::Image
            // содержит все необходимое
            kge::vkstructs::Image depthStencil = {};

            // Хендлы фреймбуферов
            std::vector<VkFramebuffer> framebuffers;
        };

        /**
        * Структура описывающая простейший буфер vulkan
        * Содержит хендл буфера и хендл памяти выделенной под него
        */
        struct Buffer {
            VkBuffer vkBuffer = nullptr;
            VkDeviceMemory vkDeviceMemory = nullptr;
            VkDeviceSize size = 0;
        };

        /**
        * Буфер вершин - (отличается наличием кол-ва вершин)
        */
        struct VertexBuffer : Buffer
        {
            uint32_t count = 0;
        };

        /**
        * Буфер индексов (отличается наличием кол-ва индексов)
        */
        struct IndexBuffer : Buffer
        {
            uint32_t count = 0;
        };

        /**
        * Буфер-uniform. Содержит информацию о матрицах преобразования координат вершин сцены
        * Может быть как статическим (при создании учитывается изветный рзамер объекта UBO, что будет передаваться)
        * либо динамическим, для множенства UBO объектов (учитывается выравнивание аллоцируемой памяти и кол-во элементов массива)
        */
        struct UniformBuffer : Buffer
        {
            // Информация для дескрипторного набора (используется при инициализации набора)
            VkDescriptorBufferInfo descriptorBufferInfo = {};

            // Указатель на размеченную память буфера
            void * pMapped = nullptr;

            // Разметить память (после этого указатель pMapped будет указывать на нее)
            VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0){
                return vkMapMemory(device, this->vkDeviceMemory, offset, size, 0, &(this->pMapped));
            }

            // Отменить разметку (отвязать указатель от памяти)
            void unmap(VkDevice device){
                if (this->pMapped){
                    vkUnmapMemory(device, this->vkDeviceMemory);
                }
            }

            // Конфигурация дескриптора
            // Указываем какая именно память буфера будет доступна дескриптору (доступна для шейдера)
            void configDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
            {
                this->descriptorBufferInfo.offset = offset;
                this->descriptorBufferInfo.buffer = this->vkBuffer;
                this->descriptorBufferInfo.range = size;
            }
        };

        /**
        * Объект передаваемый в статический uniform-буфер, описывающий матрицы глобальной сцены
        * Структура с набором ОБЩИХ ДЛЯ ВСЕХ объектов матриц преобразований
        */
        struct UboWorld
        {
            glm::mat4 worldMatrix = {};
            glm::mat4 viewMatrix = {};
            glm::mat4 projectionMatrix = {};
        };

        /**
        * Объект передаваемый в динамический uniform-буфер, описывающий матрицы для каждого отдельного объекта
        * По сути является указателем на массив матриц, при его аллокации используется выравнивание
        */
        typedef glm::mat4 * UboModelArray;

        /**
        * Структура описывающая вершину
        * Содержит координаты вершины в 3 измерениях, цвет (RGB) и текстурные координаты (на плоскости)
        */
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texCoord;
            glm::uint32 textureUsed;
        };

        /**
        * Структура с набором примтивов синхронизации (семафоры)
        * Используется для синхронизации команд рендеринга и запросов показа изображения
        */
        struct Synchronization
        {
            VkSemaphore readyToRender = nullptr;
            VkSemaphore readyToPresent = nullptr;
        };

        /**
        * Стурктура описывает параметры камеры
        * Содержит параметры используемые для подготовки матриц проекции и вида
        *
        * Матрица проекции используется для проецирования 3-мерных точек на двумерную
        * плоскость. Для ее построения используются такие параметры как:
        * - Угол обзора
        * - Пропорции
        * - Ближняя граница отсечения (ближе которой ничего не будет отображаться)
        * - Дальняя граница отсечения (дальше которой все будет отбрасываться)
        *
        * Матрица вида отвечает за положение и поворот камеры в пространстве. Матрица
        * вида по своей сути - матрица перехода из одной системы коородинат в другую. То есть
        * веришины начинают проецироваться так, будто бы они сместились относительно некоего
        * глобального центра, с учетом положения локальной системы координат (камеры) относительного
        * глобальной, что и дает эффект перемещения наблюдателя
        */
        struct CameraSettings
        {
            // Положение и поворот камеры
            glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);

            // Угол обзора, пропорции, ближняя и дальняя грань отсечения
            float fFOV = 60.0f;
            float aspectRatio = 1.0f;
            float fNear = 0.1f;
            float fFar = 256.0f;

            // Подготовить матрицу проекции
            glm::mat4 MakeProjectionMatrix() const {
                glm::mat4 result = glm::perspective(glm::radians(this->fFOV), aspectRatio, fNear, fFar);
                result[1][1] *= -1; // Хотфикс glm для вулканом (glm разработан для opengl, а там ось Y инвертирована)
                return result;
            }

            // Подготовить матрицу вида
            glm::mat4 MakeViewMatrix() const {
                glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                rotationMatrix = glm::rotate(rotationMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

                glm::mat4 translationMatrix = glm::translate(glm::mat4(), this->position);

                return rotationMatrix * translationMatrix;
            }
        };

        /**
        * Стурктура описывает текстуру
        * Содержит изображение, а так же компоненты используемые для подачи данных в шейдер
        * - Изобаржение (в памяти устройства)
        * - Дескрипторный набор (отвечает за подачу данных в конвейер)
        */
        struct Texture
        {
            vkstructs::Image image = {};
            VkDescriptorSet descriptorSet = nullptr;

            void Deinit(VkDevice logicalDevice, VkDescriptorPool descriptorPool) {

                if (descriptorSet != nullptr) {
                    if (vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &(this->descriptorSet)) != VK_SUCCESS) {
                        this->descriptorSet = nullptr;
                        throw std::runtime_error("Vulkan: Error while destroying descriptor set");
                    }
                }

                image.Deinit(logicalDevice);
            }
        };

        /**
        * Стурктура описывающая примитив (набор вершин)
        * Содержит хендлы буферов вершин и индексов, а так же параметры положеняи примитива
        * - Позиция в глобальном пространстве
        * - Повторот относительно локального (своего) центра
        * - Масштаб (размер)
        */
        struct Primitive
        {
            bool drawIndexed = true;
            vkstructs::VertexBuffer vertexBuffer;
            vkstructs::IndexBuffer indexBuffer;
            const vkstructs::Texture * texture;
            glm::vec3 position = {};
            glm::vec3 rotation = {};
            glm::vec3 scale = {};
        };
    }

    namespace vkutility
    {
        //Проверка поддержки расширения устройством
        bool CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice,
                                           std::vector<const char*> deviceExtensionsNames);

        //Проверка на наличие запрашиваемых расширений у инстанса
        bool CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames);

        //Проверка на наличие запрашиваемых слолев валидации
        bool CheckValidationLayersSupported(std::vector<const char*> instanceExtensionsNames);

        VKAPI_ATTR VkBool32 VKAPI_CALL DebugVulkanCallback(
                VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objType,
                uint64_t obj,
                size_t location,
                int32_t code,
                const char* layerPrefix,
                const char* msg,
                void* userData);

        //Получение информации о семейсте очередей для конкретного физческого устройства и поверхности
        vkstructs::QueueFamilyInfo GetQueueFamilyInfo (
                VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                bool uniqueStrict = false);  /*! нужно ли что бы семекйства с командами были уникалными */

        /*Получить информацию о поверхности */
        vkstructs::SurfaceInfo GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        /**
        * Получить индекс типа памяти, которая поддерживает конкретные особенности
        * @param physicalDevice - хендл физического устройства информацю о возможных типах памяти которого нужно получить
        * @param unsigned int typeFlags - побитовая маска с флагами типов запрашиваемой памяти
        * @param VkMemoryPropertyFlags properties - параметры запрашиваемой памяти
        * @return int - возвращает индекс типа памяти, который соответствует всем условиям
        * @note - данный метод используется, например, при создании буферов
        */
        int GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags,
                               VkMemoryPropertyFlags properties);

        /**
        * Создание буфера
        * @param vkstructs::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться буфер
        * @param VkDeviceSize size - размер создаваемого буфера
        * @param VkBufferUsageFlags usage - как буфер будет использован (например, как вершинный - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
        * @param VkMemoryPropertyFlags properties - свойства памяти буфера (память устройства, память хоста, для "кого" память видима и т.д.)
        * @param VkSharingMode sharingMode - настройка доступа к памяти буфера для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
        * @return vkstructs::Buffer - структура содержающая хендл буфера, хендл памяти а так же размер буфера
        */
        vkstructs::Buffer CreateBuffer(const vkstructs::Device &device,
                                       VkDeviceSize size,
                                       VkBufferUsageFlags usage,
                                       VkMemoryPropertyFlags properties,
                                       VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
        /**
        * Создание простого однослойного изображения
        * @param vkstructs::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться изображение
        * @param VkImageType imageType - тип изображения (1D, 2D. 3D текстура)
        * @param VkFormat format - формат изображения
        * @param VkExtent3D extent - расширение (разрешение) изображения
        * @param VkImageUsageFlags usage - использование изображения (в качестве чего, назначение)
        * @param VkImageAspectFlags subresourceRangeAspect - использование области подресурса (???)
        * @param VkSharingMode sharingMode - настройка доступа к памяти изображения для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
        */
        vkstructs::Image CreateImageSingle(const vkstructs::Device &device,
                                           VkImageType imageType,
                                           VkFormat format,
                                           VkExtent3D extent,
                                           VkImageUsageFlags usage,
                                           VkImageAspectFlags subresourceRangeAspect,
                                           VkImageLayout initialLayout,
                                           VkMemoryPropertyFlags memoryProperties,
                                           VkImageTiling tiling,
                                           VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

        /**
        * Получить описание привязок вершинных данных к конвейеру
        * @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
        * @return std::vector<VkVertexInputBindingDescription> - массив описаний привязок
        *
        * @note - при привязывании буфера вершин к конвейеру, указывается индекс привязки. Нужно получить информацию
        * для конкретной привязки, о том как конвейер будет интерпретировать привязываемый буфер, какого размера
        * один элемент (вершина) в буфере, как переходить к следующему и тд. Вся информация в этой структуре
        */
        std::vector<VkVertexInputBindingDescription> GetVertexInputBindingDescriptions(unsigned int bindingIndex);

        /**
        * Получить описание аттрибутов привязываемых к конвейеру вершин
        * @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
        * @return std::vector<VkVertexInputAttributeDescription> - массив описаний атрибутов передаваемых вершин
        *
        * @note - конвейеру нужно знать как интерпретировать данные о вершинах. Какие у каждой вершины, в привязываемом буфере,
        * есть параметры (аттрибуты). В какой последовательности они идут, какого типа каждый аттрибут.
        */
        std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions(unsigned int bindingIndex);

        /**
        * Загрузка шейдерного модуля из файла
        * @param std::string filename - наименование файла шейдера, поиск по умолчанию в папке shaders
        * @param VkDevice logicalDevice - хендл логичекского устройства, нужен для созданния шейдерного модуля
        * @return VkShaderModule - хендл шейдерного модуля созданного из загруженного файла
        */
        VkShaderModule LoadSPIRVShader(std::string filename,
                                       VkDevice logicalDevice);

        /**
        * Изменить размещение изображения
        * @param VkCommandBuffer cmdBuffer - хендл командного буфера, в который будет записана команда смены размещения
        * @param VkImage image - хендл изображения, размещение которого нужно сменить
        * @param VkImageLayout oldImageLayout - старое размещение
        * @param VkImageLayout newImageLayout - новое размещение
        * @param VkImageSubresourceRange subresourceRange - описывает какие регионы изображения подвергнутся переходу размещения
        */
        void CmdImageLayoutTransition(VkCommandBuffer cmdBuffer,
                                      VkImage image,
                                      VkImageLayout oldImageLayout,
                                      VkImageLayout newImageLayout,
                                      VkImageSubresourceRange subresourceRange);

        /**
        * Копировать изображение
        * @param VkCommandBuffer cmdBuffer - хендл командного буфера, в который будет записана команда смены размещения
        * @param VkImage srcImage - исходное изображение, память которого нужно скопировать
        * @param VkImage dstImage - целевое изображение, в которое нужно перенести память
        * @param uint32_t width   - ширина
        * @param uint32_t height  - высота
        */
        void CmdImageCopy(VkCommandBuffer cmdBuffer,
                          VkImage srcImage,
                          VkImage dstImage,
                          uint32_t width,
                          uint32_t height);

        /**
        * Создать буфер одиночных команд
        * @param const vkstructs::Device &device - устройство
        * @param VkCommandPool commandPool - командный пул, из которого будет выделен буфер
        * @return VkCommandBuffer - хендл нового буфера
        */
        VkCommandBuffer CreateSingleTimeCommandBuffer(const vkstructs::Device &device,
                                                      VkCommandPool commandPool);

        /**
        * Отправить команду на исполнение и очистить буфер
        * @param const vkstructs::Device &device - устройство
        * @param VkCommandPool commandPool - командный пул, из которого был выделен буфер
        * @param VkCommandBuffer commandBuffer - командный буфер
        */
        void FlushSingleTimeCommandBuffer(const vkstructs::Device &device,
                                          VkCommandPool commandPool,
                                          VkCommandBuffer commandBuffer,
                                          VkQueue queue);


    }

    namespace tools
    {
        /**
        * Путь к рабочему каталогу
        * @return std::string - строка содержащая путь к директории
        * @note нужно учитывать что рабочий каталок может зависеть от конфигурации проекта
        */
        //std::string WorkingDir();

        /**
        * Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe)
        * @return std::string - строка содержащая путь к директории
        */
        //std::string ExeDir();

        /**
        * Получить текущее время
        * @return std::time_t - числовое значение текущего момента системного времени
        */
        std::time_t CurrentTime();

        /**
        * Получить время в виде отформатированной строки (напр. ГГГГ-ММ-ДД ЧЧ:ММ:CC)
        * @param std::time_t * time - числовое значение времени
        * @param const char * format - формат в виде c-строки (напр. %Y-%m-%d %H:%M:%S)
        * @return std::string - строка с отформатированным временем
        * @note подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
        */
        std::string TimeToStr(std::time_t * time, const char * format);

        /**
        * Логирование. Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
        * @param std::string message - строка с соощением
        * @param bool printTime - выводить ли время
        */
        void LogMessage(std::string message, bool printTime = true);

        /**
        * То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
        * @param std::string message - строка с соощением
        * @param bool printTime - выводить ли время
        */
        void LogError(std::string message, bool printTime = true);

        /**
        * Конвертация из обычной string-строки в "широкую" wstring
        * @param const std::string& str - исходная string строка
        * @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
        * @param DWORD dwFlags - тип конвертации (как конвертировать простые символы в составные)
        * @return std::wstring - wide (широкая) строка
        * @note это обертка winapi метода MultiByteToWideChar - https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
        */
        std::wstring StrToWide(const std::string& str, UINT codePage = CP_ACP, DWORD dwFlags = MB_PRECOMPOSED);

        /**
        * Конвертация из "широкой" wstring-строки в обычную string
        * @param const std::wstring& wstr - исходная wstring (широкая) строка
        * @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
        * @param DWORD dwFlags - тип конвертации (как конвертировать составные символы в простые)
        * @return std::wstring - string строка
        * @note это обертка winapi метода WideCharToMultiByte - https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130(v=vs.85).aspx
        */
        std::string WideToStr(const std::wstring& wstr, UINT codePage = CP_ACP, DWORD dwFlags = WC_COMPOSITECHECK);

        /**
        * Загрузка бинарных данных из файла
        * @param const std::string &path - путь к файлу
        * @param char** pData - указатель на указатель на данные, память для которых будет аллоцирована
        * @param size_t * size - указатель на параметр размера (размер будет получен при загрузке)
        * @return bool - состояние загрузки (удалось или нет)
        */
        bool LoadBytesFromFile(const std::string &path, char** pData, size_t * size);
    }
}
