#pragma once

#include "Enums/AssetType.h"
#include "DataModels/FileSystem/UID/UID.h"

class Asset
{
public:

    // ------------- GETTERS ----------------------

    inline const UID GetUID() const;
    inline virtual const std::string& GetName() const;
    inline virtual AssetType GetType() const;

    // ------------- SETTERS ----------------------

    inline void SetName(const std::string& name);

protected:
    Asset(AssetType type);
    virtual ~Asset();

private:
    Asset();

private:
    UID _uid;
    std::string _name;
    AssetType _type;
};

inline const UID Asset::GetUID() const
{
    return _uid;
}

inline const std::string& Asset::GetName() const
{
    return _name;
}

inline AssetType Asset::GetType() const
{
    return _type;
}

inline void Asset::SetName(const std::string& name)
{
    _name = name;
}
