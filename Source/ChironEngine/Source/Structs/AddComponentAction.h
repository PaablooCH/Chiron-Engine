#pragma once

#include "Enums/EnumNames.h"
#include <functional>

DECLARE_ENUM_NAMES(ComponentGroup, NONE,GRAPHICS,SPECIAL)

class GameObject;

struct AddComponentAction
{
    std::string name;
    std::function<void(void)> callback;
    std::function<bool(GameObject*)> condition;
    ComponentGroup componentGroup;

    AddComponentAction(std::string&& name, std::function<void(void)> callback, std::function<bool(GameObject*)> condition,
        ComponentGroup componentGroup) : name(std::move(name)), callback(std::move(callback)), condition(std::move(condition)),
        componentGroup(componentGroup)
    {}

    AddComponentAction(std::string&& name, std::function<void(void)> callback, ComponentGroup componentGroup)
        : AddComponentAction(
            std::move(name),
            std::move(callback),
            [](GameObject*)
            {
                return true;
            },
            componentGroup)
    {}

    friend bool operator<(const AddComponentAction& first, const AddComponentAction& second)
    {
        if (first.componentGroup == second.componentGroup)
        {
            return first.name < second.name;
        }
        return first.componentGroup < second.componentGroup;
    }
};