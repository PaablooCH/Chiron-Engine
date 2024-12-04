#pragma once
#include "Module.h"

class TextureImporter;
class ModelImporter;
class Asset;


class ModuleResources : public Module
{
public:
    ModuleResources();
    ~ModuleResources();

    bool Init() override;
    bool Start() override;
    bool CleanUp() override;

    void Import(const char* filePath, const std::shared_ptr<Asset>& asset);

private:
    void CreateAssetsAndLibraryFolders();

private:
    std::unique_ptr<TextureImporter> _textureImporter;
    std::unique_ptr<ModelImporter> _modelImporter;
};

