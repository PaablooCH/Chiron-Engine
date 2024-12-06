#include "Pch.h"
#include "Asset.h"

#include "DataModels/FileSystem/UID/UIDGenerator.h"

Asset::Asset(AssetType type) : _uid(Chiron::UIDGenerator::GenerateUID()), _type(type)
{
}

Asset::~Asset()
{
    CHIRON_TODO("Guardar los binarios cuando se destruyan?");
}