#include "Pch.h"
#include "MaterialImporter.h"

#include "Application.h"

#include "Modules/ModuleFileSystem.h"
#include "Modules/ModuleResources.h"

#include "DataModels/Assets/MaterialAsset.h"
#include "DataModels/Assets/TextureAsset.h"

#include "Defines/FileSystemDefine.h"

MaterialImporter::MaterialImporter()
{
}

MaterialImporter::~MaterialImporter()
{
}

void MaterialImporter::Import(const char* filePath, const std::shared_ptr<MaterialAsset>& material)
{
    rapidjson::Document doc;
    Json json = Json(doc);
    ModuleFileSystem::LoadJson(filePath, json);
    
    std::string path = json["BaseTexturePath"];
    if (path == "")
    {

    }

    path = json["NormalMapPath"];
    if (path == "")
    {

    }

    path = json["AmbientOcclusionPath"];
    if (path == "")
    {

    }

    path = json["PropertyTexturePath"];
    if (path == "")
    {

    }

    path = json["EmissiveTexturePath"];
    if (path == "")
    {

    }

    Color baseColor = Color(json["BaseColor"]["R"], json["BaseColor"]["G"], json["BaseColor"]["B"], json["BaseColor"]["A"]);
    material->SetBaseColor(baseColor);
    Color specularColor = Color(json["SpecularColor"]["R"], json["SpecularColor"]["G"], json["SpecularColor"]["B"], json["SpecularColor"]["A"]);
    material->SetSpecularColor(specularColor);
    UINT options = json["Options"];
    material->SetOptions(options);
    Save(material);
}

void MaterialImporter::Load(const char* fileBuffer, const std::shared_ptr<MaterialAsset>& material)
{
    // ------------- BINARY ----------------------

    unsigned int bytes = sizeof(UID);
    UID textureUID;
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        material->SetBaseTexture(nullptr);
    }
    fileBuffer += bytes;

    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        material->SetNormalMap(nullptr);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        material->SetAmbientOcclusion(nullptr);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        material->SetPropertyTexture(nullptr);
    }
    fileBuffer += bytes;
    
    memcpy(&textureUID, fileBuffer, bytes);
    if (textureUID != 0)
    {
        material->SetEmissiveTexture(nullptr);
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
}

void MaterialImporter::Save(const std::shared_ptr<MaterialAsset>& material)
{
    // ------------- META ----------------------

    rapidjson::Document doc;
    Json json = Json(doc);
    std::string path = "";
    auto texture = material->GetBaseTexture();
    if (texture)
    {
        path = texture->GetAssetPath();
    }
    json["BaseTexturePath"] = path;
    path.clear();
    
    auto texture = material->GetNormalMap();
    if (texture)
    {
        path = texture->GetAssetPath();
    }
    json["NormalMapPath"] = path;
    path.clear();
    
    auto texture = material->GetAmbientOcclusion();
    if (texture)
    {
        path = texture->GetAssetPath();
    }
    json["AmbientOcclusionPath"] = path;
    path.clear();
    
    auto texture = material->GetPropertyTexture();
    if (texture)
    {
        path = texture->GetAssetPath();
    }
    json["PropertyTexturePath"] = path;
    path.clear();
    
    auto texture = material->GetEmissiveTexture();
    if (texture)
    {
        path = texture->GetAssetPath();
    }
    json["EmissiveTexturePath"] = path;

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

    char* fileBuffer = new char[size] {};
    char* cursor = fileBuffer;

    unsigned int bytes = sizeof(UID);
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
