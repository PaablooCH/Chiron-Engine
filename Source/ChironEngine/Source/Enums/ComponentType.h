#pragma once

#include "EnumNames.h"

class TransformComponent;

DECLARE_ENUM_NAMES(ComponentType, TRANSFORM, MESH_RENDERER)

template<typename T>
struct ComponentToEnum
{
};

template<>
struct ComponentToEnum<TransformComponent>
{
    const static ComponentType value = ComponentType::TRANSFORM;
};