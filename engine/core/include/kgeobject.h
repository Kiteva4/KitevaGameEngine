#ifndef KDEOBJECT_H
#define KDEOBJECT_H

#include <ikgeobject.h>
#include <string>

class KGEObject : public IKGEObject
{
public:
    KGEObject()
    {
    }

    ~KGEObject() override {}

    void Init() override;

    void Start() override;

    void Update() override;

    void Stop() override;

    std::string id;
};

#endif // KDEOBJECT_H
