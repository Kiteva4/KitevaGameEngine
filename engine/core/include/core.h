#ifndef CORE_H
#define CORE_H

#include <graphic/KGEVulkanApp.h>
#include <application/KGEAppData.h>

class Core
{
    KGEAppData m_kgeAppData;
    KGEVulkanApp m_kgeAppication;

public:
    Core();
    ~Core();
};

#endif // CORE_H
