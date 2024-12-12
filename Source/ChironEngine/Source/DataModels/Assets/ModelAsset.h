#pragma once

#include "Asset.h"

class MaterialAsset;
class MeshAsset;

struct Node
{
    std::string name;
    Matrix transform;
    int parent;
    std::vector<std::pair<std::shared_ptr<MeshAsset>, std::shared_ptr<MaterialAsset>>> meshMaterial;
};

class ModelAsset : public Asset
{
public:
    ModelAsset();
    ~ModelAsset() override;

    inline void AddNode(Node* node);

    // ------------- GETTERS ----------------------

    inline const std::vector<std::unique_ptr<Node>>& GetNodes() const;
    inline std::string GetAssetPath() const override;
    inline std::string GetLibraryPath() const override;

    // ------------- GETTERS ----------------------

    inline void SetNodes(std::vector<std::unique_ptr<Node>>& nodes);

private:
    std::vector<std::unique_ptr<Node>> _nodes;
};

inline void ModelAsset::AddNode(Node* node)
{
    _nodes.push_back(std::unique_ptr<Node>(node));
}

inline const std::vector<std::unique_ptr<Node>>& ModelAsset::GetNodes() const
{
    return _nodes;
}

inline std::string ModelAsset::GetAssetPath() const
{
    return MODELS_PATH + GetName();
}

inline std::string ModelAsset::GetLibraryPath() const
{
    return MODEL_LIB_PATH + std::to_string(GetUID()) + GENERAL_BINARY_EXTENSION;
}

inline void ModelAsset::SetNodes(std::vector<std::unique_ptr<Node>>& nodes)
{
    _nodes = std::move(nodes);
}
