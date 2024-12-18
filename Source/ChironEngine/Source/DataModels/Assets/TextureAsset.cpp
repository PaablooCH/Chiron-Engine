#include "Pch.h"
#include "TextureAsset.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/Texture.h"

TextureAsset::TextureAsset(TextureType type) : Asset(AssetType::Texture), _type(type),
_texConversionFlags(0), _texConfigFlags(isBottomLeft)
{
    if (_type == TextureType::ALBEDO)
    {
        _texConversionFlags |= kSRGB;
        _texConversionFlags |= kPreserveAlpha;
    }
    else if (_type == TextureType::NORMAL_MAP)
    {
        _texConversionFlags |= kNormalMap;
    }
    else if (_type == TextureType::HDR)
    {
        _texConversionFlags |= kQualityBC;
    }
    _texConversionFlags |= kDefaultBC;
}

TextureAsset::TextureAsset(TextureType type, UID uid, const std::string& assetPath, const std::string& libraryPath) :
    Asset(uid, assetPath, libraryPath, AssetType::Texture), _type(type), _texConversionFlags(0), _texConfigFlags(isBottomLeft)
{
    if (_type == TextureType::ALBEDO)
    {
        _texConversionFlags |= kSRGB;
        _texConversionFlags |= kPreserveAlpha;
    }
    else if (_type == TextureType::NORMAL_MAP)
    {
        _texConversionFlags |= kNormalMap;
    }
    else if (_type == TextureType::HDR)
    {
        _texConversionFlags |= kQualityBC;
    }
    _texConversionFlags |= kDefaultBC;
}

TextureAsset::TextureAsset(UID uid, const std::string& assetPath, const std::string& libraryPath) :
    Asset(uid, assetPath, libraryPath, AssetType::Texture), _type(TextureType::ALBEDO), _texConversionFlags(0), _texConfigFlags(isBottomLeft)
{
    _texConversionFlags |= kSRGB;
    _texConversionFlags |= kPreserveAlpha;
    _texConversionFlags |= kDefaultBC;
}

TextureAsset::~TextureAsset()
{
}

void TextureAsset::AddConversionFlags(unsigned int flags)
{
    bool bInterpretAsSRGB = (flags & kSRGB) != 0;
    bool bPreserveAlpha = (flags & kPreserveAlpha) != 0;
    bool bContainsNormals = (flags & kNormalMap) != 0;

    assert(!bInterpretAsSRGB || !bContainsNormals);
    assert(!bPreserveAlpha || !bContainsNormals);

    bool ownInterpretAsSRGB = GetConversionFlag(kSRGB);
    bool ownContainsNormals = GetConversionFlag(kNormalMap);
    bool ownPreserveAlpha = GetConversionFlag(kPreserveAlpha);

    assert(!ownInterpretAsSRGB || !bContainsNormals);
    assert(!ownPreserveAlpha || !bContainsNormals);
    assert(!bInterpretAsSRGB || !ownContainsNormals);
    assert(!bPreserveAlpha || !ownContainsNormals);

    _texConversionFlags |= flags;
}

void TextureAsset::RemoveConversionFlags(unsigned int flags)
{
    _texConversionFlags &= ~flags;
}

std::string TextureAsset::GetLibraryDDSPath() const
{
    return TEXTURES_LIB_PATH + std::to_string(GetUID()) + DDS_EXT;
}

void TextureAsset::SetTexture(std::shared_ptr<Texture>& newTexture)
{
    _texture = newTexture;
    SetName(_texture->GetName());
}

bool TextureAsset::InternalLoad()
{
    bool result = true;
    if (_texture)
    {
        result = result && _texture->Load();
    }
    return result;
}

bool TextureAsset::InternalUnload()
{
    if (_texture)
    {
        return _texture->Unload();
    }
    return false;
}
