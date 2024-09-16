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

    void Draw(std::shared_ptr<CommandList> commandList);

    // ------------- GETTERS ----------------------

    inline std::wstring GetName() const override;

    // ------------- GETTERS ----------------------
	
    inline void AddMaterial(std::shared_ptr<MaterialAsset>& material);
    inline void AddMesh(std::shared_ptr<MeshAsset>& mesh);

private:
    std::vector<std::shared_ptr<MaterialAsset>> _material;
    std::vector<std::shared_ptr<MeshAsset>> _mesh;
};

inline std::wstring ModelAsset::GetName() const
{
    CHIRON_TODO("TODO");
    return L"";
}

inline void ModelAsset::AddMaterial(std::shared_ptr<MaterialAsset>& material)
{
    _material.push_back(material);
}

inline void ModelAsset::AddMesh(std::shared_ptr<MeshAsset>& mesh)
{
    _mesh.push_back(mesh);
}
