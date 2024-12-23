#include "Pch.h"
#include "MeshRendererComponent.h"

#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/Components/TransformComponent.h"

#include "DataModels/GameObject/GameObject.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocator.h"
#include "DataModels/DX12/DescriptorAllocator/DescriptorAllocatorPage.h"
#include "DataModels/DX12/Resource/IndexBuffer.h"
#include "DataModels/DX12/Resource/VertexBuffer.h"
#include "DataModels/DX12/Resource/Texture.h"

#include "Structs/ModelAttributes.h"

MeshRendererComponent::MeshRendererComponent(GameObject* owner) : Component(ComponentType::MESH_RENDERER, owner), _material(nullptr),
_mesh(nullptr)
{
}

MeshRendererComponent::MeshRendererComponent(const MeshRendererComponent& copy) : Component(copy)
{
}

MeshRendererComponent::~MeshRendererComponent()
{
}

void MeshRendererComponent::Render(const std::shared_ptr<CommandList>& commandList) const
{
    commandList->SetVertexBuffers(0, 1, &_mesh->GetVertexBuffer()->GetVertexBufferView());
    commandList->SetIndexBuffer(&_mesh->GetIndexBuffer()->GetIndexBufferView());

    auto texture = _material->GetBaseTexture();
    Matrix model = _owner->GetInternalComponent<TransformComponent>()->GetGlobalMatrix();

    ModelAttributes modelAttributes;
    modelAttributes.model = model.Transpose();
    modelAttributes.uvCorrector = texture->GetConfigFlags() | isBottomLeft;
    CHIRON_TODO("CorrectUV for each texture");

    commandList->SetGraphicsRoot32BitConstants(1, sizeof(ModelAttributes) / 4, &modelAttributes);

    commandList->TransitionBarrier(texture->GetTexture().get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    // set the descriptor heap
    ID3D12DescriptorHeap* descriptorHeaps[] = {
        texture->GetTexture()->GetShaderResourceView().GetDescriptorAllocatorPage()->GetDescriptorHeap().Get()
    };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootDescriptorTable(2, texture->GetTexture()->GetShaderResourceView().GetGPUDescriptorHandle());

    commandList->DrawIndexed(static_cast<UINT>(_mesh->GetIndexBuffer()->GetNumIndices()));
}

void MeshRendererComponent::InternalSave(Field& meta)
{
}

void MeshRendererComponent::InternalLoad(const Field& meta)
{
}