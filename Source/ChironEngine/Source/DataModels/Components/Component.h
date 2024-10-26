#pragma once

#include "Enums/ComponentType.h"

class GameObject;

class Component
{
public:
    Component(ComponentType type, GameObject* owner);
    Component(const Component& copy);
    virtual ~Component();

    // ------------- GETTERS ----------------------

    inline ComponentType GetType() const;
    inline GameObject* GetOwner();
    inline bool& IsEnabled();
    bool IsActive();

    // ------------- GETTERS ----------------------

    inline void SetOwner(GameObject* owner);

protected:
    GameObject* _owner;

private:
    ComponentType _type;

    bool _enabled;
};

inline ComponentType Component::GetType() const
{
    return _type;
}

inline GameObject* Component::GetOwner()
{
    return _owner;
}

inline bool& Component::IsEnabled()
{
    return _enabled;
}

inline void Component::SetOwner(GameObject* owner)
{
    _owner = owner;
}