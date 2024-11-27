#pragma once

#include "DataModels/Components/Component.h"
#include "DataModels/FileSystem/UID/UID.h"

#include "DataModels/FileSystem/Json/Field.h"

enum class HierarchyState
{
    SELECTED,
    CHILD_SELECTED,
    NONE
};

class GameObject
{
public:
    using GameObjectView =
        std::ranges::transform_view<std::ranges::ref_view<const std::vector<std::unique_ptr<GameObject>>>,
        std::function<GameObject* (const std::unique_ptr<GameObject>&)>>;
    using ComponentsView =
        std::ranges::transform_view<std::ranges::ref_view<const std::vector<std::unique_ptr<Component>>>,
        std::function<Component* (const std::unique_ptr<Component>&)>>;

    explicit GameObject(const std::string& name);
    GameObject(const std::string& name, GameObject* parent);
    GameObject(const GameObject& copy);
    ~GameObject();

    void Save(Field& meta);
    // ------------- CHILDREN METHODS ----------------------

    void LinkChild(GameObject* child);
    [[nodiscard]] GameObject* UnLinkChild(GameObject* child);
    void MoveChild(GameObject* child, int newPos);

    bool IsChild(GameObject* child);
    bool IsDescendant(GameObject* child);

    // ------------- COMPONENTS METHODS ----------------------

    template<typename C>
    C* CreateComponent();
    // This method is intended to be used by the classes of the Engine, not its users
    // In case the component of the given type is not found, a nullptr is returned
    template<typename C>
    C* GetInternalComponent() const;
    template<typename C>
    std::vector<C*> GetInternalComponents() const;
    template<typename C>
    bool RemoveComponent();
    template<typename C>
    bool RemoveComponents();
    bool RemoveComponent(Component* component);
    template<typename C>
    inline bool HasComponent();

    // ------------- GETTERS ----------------------

    inline const UID GetUID() const;
    inline const std::string& GetName();
    inline bool& IsEnabled();
    inline bool IsActive() const;
    inline bool IsStatic() const;
    inline const std::string& GetTag();
    inline GameObject* GetParent() const;
    inline bool IsRoot() const;
    inline GameObjectView GetChildren() const;
    // Return itself
    inline std::list<GameObject*> GetAllDescendants();
    // Return itself
    inline std::list<GameObject*> GetAllAscendants();
    inline ComponentsView GetComponents() const;
    inline bool HasChildren() const;
    inline size_t HowManyComponentsHas() const;
    inline const HierarchyState GetHierarchySate() const;

    // ------------- SETTERS ----------------------

    inline void SetName(const std::string& name);
    void SetParent(GameObject* parent);
    void SetStatic(bool isStatic);
    inline void SetTag(const std::string& tag);
    inline void SetHierarchyState(HierarchyState newState);

private:
    GameObject(const std::string& name,
        GameObject* parent,
        UID uid,
        bool enabled,
        bool active,
        bool staticObject);

    // ------------- COMPONENTS METHODS ----------------------

    Component* CreateComponent(ComponentType type);
    void CopyComponent(Component* copyComponent);
    void AddComponent(Component* newComponent);

private:
    UID _uid;
    std::string _name;
    GameObject* _parent;

    bool _enabled;
    bool _active;
    bool _static;
    std::string _tag;

    std::vector<std::unique_ptr<GameObject>> _children;
    std::vector<std::unique_ptr<Component>> _components;

    HierarchyState _hierarchyState;
};

inline const UID GameObject::GetUID() const
{
    return _uid;
}

inline const std::string& GameObject::GetName()
{
    return _name;
}

inline bool& GameObject::IsEnabled()
{
    return _enabled;
}

inline bool GameObject::IsActive() const
{
    return _active && _enabled;
}

inline bool GameObject::IsStatic() const
{
    return _static;
}

inline const std::string& GameObject::GetTag()
{
    return _tag;
}

inline GameObject* GameObject::GetParent() const
{
    return _parent;
}

inline bool GameObject::IsRoot() const
{
    return _parent == nullptr;
}

inline GameObject::GameObjectView GameObject::GetChildren() const
{
    std::function<GameObject* (const std::unique_ptr<GameObject>&)> lambda = [](const std::unique_ptr<GameObject>& go)
        {
            return go.get();
        };
    return std::ranges::transform_view(_children, lambda);
}

inline std::list<GameObject*> GameObject::GetAllDescendants()
{
    std::list<GameObject*> descendants = {};
    descendants.push_back(this);
    for (auto& child : _children)
    {
        auto childDescendants = child->GetAllDescendants();
        descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
    }
    return descendants;
}

inline std::list<GameObject*> GameObject::GetAllAscendants()
{
    std::list<GameObject*> ascendants = {};
    ascendants.push_back(this);

    auto parent = _parent;
    while (parent != nullptr)
    {
        ascendants.push_back(parent);
        parent = parent->_parent;
    }
    return ascendants;
}

inline GameObject::ComponentsView GameObject::GetComponents() const
{
    std::function<Component* (const std::unique_ptr<Component>&)> lambda = [](const std::unique_ptr<Component>& component)
        {
            return component.get();
        };
    return std::ranges::transform_view(_components, lambda);
}

inline bool GameObject::HasChildren() const
{
    return _children.size() > 0;
}

inline size_t GameObject::HowManyComponentsHas() const
{
    return _components.size();
}

inline const HierarchyState GameObject::GetHierarchySate() const
{
    return _hierarchyState;
}

inline void GameObject::SetName(const std::string& name)
{
    _name = name;
}

inline void GameObject::SetTag(const std::string& tag)
{
    _tag = tag;
}

inline void GameObject::SetHierarchyState(HierarchyState newState)
{
    _hierarchyState = newState;

    if (_hierarchyState == HierarchyState::SELECTED)
    {
        auto parent = _parent;
        while (parent != nullptr)
        {
            parent->_hierarchyState = HierarchyState::CHILD_SELECTED;
            parent = parent->_parent;
        }
    }
    else if (_hierarchyState == HierarchyState::NONE)
    {
        auto parent = _parent;
        while (parent != nullptr)
        {
            parent->_hierarchyState = HierarchyState::NONE;
            parent = parent->_parent;
        }
    }
}

#include "GameObject.inl"