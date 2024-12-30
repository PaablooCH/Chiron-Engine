#pragma once

#include <memory>

class CommandList;

class Renderable
{
public:
    virtual ~Renderable() {};

    virtual void Render(const std::shared_ptr<CommandList>& commandList) const = 0;
};