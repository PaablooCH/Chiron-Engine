#include "Pch.h"
#include "MaterialImporter.h"

#include "Application.h"

#include "Modules/ModuleResources.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/TextureAsset.h"

MaterialImporter::MaterialImporter()
{
}

MaterialImporter::~MaterialImporter()
{
}

void MaterialImporter::Import(const char* filePath, const std::shared_ptr<MaterialAsset>& material)
{
    material->SetName(ModuleFileSystem::GetFile(filePath));

    rapidjson::Document doc;
    Json json = Json(doc);
    ModuleFileSystem::LoadJson(filePath, json);

    auto resources = App->GetModule<ModuleResources>();

    std::future<std::shared_ptr<TextureAsset>> futureBase;
    std::future<std::shared_ptr<TextureAsset>> futureNormalMap;
    std::future<std::shared_ptr<TextureAsset>> futureOcclusion;
    std::future<std::shared_ptr<TextureAsset>> futureProperty;
    std::future<std::shared_ptr<TextureAsset>> futureEmissive;

    bool hasBase = false;
    bool hasNormal = false;
    bool hasOcclusion = false;
    bool hasProperty = false;
    bool hasEmissive = false;
    
    std::string path = json["BaseTexturePath"];
    if (path != "")
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseBase;
        resources->RequestAsset<TextureAsset>(path, promiseBase);
        hasBase = true;
    }

    path = json["NormalMapPath"];
    if (path != "")
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseNormalMap;
        resources->RequestAsset<TextureAsset>(path, promiseNormalMap);
        hasNormal = true;
    }

    path = json["AmbientOcclusionPath"];
    if (path != "")
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseOcclusion;
        resources->RequestAsset<TextureAsset>(path, promiseOcclusion);
        hasOcclusion = true;
    }

    path = json["PropertyTexturePath"];
    if (path != "")
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseProperty;
        resources->RequestAsset<TextureAsset>(path, promiseProperty);
        hasProperty = true;
    }

    path = json["EmissiveTexturePath"];
    if (path != "")
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseEmissive;
        resources->RequestAsset<TextureAsset>(path, promiseEmissive);
        hasEmissive = true;
    }
    
    if (hasBase)
    {
        material->SetBaseTexture(futureBase.get());
    }
    if (hasNormal)
    {
        material->SetNormalMap(futureNormalMap.get());
    }
    if (hasOcclusion)
    {
        material->SetAmbientOcclusion(futureOcclusion.get());
    }
    if (hasProperty)
    {
        material->SetPropertyTexture(futureProperty.get());
    }
    if (hasEmissive)
    {
        material->SetEmissiveTexture(futureEmissive.get());
    }

    Save(material);
}

void MaterialImporter::Load(const char* libraryPath, const std::shared_ptr<MaterialAsset>& material)
{
    if (!ModuleFileSystem::ExistsFile(libraryPath))
    {
        // ------------- META ----------------------

        std::string metaPath = material->GetAssetPath() + META_EXT;
        rapidjson::Document doc;
        Json meta = Json(doc);
        ModuleFileSystem::LoadJson(metaPath.c_str(), meta);

        Color baseColor = Color(meta["BaseColor"]["R"], meta["BaseColor"]["G"], meta["BaseColor"]["B"], meta["BaseColor"]["A"]);
        material->SetBaseColor(baseColor);
        Color specularColor = Color(meta["SpecularColor"]["R"], meta["SpecularColor"]["G"], meta["SpecularColor"]["B"], meta["SpecularColor"]["A"]);
        material->SetSpecularColor(specularColor);
        UINT options = meta["Options"];
        material->SetOptions(options);

        // ------------- REIMPORT FILE ----------------------

        std::string assetPath = meta["assetPath"];
        Import(assetPath.c_str(), material);

        return;
    }

    char* fileBuffer;
    ModuleFileSystem::LoadFile(libraryPath, fileBuffer);
    char* fileBufferOriginal = fileBuffer;

    // ------------- BINARY ----------------------

    auto resourceModule = App->GetModule<ModuleResources>();

    std::future<std::shared_ptr<TextureAsset>> futureBase;
    std::future<std::shared_ptr<TextureAsset>> futureNormalMap;
    std::future<std::shared_ptr<TextureAsset>> futureOcclusion;
    std::future<std::shared_ptr<TextureAsset>> futureProperty;
    std::future<std::shared_ptr<TextureAsset>> futureEmissive;

    bool hasBase = false;
    bool hasNormal = false;
    bool hasOcclusion = false;
    bool hasProperty = false;
    bool hasEmissive = false;

    unsigned int header[1];
    unsigned int bytes = sizeof(header);
    memcpy(header, fileBuffer, bytes);
    fileBuffer += bytes;

    material->SetName(std::string(fileBuffer, header[0]));
    fileBuffer += header[0];

    bytes = sizeof(UID);
    UID textureUID;
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseBase;
        futureBase = resourceModule->SearchAsset<TextureAsset>(textureUID, promiseBase);
        hasBase = true;
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseNormalMap;
        futureNormalMap = resourceModule->SearchAsset<TextureAsset>(textureUID, promiseNormalMap);
        hasNormal = true;
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseOcclusion;
        futureOcclusion = resourceModule->SearchAsset<TextureAsset>(textureUID, promiseOcclusion);
        hasOcclusion = true;
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseProperty;
        futureProperty = resourceModule->SearchAsset<TextureAsset>(textureUID, promiseProperty);
        hasProperty = true;
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        std::promise<std::shared_ptr<TextureAsset>> promiseEmissivea;
        futureEmissive = resourceModule->SearchAsset<TextureAsset>(textureUID, promiseEmissivea);
        hasEmissive = true;
    }
    fileBuffer += bytes;

    bytes = sizeof(Color);
    Color color;
    memcpy(&color, &material->GetBaseColor(), bytes);
    material->SetBaseColor(color);
    fileBuffer += bytes;

    memcpy(&color, fileBuffer, bytes);
    material->SetSpecularColor(color);
    fileBuffer += bytes;

    bytes = sizeof(UINT);
    UINT options;
    memcpy(&options, fileBuffer, bytes);
    material->SetOptions(options);

    if (hasBase)
    {
        material->SetBaseTexture(futureBase.get());
    }
    if (hasNormal)
    {
        material->SetNormalMap(futureNormalMap.get());
    }
    if (hasOcclusion)
    {
        material->SetAmbientOcclusion(futureOcclusion.get());
    }
    if (hasProperty)
    {
        material->SetPropertyTexture(futureProperty.get());
    }
    if (hasEmissive)
    {
        material->SetEmissiveTexture(futureEmissive.get());
    }

    delete[] fileBufferOriginal;
}

