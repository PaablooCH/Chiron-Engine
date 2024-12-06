#include "Pch.h"
#include "TextureImporter.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleFileSystem.h"

#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/Texture.h"

#include "DirectXTex.h"

TextureImporter::TextureImporter()
{
}

TextureImporter::~TextureImporter()
{
}

void TextureImporter::Import(const char* filePath, const std::shared_ptr<TextureAsset>& texture)
{
    std::string sFilePath(filePath);
    std::wstring wFilePath = std::wstring(sFilePath.begin(), sFilePath.end());
    const wchar_t* path = wFilePath.c_str();

    DirectX::TexMetadata md{};
    DirectX::ScratchImage* imgResult = nullptr;
    DirectX::ScratchImage img, flippedImg, dcmprsdImg;

    std::string extension = ModuleFileSystem::GetFileExtension(filePath);
    if (extension == ".dds")
    {
        if (FAILED(DirectX::LoadFromDDSFile(path, DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_RGB, &md, img)))
        {
            LOG_ERROR("Error to convert .dds");
        }

        if (FAILED(DirectX::Decompress(img.GetImages(), img.GetImageCount(), md, DXGI_FORMAT_UNKNOWN, dcmprsdImg)))
        {
            LOG_ERROR("Error to decompress .dds");
        }
        img = std::move(dcmprsdImg);
    }
    else if (extension == ".hdr")
    {
        if (FAILED(DirectX::LoadFromHDRFile(path, &md, img)))
        {
            LOG_ERROR("Error to convert .hdr.");
        }
    }
    else if (extension == ".tga")
    {
        if (FAILED(DirectX::LoadFromTGAFile(path, &md, img)))
        {
            LOG_ERROR("Error to convert .tga.");
        }
    }
    else
    {
        if (FAILED(DirectX::LoadFromWICFile(path, DirectX::WIC_FLAGS::WIC_FLAGS_NONE, &md, img)))
        {
            LOG_ERROR("No suitable conversion for texture extension: {}", extension);
        }
    }

    /*if (options.flipVertical && options.flipHorizontal)
    {
        if (!FAILED(DirectX::FlipRotate(img.GetImages(), img.GetImageCount(), img.GetMetadata(),
            DirectX::TEX_FR_FLAGS::TEX_FR_ROTATE180, flippedImg)))
        {
            img = std::move(flippedImg);
        }
    }
    else if (options.flipVertical)
    {
        if (!FAILED(DirectX::FlipRotate(img.GetImages(), img.GetImageCount(), img.GetMetadata(),
            DirectX::TEX_FR_FLAGS::TEX_FR_FLIP_VERTICAL, flippedImg)))
        {
            img = std::move(flippedImg);
        }
    }
    else if (options.flipHorizontal)
    {
        if (!FAILED(DirectX::FlipRotate(img.GetImages(), img.GetImageCount(), img.GetMetadata(),
            DirectX::TEX_FR_FLAGS::TEX_FR_FLIP_HORIZONTAL, flippedImg)))
        {
            img = std::move(flippedImg);
        }
    }*/

    if (texture->GetTextureType() == TextureType::ALBEDO)
    {
        md.format = DirectX::MakeSRGB(md.format);
        img.OverrideFormat(md.format);
    }

    UINT16 mipLevels = md.mipLevels <= 1 ? static_cast<UINT16>(CalculateMipLevels(static_cast<int>(md.width), static_cast<int>(md.height)))
        : static_cast<UINT16>(md.mipLevels);
    D3D12_RESOURCE_DESC textureDesc = {};
    switch (md.dimension)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            md.format,
            static_cast<UINT64>(md.width),
            static_cast<UINT16>(md.arraySize),
            static_cast<UINT16>(md.mipLevels));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            md.format,
            static_cast<UINT64>(md.width),
            static_cast<UINT>(md.height),
            static_cast<UINT16>(md.arraySize),
            mipLevels);
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            md.format,
            static_cast<UINT64>(md.width),
            static_cast<UINT>(md.height),
            static_cast<UINT16>(md.depth),
            static_cast<UINT16>(md.mipLevels));
        break;
    default:
        throw std::exception("Invalid texture dimension.");
        break;
    }

    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(textureDesc, sFilePath);
    texture->SetTexture(newTexture);

    imgResult = &img;
    DirectX::ScratchImage mipChain;
    mipLevels = newTexture->GetResource()->GetDesc().MipLevels;
    if (1 < mipLevels)
    {
        DirectX::GenerateMipMaps(img.GetImages(), img.GetImageCount(), md, DirectX::TEX_FILTER_DEFAULT, mipLevels, mipChain);
        imgResult = &mipChain;
    }

    uint32_t numSubresources = static_cast<uint32_t>(imgResult->GetImageCount());
}
void TextureImporter::Load(const char* fileBuffer, std::shared_ptr<TextureAsset>& texture)
{
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(numSubresources);
    const DirectX::Image* pImages = imgResult->GetImages();
    for (uint32_t i = 0; i < numSubresources; ++i)
    {
        auto& subresource = subresources[i];
        subresource.pData = pImages[i].pixels;
        subresource.RowPitch = pImages[i].rowPitch;
        subresource.SlicePitch = pImages[i].slicePitch;
    }

    auto d3d12 = App->GetModule<ModuleID3D12>();

    auto commandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_COPY);
    commandList->UpdateBufferResource(texture->GetTexture().get(), 0, numSubresources, subresources.data());

    uint64_t initFenceValue = d3d12->ExecuteCommandList(commandList);
    d3d12->WaitForFenceValue(D3D12_COMMAND_LIST_TYPE_COPY, initFenceValue);

    CHIRON_TODO("delete????");
    commandList = d3d12->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
    commandList->TransitionBarrier(texture->GetTexture().get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    initFenceValue = d3d12->ExecuteCommandList(commandList);
    d3d12->WaitForFenceValue(D3D12_COMMAND_LIST_TYPE_DIRECT, initFenceValue);

    imgResult->Release();
}

void TextureImporter::Save(const std::shared_ptr<TextureAsset>& texture)
{
    int maxDimension = std::max(width, height);
    return static_cast<int>(std::log2(maxDimension)) + 1;
}