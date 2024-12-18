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

    bool bInterpretAsSRGB = texture->GetConversionFlag(kSRGB);
    bool bPreserveAlpha = texture->GetConversionFlag(kPreserveAlpha);
    bool bContainsNormals = texture->GetConversionFlag(kNormalMap);
    bool bBumpMap = texture->GetConversionFlag(kBumpToNormal);
    bool bBlockCompress = texture->GetConversionFlag(kDefaultBC);
    bool bUseBestBC = texture->GetConversionFlag(kQualityBC);
    bool bFlipVerticalImage = texture->GetConversionFlag(kFlipVertical);
    bool bFlipHorizontalImage = texture->GetConversionFlag(kFlipHorizontal);

    // Can't be both
    assert(!bInterpretAsSRGB || !bContainsNormals);
    assert(!bPreserveAlpha || !bContainsNormals);

    std::string ext = ModuleFileSystem::GetFileExtension(filePath);

    // ------------- LOAD TEXTURE IMAGE ----------------------

    DirectX::TexMetadata info;
    std::unique_ptr<DirectX::ScratchImage> image(new DirectX::ScratchImage);

    bool isDDS = false;
    bool isHDR = false;
    if (ext == DDS_EXT)
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
    else if (ext == TGA_EXT)
    {
        HRESULT hr = LoadFromTGAFile(path, &info, *image);
        if (FAILED(hr))
        {
            LOG_ERROR("Could not load texture {} (TGA: {}).", filePath, Chiron::Utils::GetErrorMessage(hr));
            return;
        }
    }
    else if (ext == HDR_EXT)
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
            static_cast<UINT16>(info.arraySize),
            static_cast<UINT16>(info.mipLevels));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.arraySize),
            static_cast<UINT16>(info.mipLevels));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            info.format,
            static_cast<UINT64>(info.width),
            static_cast<UINT>(info.height),
            static_cast<UINT16>(info.depth),
            static_cast<UINT16>(info.mipLevels));
        break;
    default:
        throw std::exception("Invalid texture dimension.");
        break;
    }

    std::string textureName = ModuleFileSystem::GetFileName(sFilePath) + ext;
    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(textureDesc, textureName);
    texture->SetTexture(newTexture);

    uint32_t imageSize = static_cast<uint32_t>(info.mipLevels * info.arraySize);
    std::vector<MyImage> images(imageSize);
    const DirectX::Image* pImages = image->GetImages();
    for (uint32_t i = 0; i < imageSize; ++i)
    {
        auto& subresource = images[i];
        subresource.rowPitch = pImages[i].rowPitch;
        subresource.slicePitch = pImages[i].slicePitch;

        size_t dataSize = pImages[i].slicePitch;
        subresource.pixels.resize(dataSize);
        memcpy(subresource.pixels.data(), pImages[i].pixels, dataSize);
    }
    texture->SetImages(images);

    Save(texture);

    // ------------- SAVE DDS ----------------------

    // Rename file extension to DDS
    std::string dest = texture->GetLibraryDDSPath();
    std::wstring wDest = std::wstring(dest.begin(), dest.end());

    // Save DDS
    HRESULT hr = SaveToDDSFile(image->GetImages(), image->GetImageCount(), image->GetMetadata(), DirectX::DDS_FLAGS_NONE, wDest.c_str());
    if (FAILED(hr))
    {
        LOG_ERROR("Could not write texture to file {} {}.", filePath, Chiron::Utils::GetErrorMessage(hr));
    }
}

void TextureImporter::Load(const char* libraryPath, const std::shared_ptr<TextureAsset>& texture)
{
    if (!ModuleFileSystem::ExistsFile(libraryPath))
    {
        // ------------- META ----------------------

        std::string metaPath = texture->GetAssetPath() + META_EXT;
        rapidjson::Document doc;
        Json meta = Json(doc);
        ModuleFileSystem::LoadJson(metaPath.c_str(), meta);
        texture->AddConfigFlags(meta["texConfigFlags"]);
        texture->AddConversionFlags(meta["texConversionFlags"]);

        // ------------- REIMPORT FILE ----------------------

        std::string assetPath = meta["assetPath"];
        Import(assetPath.c_str(), texture);

        return;
    }
    
    char* fileBuffer;
    ModuleFileSystem::LoadFile(libraryPath, fileBuffer);
    char* originalFileBuffer = fileBuffer;

    // ------------- BINARY ----------------------

    unsigned int header[2];
    memcpy(header, fileBuffer, sizeof(header));
    fileBuffer += sizeof(header);

    texture->SetName(std::string(fileBuffer, header[0]));
    fileBuffer += header[0];

    UINT configFlag = header[1];
    texture->AddConfigFlags(configFlag);

    // ------------- LOAD DDS ----------------------

    std::string ddsPath = texture->GetLibraryDDSPath();
    std::wstring wFilePath = std::wstring(ddsPath.begin(), ddsPath.end());
    const wchar_t* path = wFilePath.c_str();

    DirectX::TexMetadata info;
    std::unique_ptr<DirectX::ScratchImage> image(new DirectX::ScratchImage);
    HRESULT hr = LoadFromDDSFile(path, DirectX::DDS_FLAGS_NONE, &info, *image);
    if (FAILED(hr))
    {
        LOG_ERROR("Could not load texture {} (DDS: {}).", libraryPath, Chiron::Utils::GetErrorMessage(hr));
        return;
    }
    DirectX::IsCompressed(image->GetMetadata().format);

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

    std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(textureDesc, texture->GetName());
    texture->SetTexture(newTexture);

    uint32_t numSubresources = static_cast<uint32_t>(info.mipLevels * info.arraySize);
    std::vector<MyImage> images(numSubresources);
    const DirectX::Image* pImages = image->GetImages();
    for (uint32_t i = 0; i < numSubresources; ++i)
    {
        auto& subresource = images[i];
        subresource.rowPitch = pImages[i].rowPitch;
        subresource.slicePitch = pImages[i].slicePitch;

        size_t dataSize = pImages[i].slicePitch;
        subresource.pixels.resize(dataSize);
        memcpy(subresource.pixels.data(), pImages[i].pixels, dataSize);
    }
    texture->SetImages(images);

    delete[] originalFileBuffer;
}

void TextureImporter::Save(const std::shared_ptr<TextureAsset>& texture)
{
    // ------------- META ----------------------

    std::string metaPath = texture->GetAssetPath() + META_EXT;
    rapidjson::Document doc;
    Json meta = Json(doc);
    ModuleFileSystem::LoadJson(metaPath.c_str(), meta);
    meta["texConfigFlags"] = texture->GetConfigFlags();
    meta["texConversionFlags"] = texture->GetConversionFlags();

    rapidjson::StringBuffer buffer = meta.ToBuffer();
    ModuleFileSystem::SaveFile(metaPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());

    // ------------- BINARY ----------------------

    UINT configFlags = texture->GetConfigFlags();

    unsigned int header[2] = { static_cast<unsigned int>(texture->GetName().size()), configFlags };
    UINT size = sizeof(header);

    size += sizeof(char) * static_cast<unsigned int>(texture->GetName().size());

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    bytes = sizeof(char) * static_cast<unsigned int>(texture->GetName().size());
    memcpy(cursor, &texture->GetName()[0], bytes);
    cursor += bytes;

    ModuleFileSystem::SaveFile(texture->GetLibraryPath().c_str(), fileBuffer, size);

    delete[] fileBuffer;
}