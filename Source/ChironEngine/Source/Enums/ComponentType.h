#pragma once

class TransformComponent;

enum class ComponentType
{
    TRANSFORM,
    MESH_RENDERER
};

template<typename T>
struct ComponentToEnum
{
};

template<>
struct ComponentToEnum<TransformComponent>
{
    const static ComponentType value = ComponentType::TRANSFORM;
};