#pragma once
#include "Module.h"

#include "DataModels/FileSystem/UID.h"

class GameObject;
class Scene;

class ModuleScene : public Module
{
public:
    ModuleScene();
    ~ModuleScene();

    bool Init() override;
    bool Start() override;
    UpdateStatus PreUpdate() override;
    UpdateStatus Update() override;
    UpdateStatus PostUpdate() override;
    bool CleanUp() override;

    // ------------- SCENE METHODS ----------------------

    GameObject* GetRoot() const;

    GameObject* SearchGameObjectByUID(UID uid);
    GameObject* CreateGameObject(const std::string& name, GameObject* parent);
    void RemoveGameObject(GameObject* gameObject);

    // ------------- GETTERS ----------------------

    inline Scene* GetLoadedScene();
    inline GameObject* GetSelectedGameObject();

    // ------------- SETTERS ----------------------

    inline void SetSelectedGameObject(GameObject* newSelected);

private:

private:
    std::unique_ptr<Scene> _loadedScene;
    GameObject* _selectedGameObject;
};

inline Scene* ModuleScene::GetLoadedScene()
{
    return _loadedScene.get();
}

inline GameObject* ModuleScene::GetSelectedGameObject()
{
    return _selectedGameObject;
}

inline void ModuleScene::SetSelectedGameObject(GameObject* newSelected)
{
    _selectedGameObject = newSelected;
}
