#include "Pch.h"
#include "ModuleScene.h"

#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleResources.h"

#include "DataModels/Scene/Scene.h"

#include "DataModels/Assets/MeshAsset.h"
#include "DataModels/Assets/ModelAsset.h"

#include "DataModels/GameObject/GameObject.h"

#include "DataModels/Components/TransformComponent.h"
#include "DataModels/Components/MeshRendererComponent.h"

#include "Auxiliar/SceneLoader.h"

#include <sstream>

ModuleScene::ModuleScene() : _loadedScene(nullptr), _selectedGameObject(nullptr)
{
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Init()
{
    _loadedScene = std::make_unique<Scene>();
    SetSelectedGameObject(_loadedScene->GetRoot());
    return true;
}

bool ModuleScene::Start()
{
    return true;
}

UpdateStatus ModuleScene::PreUpdate()
{
    _loadedScene->PreUpdate();
    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleScene::Update()
{
    _loadedScene->Update();
    DirectX::Keyboard& keyboard = DirectX::Keyboard::Get();
    const DirectX::Keyboard::State& keyState = keyboard.GetState();
    if (keyState.LeftControl && keyState.S)
    {
        SaveScene();
    }
    if (keyState.LeftControl && keyState.D)
    {
        auto start = std::chrono::steady_clock::now();
        LoadScene("Assets/Scenes/New Scene.chiron",
            [start]()
            {
                auto end = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
                LOG_INFO("Scene Loaded! Took {} seconds", static_cast<int>(duration));
            });
    }
    return UpdateStatus::UPDATE_CONTINUE;
}

UpdateStatus ModuleScene::PostUpdate()
{
    _loadedScene->PostUpdate();
    return UpdateStatus::UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
    _loadedScene->CleanUp();
    return true;
}

void ModuleScene::SaveScene()
{
    if (!ModuleFileSystem::IsDirectory(SCENES_FOLDER))
    {
        ModuleFileSystem::CreateDirectoryC(SCENES_FOLDER);
    }
    rapidjson::Document doc;
    Json json = Json(doc);

    _loadedScene->Save(json);
    auto buffer = json.ToBuffer();

    std::ostringstream oss;
    const std::string& name = _loadedScene->GetRoot()->GetName();
    oss << SCENES_PATH << name << SCENE_EXT;

    ModuleFileSystem::SaveFile(oss.str().c_str(), buffer.GetString(), buffer.GetSize());
}

void ModuleScene::LoadScene(const std::string& scenePath, std::function<void(void)>&& callback, bool mantainCurrentScene /* = false */)
{
    Chiron::Loader::LoadScene(scenePath, std::move(callback), mantainCurrentScene);
}

void ModuleScene::ModelToGameObject(std::string& modelPath)
{
    auto modelFuture = App->GetModule<ModuleResources>()->RequestAsset<ModelAsset>(modelPath.c_str());

    std::shared_ptr<ModelAsset> modelAsset = modelFuture.get();
    GameObject* gameObjectModel = CreateGameObject(modelAsset->GetName(), _loadedScene->GetRoot());

    const std::vector<std::unique_ptr<Node>>& nodes = modelAsset->GetNodes();
    std::unordered_map<int, GameObject*> parentsGameObjects;

    for (int i = 0; i < nodes.size(); ++i)
    {
        Node* node = nodes[i].get();

        GameObject* parent = gameObjectModel;
        if (node->parent != -1)
        {
            parent = parentsGameObjects[node->parent];
        }

        GameObject* gameObjectNode = CreateGameObject(&node->name[0], parent);
        parentsGameObjects[i] = gameObjectNode;

        Vector3 pos;
        Vector3 scale;
        Quaternion rot;
        node->transform.Decompose(scale, rot, pos);

        TransformComponent* transformNode = gameObjectNode->GetInternalComponent<TransformComponent>();
        transformNode->SetLocalPos(pos);
        transformNode->SetLocalRot(rot);
        transformNode->SetLocalSca(scale);

        for (std::pair<std::shared_ptr<MeshAsset>, std::shared_ptr<MaterialAsset>> meshMaterial :
            node->meshMaterial)
        {
            std::shared_ptr<MeshAsset> mesh = meshMaterial.first;
            std::shared_ptr<MaterialAsset> material = meshMaterial.second;

            std::string meshName = ModuleFileSystem::GetFileName(mesh->GetName());
            GameObject* gameObjectModelMesh = CreateGameObject(meshName.c_str(), gameObjectNode);

            MeshRendererComponent* meshMaterial = gameObjectModelMesh->CreateComponent<MeshRendererComponent>();
            meshMaterial->SetMesh(mesh);
            meshMaterial->SetMaterial(material);
        }
    }
    gameObjectModel->GetInternalComponent<TransformComponent>()->UpdateMatrices();
}

GameObject* ModuleScene::GetRoot() const
{
    return _loadedScene->GetRoot();
}

void ModuleScene::SetRoot(GameObject* root)
{
    _loadedScene->SetRoot(root);
    _selectedGameObject = nullptr;
    SetSelectedGameObject(_loadedScene->GetRoot());
}

GameObject* ModuleScene::SearchGameObjectByUID(UID uid)
{
    return _loadedScene->SearchGameObjectByUID(uid);
}

GameObject* ModuleScene::CreateGameObject(const std::string& name, GameObject* parent)
{
    GameObject* newGameObject = new GameObject(name, parent);

    return newGameObject;
}

void ModuleScene::AddGameObject(GameObject* gameObject)
{
    _loadedScene->AddGameObject(gameObject);
}

void ModuleScene::RemoveGameObject(GameObject* gameObject)
{
    _loadedScene->RemoveGameObject(gameObject);
}

void ModuleScene::RemoveFromScene(GameObject* gameObject)
{
    _loadedScene->RemoveFromScene(gameObject);
}

void ModuleScene::AddStaticGO(GameObject* gameObject)
{
    _loadedScene->AddStaticGO(gameObject);
}

void ModuleScene::AddDynamicGO(GameObject* gameObject)
{
    _loadedScene->AddDynamicGO(gameObject);
}

void ModuleScene::AddDrawableComponent(Drawable* drawable)
{
    _loadedScene->AddDrawableComponent(drawable);
}

void ModuleScene::AddUpdatableComponent(Updatable* updatable)
{
    _loadedScene->AddUpdatableComponent(updatable);
}

void ModuleScene::RemoveStaticGO(GameObject* gameObject)
{
    _loadedScene->RemoveStaticGO(gameObject);
}

void ModuleScene::RemoveDynamicGO(GameObject* gameObject)
{
    _loadedScene->RemoveDynamicGO(gameObject);
}

void ModuleScene::RemoveDrawableComponent(Drawable* drawable)
{
    _loadedScene->RemoveDrawableComponent(drawable);
}

void ModuleScene::RemoveUpdatableComponent(Updatable* updatable)
{
    _loadedScene->RemoveUpdatableComponent(updatable);
}

void ModuleScene::SetSelectedGameObject(GameObject* newSelected)
{
    if (_selectedGameObject)
    {
        _selectedGameObject->SetHierarchyState(HierarchyState::NONE);
    }
    _selectedGameObject = newSelected;
    _selectedGameObject->SetHierarchyState(HierarchyState::SELECTED);
}

void ModuleScene::SetLoadedScene(Scene* newScene)
{
    _loadedScene = std::unique_ptr<Scene>(newScene);
    _selectedGameObject = nullptr;
    SetSelectedGameObject(_loadedScene->GetRoot());
}