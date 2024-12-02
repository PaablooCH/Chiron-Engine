#pragma once
#include "../SubWindow.h"
#include "Interfaces/Serializable.h"

class CameraSubWindow : public SubWindow, public Serializable
{
public:
    CameraSubWindow();
    ~CameraSubWindow() override;

    void Save(Json& json);
    void Load(const Json& json);

private:
    void DrawWindowContent(const std::shared_ptr<CommandList>& commandList = nullptr) override;
};
