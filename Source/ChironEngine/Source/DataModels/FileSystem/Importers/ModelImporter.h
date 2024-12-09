#pragma once
#include "Importer.h"

struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiNode;
struct aiString;
struct aiAnimation;

class CommandList;
class ModelAsset;
class MeshAsset;
class MaterialAsset;

class ModelImporter : public Importer<ModelAsset>
{
public:
    ModelImporter();
    ~ModelImporter() override;

    void Import(const char* filePath, const std::shared_ptr<ModelAsset>& model) override;
    void Load(const char* fileBuffer, const std::shared_ptr<ModelAsset>& resource) override;

private:
    void Save(const std::shared_ptr<ModelAsset>& model) override;
    void ImportNode(const aiScene* scene, const char* filePath, const std::shared_ptr<ModelAsset>& model, const aiNode* node,
        int parentIdx, const Matrix& accTransform);
    std::shared_ptr<MeshAsset> ImportMesh(const aiMesh* mesh, const std::string& fileName, int iteration,
        const std::shared_ptr<CommandList>& copyCommandList);
    std::shared_ptr<MaterialAsset> ImportMaterial(const aiMaterial* material, const std::string& filePath, int iteration);

    void CheckPathMaterial(const char* filePath, const aiString& file, std::string& dataBuffer);
};
