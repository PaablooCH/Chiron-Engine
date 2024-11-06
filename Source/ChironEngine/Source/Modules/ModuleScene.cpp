#include "Pch.h"
#include "ModuleScene.h"

#include "DataModels/Scene/Scene.h"
#include "DataModels/GameObject/GameObject.h"

#include "DataModels/Components/TransformComponent.h"
ModuleScene::ModuleScene() : _loadedScene(nullptr), _selectedGameObject(nullptr)
{
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Init()
{
    _loadedScene = std::make_unique<Scene>();
    _selectedGameObject = _loadedScene->GetRoot();
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

GameObject* ModuleScene::GetRoot() const
{
    return _loadedScene->GetRoot();
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

void ModuleScene::RemoveGameObject(GameObject* gameObject)
{
    _loadedScene->RemoveGameObject(gameObject);
}