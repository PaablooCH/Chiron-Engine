#include "Pch.h"
#include "TextureAsset.h"

#include "DataModels/DX12/Resource/Texture.h"

TextureAsset::TextureAsset(TextureType type) : Asset(AssetType::Texture), _type(type)
{
}

TextureAsset::~TextureAsset()
{
}

const std::string& TextureAsset::GetName() const
{
    return _texture->GetName();
}