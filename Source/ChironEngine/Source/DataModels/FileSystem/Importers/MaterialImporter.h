#pragma once
#include "Importer.h"

class MaterialAsset;

class MaterialImporter : public Importer<MaterialAsset>
{
public:
    MaterialImporter();
    ~MaterialImporter() override;

    void Import(const char* filePath, const std::shared_ptr<MaterialAsset>& material) override;
    void Load(const char* fileBuffer, std::shared_ptr<MaterialAsset>& material) override;

private:
    void Save(const std::shared_ptr<MaterialAsset>& material) override;
};

