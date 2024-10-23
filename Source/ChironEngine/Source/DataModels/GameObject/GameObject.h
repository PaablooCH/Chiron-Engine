#pragma once

#include "DataModels/Components/Component.h"

enum class ComponentType;

class GameObject
{
public:
    using GameObjectView =
        std::ranges::transform_view<std::ranges::ref_view<const std::vector<std::unique_ptr<GameObject>>>,
        std::function<GameObject* (const std::unique_ptr<GameObject>&)>>;

    explicit GameObject(const std::string& name);
    GameObject(const std::string& name, GameObject* parent);
    GameObject(const GameObject& copy);
    ~GameObject();

    // ------------- CHILDREN METHODS ----------------------

    void LinkChild(GameObject* child);
    [[nodiscard]] GameObject* UnLinkChild(GameObject* child);

    bool IsChild(GameObject* child);

    // ------------- COMPONENTS METHODS ----------------------

    template<typename C>
    C* CreateComponent();
    // This method is intended to be used by the classes of the Engine, not its users
    // In case the component of the given type is not found, a nullptr is returned
    template<typename C>
    C* GetInternalComponent() const;
    template<typename C>
    std::vector<C*> GetComponents() const;
    template<typename C>
    bool RemoveComponent();
    template<typename C>
    bool RemoveComponents();
    bool RemoveComponent(const Component* component);
    template<typename C>
    bool HasComponent();

    // ------------- GETTERS ----------------------

    inline const std::string& GetName();
    inline bool& IsEnabled();
    inline bool IsActive() const;
    inline bool IsStatic() const;
    inline const std::string& GetTag();
    inline GameObject* GetParent() const;
    inline GameObjectView GetChildren() const;

    // ------------- SETTERS ----------------------

    void SetStatic(bool isStatic);

private:
    GameObject(const std::string& name,
        GameObject* parent,
        bool enabled,
        bool active,
        bool staticObject);

    // ------------- COMPONENTS METHODS ----------------------

    Component* CreateComponent(ComponentType type);
    bool RemoveComponent(Component* component);
    void CopyComponent(Component* copyComponent);
    void AddComponent(Component* newComponent);

private:
    std::string _name;
    
    bool _enabled;
    bool _active;
    bool _static;
    std::string _tag;

    GameObject* _parent;
    std::vector<std::unique_ptr<GameObject>> _children;
    std::vector<std::unique_ptr<Component>> _components;
};

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

#include "GameObject.inl"