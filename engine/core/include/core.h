#ifndef CORE_H
#define CORE_H

#include <graphic/GKEVulkanApp.h>
#include <application/KGEAppData.h>

class Core
{
    KGEAppData m_kgeAppData;
    GKEVulkanApp m_kgeAppication;

public:
    Core();
    ~Core();
};

#endif // CORE_H
