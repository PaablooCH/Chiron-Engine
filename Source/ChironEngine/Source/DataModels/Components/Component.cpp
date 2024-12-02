#include "Pch.h"
#include "Component.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "Interfaces/Drawable.h"
#include "Interfaces/Updatable.h"

#include "DataModels/GameObject/GameObject.h"

Component::Component(ComponentType type, GameObject* owner) : _type(type), _owner(owner), _enabled(true)
{
    if (auto* drawable = dynamic_cast<Drawable*>(this)) 
    {
        App->GetModule<ModuleScene>()->AddDrawableComponent(drawable);
    }
    if (auto* updatable = dynamic_cast<Updatable*>(this))
    {
        App->GetModule<ModuleScene>()->AddUpdatableComponent(updatable);
    }
}

Component::Component(const Component& copy) : _type(copy._type), _owner(copy._owner), _enabled(copy._enabled)
{
}

Component::~Component()
{
    if (auto* drawable = dynamic_cast<Drawable*>(this))
    {
        App->GetModule<ModuleScene>()->RemoveDrawableComponent(drawable);
    }
    if (auto* updatable = dynamic_cast<Updatable*>(this))
    {
        App->GetModule<ModuleScene>()->RemoveUpdatableComponent(updatable);
    }
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