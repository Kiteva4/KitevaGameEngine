#pragma once

#define ENABLE_DEBUG_LAYERS

#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

namespace kge
{
    namespace vkstructs
    {
        /*! Индексы семейств о поддерживаемых командах */
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
                return  physicalDevice != nullptr &&
                        logicalDevice != nullptr &&
                        queues.graphics != nullptr &&
                        queues.present != nullptr &&
                        queueFamilies.IsRenderingCompatible();
            }

            void Deinit(){
                if(logicalDevice != nullptr){
                    vkDestroyDevice(logicalDevice, nullptr);
                    logicalDevice = nullptr;
                }

                physicalDevice = nullptr;
                queues.graphics = nullptr;
                queues.present = nullptr;
                queueFamilies = {};
            }
        };

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
    }

    namespace vkutility
    {
        //Проверка поддержки расширения устройством
        bool CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice,
                                           std::vector<const char*> deviceExtensionsNames);

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

        //Получение информации о семейсте очередей для конкретного физческого устройства и поверхности
        vkstructs::QueueFamilyInfo GetQueueFamilyInfo (
                VkPhysicalDevice physicalDevice,
                VkSurfaceKHR surface,
                bool uniqueStrict = false);  /*! нужно ли что бы семекйства с командами были уникалными */

        /*Получить информацию о поверхности */
        vkstructs::SurfaceInfo GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    }
}
