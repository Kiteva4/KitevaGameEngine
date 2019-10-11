#ifndef CORE_H
#define CORE_H

#include<initializer.h>

class Core
{

public:
    Core()
    {

    }

    Initializer initializer;

    Core operator[](KGEObject const &obj)
    {
        initializer.AddToInitLists(std::bind( &KGEObject::Init, obj));
        initializer.AddToStartList(std::bind( &KGEObject::Start, obj));
        initializer.AddToUpdateList(std::bind( &KGEObject::Update, obj));
        initializer.AddToStopList(std::bind( &KGEObject::Stop, obj));
    }
};

#endif // CORE_H
