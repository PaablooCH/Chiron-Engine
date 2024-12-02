#include "Pch.h"
#include "GameObject.h"

#include "Application.h"

#include "Modules/ModuleScene.h"

#include "DataModels/Components/TransformComponent.h"
#include "DataModels/Components/Interfaces/Drawable.h"
#include "DataModels/Components/Interfaces/Updatable.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

// root constructor
GameObject::GameObject(const std::string& name) : GameObject(name, nullptr, Chiron::UIDGenerator::GenerateUID(), true, true,
    true)
{
}

// JSON constructor
GameObject::GameObject(const Field& meta) : _uid(meta["uid"]), _name(meta["name"]), _parent(nullptr), 
_enabled(meta["enabled"]), _active(false), _static(meta["static"]), _tag(meta["tag"]), _hierarchyState(HierarchyState::NONE)
{
    App->GetModule<ModuleScene>()->AddGameObject(this);
    
    auto components = meta["Components"];
    for (int i = 0; i < components.Size(); i++)
    {
        auto componentField = components[i]["Component"];
        ComponentType type = ComponentTypeUtils::FromString(componentField["type"]);
        CreateComponent(type);
    }
}

GameObject::GameObject(const std::string& name, GameObject* parent) : GameObject(name, parent,
    Chiron::UIDGenerator::GenerateUID(), true, parent->IsActive(), parent->IsStatic())
{
    _parent->LinkChild(this);
    App->GetModule<ModuleScene>()->AddGameObject(this);
}

GameObject::GameObject(const std::string& name, GameObject* parent, UID uid, bool enabled, bool active, bool staticObject)
    : _name(name), _parent(parent), _uid(uid), _enabled(enabled), _active(active), _static(staticObject),
    _hierarchyState(HierarchyState::NONE)
{
    CreateComponent<TransformComponent>();
}

GameObject::GameObject(const GameObject& copy) : GameObject(copy._name, copy._parent, Chiron::UIDGenerator::GenerateUID(),
    copy._enabled, copy._active, copy._static)
{
    std::ranges::for_each(copy._components.begin(), copy._components.end(),
        [this](const std::unique_ptr<Component>& copyComponent)
        {
            this->CopyComponent(copyComponent.get());
        });

    std::ranges::for_each(copy._children.begin(), copy._children.end(),
        [this](const std::unique_ptr<GameObject>& copyChild)
        {
            GameObject* newChild = new GameObject(*copyChild.get());
            this->LinkChild(newChild);
        });
}

GameObject::~GameObject()
{
    auto moduleScene = App->GetModule<ModuleScene>();
    moduleScene->RemoveFromScene(this);

    _children.clear();
    _components.clear();
}

void GameObject::Save(Field& meta)
{
    meta["uid"] = _uid;
    meta["name"] = _name;
    meta["uidParent"] = _parent ? _parent->_uid : 0;
    meta["enabled"] = _enabled;
    meta["static"] = _static;
    meta["tag"] = _tag;

    auto components = meta["Components"];
    for (int i = 0; i < _components.size(); i++)
    {
        auto field = components[i]["Component"];
        _components[i]->Save(field);
    }
}

void GameObject::Load(const Field& meta)
{
    for (int i = 0; i < meta.Size(); i++)
    {
        auto componentField = meta[i]["Component"];
        auto& component = _components[i];
        component->Load(componentField);
    }
}

void GameObject::ReGenerateUID()
{
    _uid = Chiron::UIDGenerator::GenerateUID();
}

void GameObject::OnAwake()
{
    for (auto& component : _components)
    {
        component->OnAwake();
    }
}

void GameObject::SetParent(GameObject* parent)
{
    assert(parent);
    if (parent->IsChild(this) || IsDescendant(parent))
    {
        return;
    }
    std::ignore = _parent->UnLinkChild(this);

    parent->LinkChild(this);
    auto transform = GetInternalComponent<TransformComponent>();
    auto newParentTransform = parent->GetInternalComponent<TransformComponent>();
    if (newParentTransform && transform)
    {
        transform->CalculateLocalFromNewGlobal(newParentTransform);
    }
}

void GameObject::SetEnabled(bool enabled)
{
    _enabled = enabled;
    
    for (auto& child : _children)
    {
        child->PropagateActive(_enabled && _active);
    }
}

