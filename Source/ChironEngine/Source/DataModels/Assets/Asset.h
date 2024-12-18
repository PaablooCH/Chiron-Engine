#pragma once

#include "Enums/AssetType.h"
#include "DataModels/FileSystem/UID/UID.h"
#include "Defines/FileSystemDefine.h"

class Asset
{
public:
    inline bool IsValid() const;

    bool Load();
    bool Unload();

    // ------------- GETTERS ----------------------

    inline const UID GetUID() const;
    inline virtual const std::string& GetName() const;
    inline virtual AssetType GetType() const;
    inline const std::string& GetAssetPath() const;
    inline const std::string& GetLibraryPath() const;

    // ------------- SETTERS ----------------------

    inline void SetName(const std::string& name);
    inline void SetAssetPath(const std::string& assetPath);
    inline void SetLibraryPath(const std::string& libraryPath);

protected:
    Asset(UID uid, const std::string& assetPath, const std::string& libraryPath, AssetType type);
    Asset(AssetType type);
    virtual ~Asset();

    virtual bool InternalLoad() { return true; };
    virtual bool InternalUnload() { return false; };

private:
    Asset();

private:
    UID _uid;
    std::string _name;
    AssetType _type;

    std::string _assetPath;
    std::string _libraryPath;

    bool _loaded;
};

inline bool Asset::IsValid() const
{
    return _loaded;
}

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

inline const std::string& Asset::GetAssetPath() const
{
    return _assetPath;
}

inline const std::string& Asset::GetLibraryPath() const
{
    return _libraryPath;
}

inline void Asset::SetName(const std::string& name)
{
    _name = name;
}

inline void Asset::SetAssetPath(const std::string& assetPath)
{
    _assetPath = assetPath;
}

inline void Asset::SetLibraryPath(const std::string& libraryPath)
{
    _libraryPath = libraryPath;
}
