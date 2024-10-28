#pragma once

#include "DataModels/Components/Component.h"
#include "DataModels/FileSystem/UID.h"

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
    GameObject(const std::string& name, GameObject* parent, UID uid); // delete
    GameObject(const std::string& name, GameObject* parent);
    GameObject(const GameObject& copy);
    ~GameObject();

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
    inline GameObjectView GetChildren() const;
    inline ComponentsView GetComponents() const;
    inline bool HasChildren() const;
    inline size_t HowManyComponentsHas() const;

    // ------------- SETTERS ----------------------

    inline void SetName(const std::string& name);
    void SetParent(GameObject* parent);
    void SetStatic(bool isStatic);

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
    std::string _name;
    GameObject* _parent;
    UID _uid;
    
    bool _enabled;
    bool _active;
    bool _static;
    std::string _tag;

    std::vector<std::unique_ptr<GameObject>> _children;
    std::vector<std::unique_ptr<Component>> _components;
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

inline GameObject::GameObjectView GameObject::GetChildren() const
{
    std::function<GameObject* (const std::unique_ptr<GameObject>&)> lambda = [](const std::unique_ptr<GameObject>& go)
        {
            return go.get();
        };
    return std::ranges::transform_view(_children, lambda);
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

inline void GameObject::SetName(const std::string& name)
{
    _name = name;
}

#include "GameObject.inl"