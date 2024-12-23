#include "Pch.h"
#include "MaterialAsset.h"

#include "TextureAsset.h"

MaterialAsset::MaterialAsset() : Asset(AssetType::Model),
_baseColor(0.2752307057380676, 0.31043916940689087, 0.3382353186607361, 1.0),
_specularColor(0.5, 0.30000001192092896, 0.5, 1.0), _options(usePBR | isOpaque)
{
}

MaterialAsset::MaterialAsset(UID uid, const std::string& assetPath, const std::string& libraryPath) :
    Asset(uid, assetPath, libraryPath, AssetType::Material),
    _baseColor(0.2752307057380676, 0.31043916940689087, 0.3382353186607361, 1.0),
    _specularColor(0.5, 0.30000001192092896, 0.5, 1.0), _options(usePBR | isOpaque)
{
}

MaterialAsset::~MaterialAsset()
{
}

TextureAsset* MaterialAsset::GetBaseTexture()
{
    if (_baseTexture)
    {
        _baseTexture->Load();
    }
    return _baseTexture.get();
}

TextureAsset* MaterialAsset::GetNormalMap()
{
    if (_normalMap)
    {
        _normalMap->Load();
    }
    return _normalMap.get();
}

TextureAsset* MaterialAsset::GetPropertyTexture()
{
    if (_propertyTexture)
    {
        _propertyTexture->Load();
    }
    return _propertyTexture.get();
}

TextureAsset* MaterialAsset::GetEmissiveTexture()
{
    if (_emissiveTexture)
    {
        _emissiveTexture->Load();
    }
    return _emissiveTexture.get();
}

TextureAsset* MaterialAsset::GetAmbientOcclusion()
{
    if (_ambientOcclusion)
    {
        _ambientOcclusion->Load();
    }
    return _ambientOcclusion.get();
}

void MaterialAsset::SetBaseTexture(const std::shared_ptr<TextureAsset>& diffuse)
{
    _baseTexture = diffuse;
    _baseTexture->SetTextureType(TextureType::ALBEDO);
}

void MaterialAsset::SetNormalMap(const std::shared_ptr<TextureAsset>& normal)
{
    _normalMap = normal;
    _normalMap->SetTextureType(TextureType::NORMAL_MAP);
}

void MaterialAsset::SetPropertyTexture(const std::shared_ptr<TextureAsset>& metalness)
{
    _propertyTexture = metalness;
    _propertyTexture->SetTextureType(TextureType::METALLIC);
}

void MaterialAsset::SetEmissiveTexture(const std::shared_ptr<TextureAsset>& emissive)
{
    _emissiveTexture = emissive;
    _emissiveTexture->SetTextureType(TextureType::EMISSIVE);
}

void MaterialAsset::SetAmbientOcclusion(const std::shared_ptr<TextureAsset>& occlusion)
{
    _ambientOcclusion = occlusion;
    _ambientOcclusion->SetTextureType(TextureType::OCCLUSION);
}

bool MaterialAsset::InternalLoad()
{
    bool result = true;

    if (_baseTexture)
    {
        result = result && _baseTexture->Load();
    }
    if (_normalMap)
    {
        result = result && _normalMap->Load();
    }
    if (_propertyTexture)
    {
        result = result && _propertyTexture->Load();
    }
    if (_emissiveTexture)
    {
        result = result && _emissiveTexture->Load();
    }
    if (_ambientOcclusion)
    {
        result = result && _ambientOcclusion->Load();
    }
    return result;
}

bool MaterialAsset::InternalUnload()
{
    bool result = false;
    if (_baseTexture)
    {
        result = result || _baseTexture->Unload();
    }
    if (_normalMap)
    {
        result = result || _normalMap->Unload();
    }
    if (_propertyTexture)
    {
        result = result || _propertyTexture->Unload();
    }
    if (_emissiveTexture)
    {
        result = result || _emissiveTexture->Unload();
    }
    if (_ambientOcclusion)
    {
        result = result || _ambientOcclusion->Unload();
    }
    return result;
}