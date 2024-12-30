#pragma once
#include "Component.h"
#include "Interfaces/Renderable.h"

class MaterialAsset;
class MeshAsset;

class MeshRendererComponent : public Component, public Renderable
{
public:
    MeshRendererComponent(GameObject* owner);
    MeshRendererComponent(const MeshRendererComponent& copy);
    ~MeshRendererComponent() override;

    void Render(const std::shared_ptr<CommandList>& commandList) const override;

    // ------------- GETTERS ----------------------

    inline MaterialAsset* GetMaterial() const;
    inline MeshAsset* GetMesh() const;

    // ------------- SETTERS ----------------------

    inline void SetMaterial(const std::shared_ptr<MaterialAsset>& material);
    inline void SetMesh(const std::shared_ptr<MeshAsset>& mesh);

private:
    void InternalSave(Field& meta) override;
    void InternalLoad(const Field& meta) override;

private:
    std::shared_ptr<MaterialAsset> _material;
    std::shared_ptr<MeshAsset> _mesh;
};

inline MaterialAsset* MeshRendererComponent::GetMaterial() const
{
    return _material.get();
}

inline MeshAsset* MeshRendererComponent::GetMesh() const
{
    return _mesh.get();
}

inline void MeshRendererComponent::SetMaterial(const std::shared_ptr<MaterialAsset>& material)
{
    _material = material;
}

inline void MeshRendererComponent::SetMesh(const std::shared_ptr<MeshAsset>& mesh)
{
    _mesh = mesh;
}