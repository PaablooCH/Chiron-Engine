#pragma once
#include "EnumNames.h"

DECLARE_ENUM_NAMES(AssetType, Material,Mesh,Model,Texture)

namespace AssetTypeUtils
{
    inline std::string GetFolder(AssetType enumTmp)
    {
        switch (enumTmp)
        {
        case AssetType::Material:
            return "Materials";
        case AssetType::Mesh:
            return "Meshes";
        case AssetType::Model:
            return "Models";
        case AssetType::Texture:
            return "Textures";
        case AssetType::UNKNOWN:
            LOG_ERROR("Try to get folder UKNOWN");
            break;
        }
        return "";
    }
}