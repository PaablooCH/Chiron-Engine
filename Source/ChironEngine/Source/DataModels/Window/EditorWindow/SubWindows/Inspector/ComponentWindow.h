#pragma once
#include "../SubWindow.h"

#include "DataModels/FileSystem/UID.h"

class Component;

class ComponentWindow : public SubWindow
{
public:
    virtual ~ComponentWindow() override {};

    void Draw(const std::shared_ptr<CommandList>& commandList = nullptr) override;

protected:
    ComponentWindow(std::string name, Component* component, bool disableEnable = false, bool disableRemove = false);

    bool CollapsingHeader();

protected:
    Component* _component;

private:
    void DrawEnable();
    void DrawRemoveComponent();

private:
    UID _windowUID;
    bool _disableEnable;
    bool _disableRemove;
};
