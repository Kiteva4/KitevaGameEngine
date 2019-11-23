#ifndef CORE_H
#define CORE_H

#include<initializer.h>
#include <vk_icd.h>
class Core
{

public:
    Core()
    {

    }

    Initializer initializer;

    Core operator[](KGEObject const &obj)
    {
    }
};

#endif // CORE_H
