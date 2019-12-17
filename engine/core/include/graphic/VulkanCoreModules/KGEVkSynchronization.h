#ifndef KGEVKSYNCHRONIZATION_H
#define KGEVKSYNCHRONIZATION_H

#include <graphic/KGEVulkan.h>

class KGEVkSynchronization
{
    kge::vkstructs::Synchronization* m_sync;
    const kge::vkstructs::Device* m_device;
public:
    KGEVkSynchronization(kge::vkstructs::Synchronization* sync,
                         const kge::vkstructs::Device* device);
    ~KGEVkSynchronization();
};

#endif // KGEVKSYNCHRONIZATION_H
