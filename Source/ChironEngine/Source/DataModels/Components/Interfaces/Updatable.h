#pragma once

class Updatable
{
public:
    virtual ~Updatable() {};

    virtual void PreUpdate(bool gameRunning) {};
    virtual void Update(bool gameRunning) = 0;
    virtual void PostUpdate(bool gameRunning) {};
};