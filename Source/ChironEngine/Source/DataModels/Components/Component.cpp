#include "Pch.h"
#include "Component.h"

#include "DataModels/GameObject/GameObject.h"

Component::Component(ComponentType type, GameObject* owner) : _type(type), _owner(owner), _enabled(true)
{
}

Component::Component(const Component& copy) : _type(copy._type), _owner(copy._owner), _enabled(copy._enabled)
{
}

Component::~Component()
{
}

void Component::Save(Field& meta)
{
    meta["type"] = ComponentTypeUtils::ToString(_type);
    meta["enabled"] = _enabled;
    InternalSave(meta);
}

void Component::Load(const Field& meta)
{
    _enabled = meta["enabled"];
    InternalLoad(meta);
}

bool Component::IsActive()
{
    return _owner->IsActive() && _enabled;
}