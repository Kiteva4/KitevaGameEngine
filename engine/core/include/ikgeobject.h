#ifndef IKGEOBJECT1_H
#define IKGEOBJECT1_H

class IKGEObject
{
public:

    IKGEObject() {

    }

    virtual void Init() = 0;

    virtual void Start() = 0;

    virtual void Update() = 0;

    virtual void Stop() = 0;

    virtual ~IKGEObject() {}
};

#endif // IKGEOBJECT_H


