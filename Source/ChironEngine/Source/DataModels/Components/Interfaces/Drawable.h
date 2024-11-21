#pragma once

class Drawable
{
public:
    virtual ~Drawable() {};

    virtual void Draw() = 0;
};