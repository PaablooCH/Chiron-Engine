#include "Pch.h"
#include "Scene.h"

#include "DataModels/GameObject/GameObject.h"

#include "DataModels/Components/Interfaces/Drawable.h"
#include "DataModels/Components/Interfaces/Updatable.h"

Scene::Scene(const std::string& sceneName)
{
    _root = std::make_unique<GameObject>(sceneName);
}

Scene::~Scene()
{
}

void Scene::PreUpdate()
{
    for (Updatable* updatable : _updatableComponents)
    {
        updatable->PreUpdate(true);
    }
}

void Scene::Update()
{
    for (Updatable* updatable : _updatableComponents)
    {
        updatable->Update(true);
    }
    for (Drawable* drawable : _drawableComponents)
    {
        drawable->Draw();
    }
}

void Scene::PostUpdate()
{
    for (Updatable* updatable : _updatableComponents)
    {
        updatable->PostUpdate(true);
    }
}

void Scene::CleanUp()
{
    _root.reset();
    _sceneGameObjects.clear();
    _drawableComponents.clear();
    _updatableComponents.clear();
}

GameObject* Scene::SearchGameObjectByUID(UID uid)
{
    auto gameObjectIt = std::find_if(_sceneGameObjects.begin(), _sceneGameObjects.end(),
        [uid](GameObject* gameObject)
        {
            return gameObject->GetUID() == uid;
        });

    if (gameObjectIt == _sceneGameObjects.end())
    {
        LOG_TRACE("UID: {}, doesn't exist.", uid);
        return nullptr;
    }

    return (*gameObjectIt);
}

void Scene::RemoveGameObject(GameObject* gameObject)
{
    if (gameObject && gameObject != _root.get())
    {
        delete gameObject->GetParent()->UnLinkChild(gameObject);
    }
}

void Scene::RemoveStaticGO(GameObject* gameObject)
{
    assert(gameObject);
    std::erase(_staticGOs, gameObject);
}

void Scene::RemoveDynamicGO(GameObject* gameObject)
{
    assert(gameObject);
    std::erase(_dynamicsGOs, gameObject);
}

void Scene::RemoveDrawableComponent(Drawable* drawable)
{
    assert(drawable);
    std::erase(_drawableComponents, drawable);
}

void Scene::RemoveUpdatableComponent(Updatable* updatable)
{
    assert(updatable);
    std::erase(_updatableComponents, updatable);
}
