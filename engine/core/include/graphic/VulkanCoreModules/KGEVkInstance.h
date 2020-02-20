#ifndef KGEVKINSTANCE_H
#define KGEVKINSTANCE_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class KGEVkInstance
{   
    VkInstance m_instance;
public:


    /**
    * Метод инициализации instance'а vulkan'а
    * @param std::string applicationName - наименование приложения (информация может быть полезна разработчикам драйверов)
    * @param std::string engineName - наименование движка (информация может быть полезна разработчикам драйверов)
    * @param std::vector<const char*> extensionsRequired - запрашиваемые расширения экземпляра
    * @param std::vector<const char*> validationLayersRequired - запрашиваемые слои валидации
    * @return VkInstance - хендл экземпляра Vulkan
    */
    KGEVkInstance(std::string applicationName,
                  std::string engineName,
                  std::vector<const char*> extensionsRequired,
                  std::vector<const char*> validationLayersRequired);

    ~KGEVkInstance();
    const VkInstance &instance();
};

#endif // KGEVKINSTANCE_H
