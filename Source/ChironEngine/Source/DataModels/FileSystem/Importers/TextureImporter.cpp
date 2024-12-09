#include "Pch.h"
#include "TextureImporter.h"

#include "Application.h"

#include "Modules/ModuleID3D12.h"
#include "Modules/ModuleFileSystem.h"

#include "DataModels/Assets/TextureAsset.h"

#include "DataModels/DX12/CommandList/CommandList.h"
#include "DataModels/DX12/Resource/Texture.h"

#include "Defines/FileSystemDefine.h"

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

    bool bInterpretAsSRGB = texture->GetFlag(kSRGB);
    bool bPreserveAlpha = texture->GetFlag(kPreserveAlpha);
    bool bContainsNormals = texture->GetFlag(kNormalMap);
    bool bBumpMap = texture->GetFlag(kBumpToNormal);
    bool bBlockCompress = texture->GetFlag(kDefaultBC);
    bool bUseBestBC = texture->GetFlag(kQualityBC);
    bool bFlipVerticalImage = texture->GetFlag(kFlipVertical);
    bool bFlipHorizontalImage = texture->GetFlag(kFlipHorizontal);

    // Can't be both
    assert(!bInterpretAsSRGB || !bContainsNormals);
    assert(!bPreserveAlpha || !bContainsNormals);

    std::string ext = ModuleFileSystem::GetFileExtension(filePath);

    // ------------- LOAD TEXTURE IMAGE ----------------------

    DirectX::TexMetadata info;
    std::unique_ptr<DirectX::ScratchImage> image(new DirectX::ScratchImage);

    bool isDDS = false;
    bool isHDR = false;
    if (ext == ".dds")
    {
        isDDS = true;
        HRESULT hr = LoadFromDDSFile(path, DirectX::DDS_FLAGS_NONE, &info, *image);
        if (FAILED(hr))
        {
            LOG_ERROR("Could not load texture {} (DDS: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
        if (DirectX::IsCompressed(image->GetMetadata().format))
        {
            std::unique_ptr<DirectX::ScratchImage> dcmprsdImg(new DirectX::ScratchImage);
            hr = DirectX::Decompress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DXGI_FORMAT_UNKNOWN, *dcmprsdImg);
            if (FAILED(hr))
            {
                LOG_ERROR("Error to decompress .dds");
                return;
            }
            image.swap(dcmprsdImg);
        }
    }
    else if (ext == ".tga")
    {
        HRESULT hr = LoadFromTGAFile(path, &info, *image);
        if (FAILED(hr))
        {
            LOG_ERROR("Could not load texture {} (TGA: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
    }
    else if (ext == ".hdr")
    {
        isHDR = true;
        HRESULT hr = LoadFromHDRFile(path, &info, *image);
        if (FAILED(hr))
        {
            LOG_ERROR("Could not load texture {} (HDR: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
    }
    else
    {
        DirectX::WIC_FLAGS wicFlags = DirectX::WIC_FLAGS_NONE;

        HRESULT hr = LoadFromWICFile(path, wicFlags, &info, *image);
        if (FAILED(hr))
        {
            LOG_ERROR("Could not load texture {} (WIC: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
    }

    if (info.width > 16384 || info.height > 16384)
    {
        LOG_ERROR("Texture size ({},{}) too large for feature level 11.0 or later (16384).", info.width, info.height);
        return;
    }

    // ------------- HANDLE ROTATIONS ----------------------

    // rotate 180º
    if (bFlipVerticalImage && bFlipHorizontalImage)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::FlipRotate(image->GetImages()[0], DirectX::TEX_FR_ROTATE180, *timage);
        if (FAILED(hr))
        {
            LOG_WARNING("Could not flip image 180º {} ({}).", filePath, Chiron::Utils::GetErrorMessage(hr));
        }
        else
        {
            image.swap(timage);
            info = image->GetMetadata();
        }
    }
    // rotate vertical
    else if (bFlipVerticalImage)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::FlipRotate(image->GetImages()[0], DirectX::TEX_FR_FLIP_VERTICAL, *timage);
        if (FAILED(hr))
        {
            LOG_WARNING("Could not flip image vertically {} ({}).", filePath, Chiron::Utils::GetErrorMessage(hr));
        }
        else
        {
            image.swap(timage);
            info = image->GetMetadata();
        }
    }
    // rotate horizontal
    else if (bFlipHorizontalImage)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::FlipRotate(image->GetImages()[0], DirectX::TEX_FR_FLIP_HORIZONTAL, *timage);
        if (FAILED(hr))
        {
            LOG_WARNING("Could not flip image horizontally {} ({}).", filePath, Chiron::Utils::GetErrorMessage(hr));
        }
        else
        {
            image.swap(timage);
            info = image->GetMetadata();
        }
    }

    DXGI_FORMAT tformat;
    DXGI_FORMAT cformat;

    if (isHDR)
    {
        tformat = DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
        cformat = bBlockCompress ? DXGI_FORMAT_BC6H_UF16 : DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    }
    else if (bBlockCompress)
    {
        tformat = bInterpretAsSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        if (bUseBestBC)
        {
            cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
        }
        else if (bPreserveAlpha)
        {
            cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
        }
        else
        {
            cformat = bInterpretAsSRGB ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
        }
    }
    else
    {
        cformat = tformat = bInterpretAsSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    if (bBumpMap)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::ComputeNormalMap(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
            DirectX::CNMAP_CHANNEL_LUMINANCE, 10.0f, tformat, *timage);

        if (FAILED(hr))
        {
            LOG_ERROR("Could not compute normal map for {} ({}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
        else
        {
            image.swap(timage);
            info.format = tformat;
        }
    }
    else if (info.format != tformat)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::Convert(image->GetImages(), image->GetImageCount(), image->GetMetadata(),
            tformat, DirectX::TEX_FILTER_DEFAULT, 0.5f, *timage);

        if (FAILED(hr))
        {
            LOG_ERROR("Could not convert {} ({}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
        else
        {
            image.swap(timage);
            info.format = tformat;
        }
    }

    // ------------- HANDLE MIPMAPS ----------------------

    if (info.mipLevels == 1)
    {
        std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

        HRESULT hr = DirectX::GenerateMipMaps(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, *timage);

        if (FAILED(hr))
        {
            LOG_ERROR("Failing generating mimaps for {} (WIC: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
        else
        {
            image.swap(timage);
            info = image->GetMetadata();
        }
    }

    // ------------- HANDLE COMPRESSION ----------------------

    if (bBlockCompress)
    {
        if (info.width % 4 || info.height % 4)
        {
            LOG_WARNING("Texture size ({}, {}) not a multiple of 4 {}, so skipping compress", info.width, info.height, filePath);
        }
        else
        {
            std::unique_ptr<DirectX::ScratchImage> timage(new DirectX::ScratchImage);

            HRESULT hr = DirectX::Compress(image->GetImages(), image->GetImageCount(), image->GetMetadata(), cformat, 
                DirectX::TEX_COMPRESS_DEFAULT, 0.5f, *timage);
            if (FAILED(hr))
            {
                LOG_ERROR("Failing compressing {} (WIC: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            }
            else
            {
                image.swap(timage);
                info = image->GetMetadata();
            }
        }
    }

    // ------------- LOAD INTO RESOURCE ----------------------

    D3D12_RESOURCE_DESC textureDesc = {};
    switch (info.dimension)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT16>(info.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.depth));
        break;
    default:
        throw std::exception("Invalid texture dimension.");
        break;
    }

    std::string textureName = ModuleFileSystem::GetFileName(sFilePath) + ext;
    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(textureDesc, textureName);
    texture->SetTexture(newTexture);

    uint32_t numSubresources = static_cast<uint32_t>(info.mipLevels * info.arraySize);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(numSubresources);
    const DirectX::Image* pImages = image->GetImages();
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

    Save(texture);

    // ------------- SAVE DDS ----------------------

    // Rename file extension to DDS
    std::string dest = TEXTURES_LIB_PATH + ModuleFileSystem::GetFileName(filePath) + ".dds";
    std::wstring wDest = std::wstring(dest.begin(), dest.end());

    // Save DDS
    HRESULT hr = SaveToDDSFile(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DirectX::DDS_FLAGS_NONE, wDest.c_str());
    if (FAILED(hr))
    {
        LOG_ERROR("Could not write texture to file {} {}.", filePath, Chiron::Utils::GetErrorMessage(hr));
    }
}

void TextureImporter::Load(const char* fileBuffer, const std::shared_ptr<TextureAsset>& texture)
{
    // ------------- META ----------------------

    std::string metaPath = TEXTURES_PATH + texture->GetName() + META_EXT;
    rapidjson::Document doc;
    Json meta = Json(doc);
    ModuleFileSystem::LoadJson(metaPath.c_str(), meta);
    texture->AddConversionFlags(meta["texConversionFlags"]);

    // ------------- LOAD DDS ----------------------

    std::string filePath = TEXTURES_LIB_PATH + ModuleFileSystem::GetFileName(texture->GetName()) + ".dds";
    std::wstring wFilePath = std::wstring(filePath.begin(), filePath.end());
    const wchar_t* path = wFilePath.c_str();

    DirectX::TexMetadata info;
    std::unique_ptr<DirectX::ScratchImage> image(new DirectX::ScratchImage);
    HRESULT hr = LoadFromDDSFile(path, DirectX::DDS_FLAGS_NONE, &info, *image);
    if (FAILED(hr))
    {
        LOG_ERROR("Could not load texture {} (DDS: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
        return;
    }

    // ------------- LOAD INTO RESOURCE ----------------------

    D3D12_RESOURCE_DESC textureDesc = {};
    switch (info.dimension)
    {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT16>(info.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.depth));
        break;
    default:
        throw std::exception("Invalid texture dimension.");
        break;
    }

    std::string textureName = texture->GetName();
    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(textureDesc, textureName);
    texture->SetTexture(newTexture);

    uint32_t numSubresources = static_cast<uint32_t>(info.mipLevels * info.arraySize);
    std::vector<D3D12_SUBRESOURCE_DATA> subresources(numSubresources);
    const DirectX::Image* pImages = image->GetImages();
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
}

void TextureImporter::Save(const std::shared_ptr<TextureAsset>& texture)
{
    // ------------- META ----------------------

    std::string metaPath = TEXTURES_PATH + texture->GetName() + META_EXT;
    rapidjson::Document doc;
    Json meta = Json(doc);
    ModuleFileSystem::LoadJson(metaPath.c_str(), meta);
    meta["texConversionFlags"] = texture->GetFlags();

    rapidjson::StringBuffer buffer = meta.ToBuffer();
    ModuleFileSystem::SaveFile(metaPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());
}