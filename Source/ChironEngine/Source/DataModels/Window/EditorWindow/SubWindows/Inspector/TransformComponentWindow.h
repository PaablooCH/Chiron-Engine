#pragma once
#include "ComponentWindow.h"

class TransformComponent;
class ComponentWindowFactory;

class TransformComponentWindow : public ComponentWindow
{
public:
    ~TransformComponentWindow() override;

private:
    friend class ComponentWindowFactory;

    TransformComponentWindow(TransformComponent* component);

    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList = nullptr) override;

    void DrawTransforms();

private:
    float _dragSpeed;
};
