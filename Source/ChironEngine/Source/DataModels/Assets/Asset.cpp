#include "Pch.h"
#include "Asset.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

Asset::Asset(UID uid, const std::string& assetPath, const std::string& libraryPath, AssetType type) : _uid(uid), _type(type),
_assetPath(assetPath), _libraryPath(libraryPath), _loaded(false)
{
}

Asset::Asset(AssetType type) : _uid(Chiron::UIDGenerator::GenerateUID()), 
_type(type), _assetPath(""), _libraryPath(""), _loaded(false)
{
}

Asset::~Asset()
{
}

bool Asset::Load()
{
    if (!_loaded)
    {
        _loaded = InternalLoad();
    }
    return _loaded;
}

bool Asset::Unload()
{
    if (_loaded)
    {
        _loaded = InternalUnload();
    }
    return _loaded;
}
