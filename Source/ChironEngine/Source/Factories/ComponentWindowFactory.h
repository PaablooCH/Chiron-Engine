#pragma once

class Component;
class ComponentWindow;

class ComponentWindowFactory
{
public:
    static std::unique_ptr<ComponentWindow> CreateComponentWindow(Component* component);
    
private:
    ComponentWindowFactory() = delete;
    ~ComponentWindowFactory() = delete;
};

