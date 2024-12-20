#include "Pch.h"
#include "ModelAsset.h"

ModelAsset::ModelAsset(UID uid, const std::string& assetPath, const std::string& libraryPath) :
    Asset(uid, assetPath, libraryPath, AssetType::Model)
{
}

ModelAsset::~ModelAsset()
{
}