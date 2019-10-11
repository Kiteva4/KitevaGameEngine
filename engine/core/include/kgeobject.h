#ifndef KDEOBJECT_H
#define KDEOBJECT_H

#include <ikgeobject.h>

class KGEObject : public IKGEObject
{
public:
    KGEObject() {}

    ~KGEObject() override {}

    void Init() override;

    void Start() override;

    void Update() override;

    void Stop() override;
};

#endif // KDEOBJECT_H
