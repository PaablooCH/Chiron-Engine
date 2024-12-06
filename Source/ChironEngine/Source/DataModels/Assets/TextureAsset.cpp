#include "Pch.h"
#include "TextureAsset.h"

#include "DataModels/DX12/Resource/Texture.h"

TextureAsset::TextureAsset(TextureType type) : Asset(AssetType::Texture), _type(type), _texConversionFlags(0)
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

    bool ownInterpretAsSRGB = GetFlag(kSRGB);
    bool ownContainsNormals = GetFlag(kNormalMap);
    bool ownPreserveAlpha = GetFlag(kPreserveAlpha);

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

void TextureAsset::SetTexture(std::shared_ptr<Texture>& newTexture)
{
    _texture = newTexture;
    SetName(_texture->GetName());
}