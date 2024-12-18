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
    
    std::string path = json["BaseTexturePath"];
    if (path != "")
    {
        material->SetBaseTexture(resources->RequestAsset<TextureAsset>(path));
    }

    path = json["NormalMapPath"];
    if (path != "")
    {
        material->SetNormalMap(resources->RequestAsset<TextureAsset>(path));
    }

    path = json["AmbientOcclusionPath"];
    if (path != "")
    {
        material->SetAmbientOcclusion(resources->RequestAsset<TextureAsset>(path));
    }

    path = json["PropertyTexturePath"];
    if (path != "")
    {
        material->SetPropertyTexture(resources->RequestAsset<TextureAsset>(path));
    }

    path = json["EmissiveTexturePath"];
    if (path != "")
    {
        material->SetEmissiveTexture(resources->RequestAsset<TextureAsset>(path));
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
        auto texture = resourceModule->SearchAsset<TextureAsset>(textureUID);
        material->SetBaseTexture(texture);
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        auto texture = resourceModule->SearchAsset<TextureAsset>(textureUID);
        material->SetNormalMap(texture);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        auto texture = resourceModule->SearchAsset<TextureAsset>(textureUID);
        material->SetAmbientOcclusion(texture);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        auto texture = resourceModule->SearchAsset<TextureAsset>(textureUID);
        material->SetPropertyTexture(texture);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        auto texture = resourceModule->SearchAsset<TextureAsset>(textureUID);
        material->SetEmissiveTexture(texture);
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
