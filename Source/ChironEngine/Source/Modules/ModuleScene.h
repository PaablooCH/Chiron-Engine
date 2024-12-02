#pragma once
#include "Module.h"

#include "DataModels/FileSystem/UID/UID.h"

class GameObject;
class Drawable;
class Scene;
class Updatable;

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

    void SaveScene();
    void LoadScene(const std::string& scenePath, std::function<void(void)>&& callback, bool mantainCurrentScene = false);
    GameObject* GetRoot() const;
    void SetRoot(GameObject* newRoot);

    GameObject* SearchGameObjectByUID(UID uid);
    GameObject* CreateGameObject(const std::string& name, GameObject* parent);
    void AddGameObject(GameObject* gameObject);
    void RemoveGameObject(GameObject* gameObject);
    void RemoveFromScene(GameObject* gameObject);

    void AddStaticGO(GameObject* gameObject);
    void AddDynamicGO(GameObject* gameObject);
    void AddDrawableComponent(Drawable* drawable);
    void AddUpdatableComponent(Updatable* updatable);

    void RemoveStaticGO(GameObject* gameObject);
    void RemoveDynamicGO(GameObject* gameObject);
    void RemoveDrawableComponent(Drawable* drawable);
    void RemoveUpdatableComponent(Updatable* updatable);

    // ------------- GETTERS ----------------------

    inline Scene* GetLoadedScene();
    inline GameObject* GetSelectedGameObject();

    // ------------- SETTERS ----------------------

    void SetSelectedGameObject(GameObject* newSelected);
    void SetLoadedScene(Scene* newScene);

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
