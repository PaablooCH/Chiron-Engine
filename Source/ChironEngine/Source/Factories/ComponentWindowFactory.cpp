#include "Pch.h"
#include "ComponentWindowFactory.h"

#include "DataModels/Components/TransformComponent.h"

#include "DataModels/Window/EditorWindow/SubWindows/Inspector/TransformComponentWindow.h"

std::unique_ptr<ComponentWindow> ComponentWindowFactory::CreateComponentWindow(Component* component)
{
    switch (component->GetType())
    {
    case ComponentType::TRANSFORM:
        return std::unique_ptr<TransformComponentWindow>(new TransformComponentWindow(static_cast<TransformComponent*>(component)));
    case ComponentType::MESH_RENDERER:
        break;
    }
    return nullptr;
}
