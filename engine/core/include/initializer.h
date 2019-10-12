#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <map>
#include <functional>
#include <stdint.h>
#include <ikgeobject.h>
#include <kgeobject.h>
#include <iostream>

class Initializer
{

public:

    Initializer();

    void Init()
    {
        for(auto& obj : KGEObjects)
        {
            std::cout << "KGEObject " << obj.first << "Init" << std::endl;
            obj.second.Init();
        }
    }

    void Start()
    {
        for(auto& obj : KGEObjects)
        {
            std::cout << "KGEObject " << obj.first << " Start" << std::endl;
            obj.second.Init();
        }
    }

    void Update()
    {
        for(auto& obj : KGEObjects)
        {
            std::cout << "KGEObject " << obj.first << " Update" << std::endl;
            obj.second.Update();
        }
    }

    void Stop()
    {
        for(auto& obj : KGEObjects)
        {
            std::cout << "KGEObject " << obj.first << " Stop" << std::endl;
            obj.second.Stop();
        }
    }

    void AddKGEObjectToInitLists(KGEObject* adr, const KGEObject& obj)
    {
        uint32_t id = *(uint32_t *) &obj;

        KGEObjects.insert({ id, obj});
    }

    ~Initializer() { ClearLists(); }

private:

    std::map<uint32_t, KGEObject> KGEObjects;

    void ClearLists()
    {
        KGEObjects.clear();
    }
};


#endif // INITIALIZER_H
