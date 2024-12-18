#pragma once
#include "Importer.h"

class TextureAsset;

class TextureImporter : public Importer<TextureAsset>
{
public:
    TextureImporter();
    ~TextureImporter() override;

    void Import(const char* filePath, const std::shared_ptr<TextureAsset>& texture) override;
    void Load(const char* libraryPath, const std::shared_ptr<TextureAsset>& texture) override;

private:
    void Save(const std::shared_ptr<TextureAsset>& texture) override;
};