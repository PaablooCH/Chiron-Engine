#pragma once
#include "Importer.h"

class MeshAsset;

class MeshImporter : public Importer<MeshAsset>
{
public:
    MeshImporter();
    ~MeshImporter() override;

    void Import(const char* filePath, const std::shared_ptr<MeshAsset>& mesh) override;
    void Load(const char* fileBuffer, std::shared_ptr<MeshAsset>& mesh) override;

private:
    void Save(const std::shared_ptr<MeshAsset>& mesh) override;
};