void GameObject::SetStatic(bool isStatic)
{
    _static = isStatic;
    if (_static)
    {
        App->GetModule<ModuleScene>()->RemoveDynamicGO(this);
        App->GetModule<ModuleScene>()->AddStaticGO(this);
    }
    else
    {
        App->GetModule<ModuleScene>()->RemoveStaticGO(this);
        App->GetModule<ModuleScene>()->AddDynamicGO(this);
    }
    for (auto& child : _children)
    {
        child->SetStatic(_static);
    }
}

void GameObject::LinkChild(GameObject* child)
{
    assert(child);

    if (!IsChild(child))
    {
        child->_parent = this;
        child->_active = IsActive();

        _children.push_back(std::unique_ptr<GameObject>(child));

        child->GetInternalComponent<TransformComponent>()->UpdateMatrices();
    }
}

GameObject* GameObject::UnLinkChild(GameObject* child)
{
    assert(child);

    if (IsChild(child))
    {
        auto childIt = std::ranges::find_if(_children,
            [child](std::unique_ptr<GameObject>& actualChild)
            {
                return actualChild.get() == child;
            });

        auto orphan = childIt->release();
        _children.erase(childIt);

        return orphan;
    }

    return nullptr;
}

void GameObject::MoveChild(GameObject* child, int newPos)
{
    for (int i = 0; i < _children.size(); i++)
    {
        if (_children[i].get() == child)
        {
            if ((i + newPos == _children.size()) || (i + newPos < 0))
            {
                LOG_WARNING("Couldn't move child to the new position.");
                return;
            }
            std::rotate(_children.begin() + i, _children.begin() + i + 1, _children.begin() + newPos + (newPos > i ? 1 : 0));
            return;
        }
    }
    LOG_TRACE("{} is not a child of {}", child, this);
}

bool GameObject::IsChild(GameObject* child)
{
    return std::ranges::any_of(_children.begin(), _children.end(),
        [child](std::unique_ptr<GameObject>& actualChild)
        {
            return actualChild.get() == child;
        });
}

bool GameObject::IsDescendant(GameObject* gameObject)
{
    std::queue<GameObject*> descendants;
    descendants.push(this);
    while (!descendants.empty())
    {
        GameObject* descendant;
        Chiron::Utils::TryFrontPop(descendants, descendant);
        if (descendant == gameObject)
        {
            return true;
        }

        for (auto& child : _children)
        {
            descendants.push(child.get());
        }
    }
    return false;
}

bool GameObject::RemoveComponent(Component* deleteComponent)
{
    auto newEnd = std::remove_if(_components.begin(), _components.end(),
        [&deleteComponent](const std::unique_ptr<Component>& component)
        {
            return component.get() == deleteComponent;
        });

    if (newEnd == _components.end())
    {
        LOG_WARNING("Couldn't delete component from {}. Doesn't exist.", this);
        return false;
    }

    _components.erase(newEnd, _components.end());
    return true;
}

void GameObject::PropagateActive(bool active)
{
    _active = active;
    for (auto& child : _children)
    {
        child->PropagateActive(_enabled && _active);
    }
}

Component* GameObject::CreateComponent(ComponentType type)
{
    Component* newComponent = nullptr;
    switch (type)
    {
    case ComponentType::TRANSFORM:
        newComponent = new TransformComponent(this);
        break;
    case ComponentType::MESH_RENDERER:
        break;
    case ComponentType::UNKNOWN:
        LOG_ERROR("UNKNOWN component type");
        break;
    }

    AddComponent(newComponent);

    return newComponent;
}

void GameObject::CopyComponent(Component* copyComponent)
{
    Component* newComponent = nullptr;
    switch (copyComponent->GetType())
    {
    case ComponentType::TRANSFORM:
        newComponent = new TransformComponent(static_cast<TransformComponent&>(*copyComponent));
        break;
    case ComponentType::MESH_RENDERER:

        break;
    case ComponentType::UNKNOWN:
        LOG_ERROR("UNKNOWN component type");
        break;
    }

    AddComponent(newComponent);
}

void GameObject::AddComponent(Component* newComponent)
{
    if (newComponent)
    {
        newComponent->SetOwner(this);
        _components.push_back(std::unique_ptr<Component>(newComponent));
    }
}