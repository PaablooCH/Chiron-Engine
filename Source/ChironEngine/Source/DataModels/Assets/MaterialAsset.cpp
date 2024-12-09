#include "Pch.h"
#include "MaterialAsset.h"

MaterialAsset::MaterialAsset() : Asset(AssetType::Material), 
_baseColor(0.2752307057380676, 0.31043916940689087, 0.3382353186607361, 1.0), 
_specularColor(0.5, 0.30000001192092896, 0.5, 1.0), _options(usePBR | isOpaque)
{
}

MaterialAsset::~MaterialAsset()
{
}