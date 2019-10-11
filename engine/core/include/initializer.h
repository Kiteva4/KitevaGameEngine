#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <list>
#include <functional>
#include <ikgeobject.h>
#include <kgeobject.h>

class Initializer
{

public:

    Initializer();


    void Init()
    {
        for(auto f : InitLists)
            f();

        for(auto f : StartList)
            f();
    }

    void Start()
    {
        for(auto f : StartList)
            f();
    }

    void Update()
    {
        for(auto f : UpdateList)
            f();
    }

    void Stop()
    {
        for(auto f : StopList)
            f();
    }

    void AddToInitLists(std::function<void(void)>)  {}
    void AddToStartList(std::function<void(void)>)  {}
    void AddToUpdateList(std::function<void(void)>) {}
    void AddToStopList(std::function<void(void)>)   {}
    ~Initializer() { ClearLists(); }
private:



    std::list<std::function<void(void)>> InitLists;

    std::list<std::function<void(void)>> StartList;

    std::list<std::function<void(void)>> UpdateList;

    std::list<std::function<void(void)>> StopList;

    void ClearLists()
    {
        InitLists.clear();
        StartList.clear();
        UpdateList.clear();
        StopList.clear();
    }
};


#endif // INITIALIZER_H
