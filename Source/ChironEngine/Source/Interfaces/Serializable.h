#pragma once

class Json;

class Serializable
{
public:
    virtual ~Serializable() {};

    virtual void Save(Json& json) = 0;
    virtual void Load(const Json&json) = 0;
};