void MaterialImporter::Save(const std::shared_ptr<MaterialAsset>& material)
{
    // ------------- META ----------------------

    rapidjson::Document doc;
    Json json = Json(doc);

    json["BaseColor"]["R"] = material->GetBaseColor().R();
    json["BaseColor"]["G"] = material->GetBaseColor().G();
    json["BaseColor"]["B"] = material->GetBaseColor().B();
    json["BaseColor"]["A"] = material->GetBaseColor().A();

    json["SpecularColor"]["R"] = material->GetSpecularColor().R();
    json["SpecularColor"]["G"] = material->GetSpecularColor().G();
    json["SpecularColor"]["B"] = material->GetSpecularColor().B();
    json["SpecularColor"]["A"] = material->GetSpecularColor().A();
    json["Options"] = material->GetOptions();

    auto filebuffer = json.ToBuffer();
    ModuleFileSystem::SaveFile((material->GetAssetPath() + META_EXT).c_str(), filebuffer.GetString(), filebuffer.GetSize());

    // ------------- BINARY ----------------------

                //textures' uid     //base/specular color  //options
    UINT size = (sizeof(UID) * 5) + sizeof(Color) * 2 + sizeof(UINT);

    unsigned int header[1] = { static_cast<unsigned int>(material->GetName().size()) };

    size += sizeof(header);
    size += sizeof(char) * static_cast<unsigned int>(material->GetName().size());

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(header);
    memcpy(cursor, header, bytes);
    cursor += bytes;

    bytes = sizeof(char) * static_cast<unsigned int>(material->GetName().size());
    memcpy(cursor, &material->GetName()[0], bytes);
    cursor += bytes;

    bytes = sizeof(UID);
    UID textureUID = material->GetBaseTexture() ? material->GetBaseTexture()->GetUID() : 0;
    memcpy(cursor, &textureUID, bytes);
    cursor += bytes;

    textureUID = material->GetNormalMap() ? material->GetNormalMap()->GetUID() : 0;
    memcpy(cursor, &textureUID, bytes);
    cursor += bytes;

    textureUID = material->GetAmbientOcclusion() ? material->GetAmbientOcclusion()->GetUID() : 0;
    memcpy(cursor, &textureUID, bytes);
    cursor += bytes;

    textureUID = material->GetPropertyTexture() ? material->GetPropertyTexture()->GetUID() : 0;
    memcpy(cursor, &textureUID, bytes);
    cursor += bytes;

    textureUID = material->GetEmissiveTexture() ? material->GetEmissiveTexture()->GetUID() : 0;
    memcpy(cursor, &textureUID, bytes);
    cursor += bytes;

    bytes = sizeof(Color);
    memcpy(cursor, &material->GetBaseColor(), bytes);
    cursor += bytes;

    memcpy(cursor, &material->GetSpecularColor(), bytes);
    cursor += bytes;

    bytes = sizeof(UINT);
    UINT options = material->GetOptions();
    memcpy(cursor, &options, bytes);

    ModuleFileSystem::SaveFile(material->GetLibraryPath().c_str(), fileBuffer, size);

    delete[] fileBuffer;
}