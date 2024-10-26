#include <concepts>

namespace Chiron::detail
{
    template<typename T>
    concept CompleteType = requires
    {
        sizeof(T);
    };

#define ASSERT_TYPE_COMPLETE(T)                                                                    \
	static_assert(Chiron::detail::CompleteType<T>,                                                    \
				  "Trying to call method template with an incomplete type. Check the type and " \
				  "the corresponding include.");
} // namespace Chiron::detail

template<typename C>
inline C* GameObject::CreateComponent()
{
    ASSERT_TYPE_COMPLETE(C);
    return static_cast<C*>(CreateComponent(ComponentToEnum<C>::value));
}

template<typename C>
inline C* GameObject::GetInternalComponent() const
{
    ASSERT_TYPE_COMPLETE(C);
    auto firstComponent = std::ranges::find_if(_components,
        [](const std::unique_ptr<Component>& component)
        {
            return component && component->GetType() == ComponentToEnum<C>::value;
        });
    return firstComponent != _components.end() ? static_cast<C*>((*firstComponent).get()) : nullptr;
}

template<typename C>
inline std::vector<C*> GameObject::GetComponents() const
{
    ASSERT_TYPE_COMPLETE(C);
    auto viewComponents = _components |
        std::views::filter(
            [](const std::unique_ptr<Component>& component)
            {
                return component && component->GetType() == ComponentToEnum<C>::value;
            })
        |
        std::views::transform(
            [](const std::unique_ptr<Component>& component)
            {
                return static_cast<C*>(component.get());
            });
    return std::vector<C*>(viewComponents.begin(), viewComponents.end());
}

template<typename C>
inline bool GameObject::RemoveComponent()
{
    ASSERT_TYPE_COMPLETE(C);
    return RemoveComponent(GetInternalComponent<C>());
}

template<typename C>
inline bool GameObject::RemoveComponents()
{
    ASSERT_TYPE_COMPLETE(C);
    int i = 0;
    while (RemoveComponent<C>())
    {
        i++;
    }

    if (i > 0)
    {
        return true;
    }
    return false;
}

template<typename C>
inline bool GameObject::HasComponent()
{
    ASSERT_TYPE_COMPLETE(C);
    return GetInternalComponent<C>() != nullptr;
}