#pragma once

#include "DataModels/FileSystem/Json/Json.h"
#include "DataModels/FileSystem/UID/UID.h"

class GameObject;
class Drawable;
class Updatable;

class Scene
{
public:
    Scene(const std::string& sceneName = "New Scene");
    ~Scene();

    void PreUpdate();
    void Update();
    void PostUpdate();

    void CleanUp();

    GameObject* SearchGameObjectByUID(UID uid);

    void Save(Json& json);

    // ------------- MODIFY CONTAINERS ----------------------

    void AddGameObject(GameObject* gameObject);
    void AddStaticGO(GameObject* gameObject);
    void AddDynamicGO(GameObject* gameObject);
    void AddDrawableComponent(Drawable* drawable);
    void AddUpdatableComponent(Updatable* updatable);

    void RemoveGameObject(GameObject* gameObject);
    void RemoveStaticGO(GameObject* gameObject);
    void RemoveDynamicGO(GameObject* gameObject);
    void RemoveDrawableComponent(Drawable* drawable);
    void RemoveUpdatableComponent(Updatable* updatable);

    // ------------- GETTERS ----------------------

    inline GameObject* GetRoot() const;
    inline const std::vector<GameObject*>& GetStaticGOs() const;
    inline const std::vector<GameObject*>& GetDynamicsGOs() const;

private:
    std::unique_ptr<GameObject> _root;
    std::vector<GameObject*> _sceneGameObjects;

    std::vector<GameObject*> _dynamicsGOs;
    std::vector<GameObject*> _staticGOs;
    std::vector<Drawable*> _drawableComponents;
    std::vector<Updatable*> _updatableComponents;
};

inline void Scene::AddGameObject(GameObject* gameObject)
{
    _sceneGameObjects.push_back(gameObject);
}

inline void Scene::AddStaticGO(GameObject* gameObject)
{
    _staticGOs.push_back(gameObject);
}

inline void Scene::AddDynamicGO(GameObject* gameObject)
{
    _dynamicsGOs.push_back(gameObject);
}

inline void Scene::AddDrawableComponent(Drawable* drawable)
{
    _drawableComponents.push_back(drawable);
}

inline void Scene::AddUpdatableComponent(Updatable* updatable)
{
    _updatableComponents.push_back(updatable);
}

inline GameObject* Scene::GetRoot() const
{
    return _root.get();
}

inline const std::vector<GameObject*>& Scene::GetStaticGOs() const
{
    return _staticGOs;
}

inline const std::vector<GameObject*>& Scene::GetDynamicsGOs() const
{
    return _dynamicsGOs;
}
