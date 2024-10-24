#pragma once

#include "Asset.h"

class MaterialAsset;
class MeshAsset;

class CommandList;

class ModelAsset : public Asset
{
public:
    ModelAsset();
    ~ModelAsset() override;

    void Draw(const std::shared_ptr<CommandList>& commandList);

    inline void AddMaterial(std::shared_ptr<MaterialAsset>& material);
    inline void AddMesh(std::shared_ptr<MeshAsset>& mesh);

    // ------------- GETTERS ----------------------

    inline const std::vector<std::shared_ptr<MaterialAsset>>& GetMaterials() const;
    inline const std::vector<std::shared_ptr<MeshAsset>>& GetMeshes() const;

private:
    std::vector<std::shared_ptr<MaterialAsset>> _materials;
    std::vector<std::shared_ptr<MeshAsset>> _meshes;

    bool _isTopLeft;
};


inline const std::vector<std::shared_ptr<MaterialAsset>>& ModelAsset::GetMaterials() const
{
    return _materials;
}

inline const std::vector<std::shared_ptr<MeshAsset>>& ModelAsset::GetMeshes() const
{
    return _meshes;
}

inline void ModelAsset::AddMaterial(std::shared_ptr<MaterialAsset>& material)
{
    _materials.push_back(material);
}

inline void ModelAsset::AddMesh(std::shared_ptr<MeshAsset>& mesh)
{
    _meshes.push_back(mesh);
}
