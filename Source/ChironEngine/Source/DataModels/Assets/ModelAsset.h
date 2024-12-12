#pragma once

#include "Asset.h"

class MaterialAsset;
class MeshAsset;


class ModelAsset : public Asset
{
public:
    ModelAsset();
    ~ModelAsset() override;


    inline void AddMaterial(std::shared_ptr<MaterialAsset>& material);
    inline void AddMesh(std::shared_ptr<MeshAsset>& mesh);
    inline std::string GetAssetPath() const override;
    inline std::string GetLibraryPath() const override;

    // ------------- GETTERS ----------------------

    inline const std::vector<std::shared_ptr<MaterialAsset>>& GetMaterials() const;
    inline const std::vector<std::shared_ptr<MeshAsset>>& GetMeshes() const;

private:
    std::vector<std::shared_ptr<MaterialAsset>> _materials;
    std::vector<std::shared_ptr<MeshAsset>> _meshes;

};

inline const std::vector<std::shared_ptr<MaterialAsset>>& ModelAsset::GetMaterials() const
{
    return _materials;
}

inline std::string ModelAsset::GetAssetPath() const
{
    return MODELS_PATH + GetName();
}

inline std::string ModelAsset::GetLibraryPath() const
{
    return MODEL_LIB_PATH + std::to_string(GetUID()) + GENERAL_BINARY_EXTENSION;
}

inline void ModelAsset::AddMesh(std::shared_ptr<MeshAsset>& mesh)
{
    _meshes.push_back(mesh);
}
