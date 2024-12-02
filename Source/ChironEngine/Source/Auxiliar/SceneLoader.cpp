#include "Pch.h"
#include "SceneLoader.h"

#include "Application.h"

#include "Modules/ModuleFileSystem.h"
#include "Modules/ModuleScene.h"

#include "DataModels/GameObject/GameObject.h"
#include "DataModels/Scene/Scene.h"

namespace Chiron::Loader
{
    namespace
    {
        struct LoadingConfig
        {
            std::function<void(void)> callback;
            bool mantainCurrentScene;
            std::optional<std::string> scenePath;
            std::optional<std::reference_wrapper<rapidjson::Document>> doc;
        };

        std::optional<LoadingConfig> loadingConfig;

        void Loading(Json& json)
        {
            auto sceneModule = App->GetModule<ModuleScene>();

            if (!loadingConfig->mantainCurrentScene)
            {
                sceneModule->SetLoadedScene(new Scene());
            }

            std::unordered_map<UID, GameObject*> restoredGameObjects;
            std::unordered_map<UID, UID> regeneratedUIDs;

            // ------------- GENERATE GAMEOBJECTS AND ITS COMPONENTS ----------------------

            auto gameObjects = json["GameObjects"];
            for (int i = 0; i < gameObjects.Size(); i++)
            {
                GameObject* gameObject = new GameObject(gameObjects[i]["GameObject"]);
                if (loadingConfig->mantainCurrentScene)
                {
                    UID oldUID = gameObject->GetUID();
                    gameObject->ReGenerateUID();
                    regeneratedUIDs[oldUID] = gameObject->GetUID();
                }
                restoredGameObjects[gameObject->GetUID()] = gameObject;
            }

            // ------------- LINK COMPONENTS/PARENT ----------------------
            
            std::vector<std::thread> threads;
            for (int i = 0; i < gameObjects.Size(); i++)
            {
                auto gameObjectField = gameObjects[i]["GameObject"];
                UID uid = gameObjectField["uid"];
                if (loadingConfig->mantainCurrentScene)
                {
                    uid = regeneratedUIDs[uid];
                }
                auto gameObject = restoredGameObjects[uid];

                // ------------- LINK COMPONENTS ----------------------

                auto components = gameObjectField["Components"];
                threads.emplace_back(
                    [&]() {
                    gameObject->Load(components);
                    });

                // ------------- LINK PARENT ----------------------

                UID parentUID = gameObjectField["uidParent"];
                // imported scene root case
                if (loadingConfig->mantainCurrentScene && parentUID == 0)
                {
                    sceneModule->GetRoot()->LinkChild(gameObject);
                }
                else if (parentUID == 0)
                {
                    sceneModule->SetRoot(gameObject);
                }
                else if (parentUID != 0)
                {
                    if (loadingConfig->mantainCurrentScene)
                    {
                        parentUID = regeneratedUIDs[parentUID];
                    }
                    auto parent = restoredGameObjects[parentUID];
                    parent->LinkChild(gameObject);
                }
            }

            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }

            // ------------- ON AWAKE GAMEOBJECTS ----------------------

            for (auto& gameObject : sceneModule->GetLoadedScene()->GetSceneGameObjects())
            {
                gameObject->OnAwake();
            }

            // ------------- CORRECT SOME GAMEOBJECTS ----------------------


            // ------------- CALLBACK AND RESET ----------------------

            loadingConfig->callback();
            loadingConfig.reset();
        }

        void StartLoading()
        {
            if (loadingConfig->scenePath.has_value())
            {
                LOG_INFO("Started load of scene {}", loadingConfig->scenePath.value());

                rapidjson::Document doc;
                Json json = Json(doc);
                if (ModuleFileSystem::LoadJson(loadingConfig->scenePath->c_str(), json) == -1)
                {
                    LOG_ERROR("Couldn't load scene file.");
                    return;
                }
                Loading(json);
            }
            else if (loadingConfig->doc.has_value())
            {
                Json json(loadingConfig->doc.value());
                Loading(json);
            }
        }
    }

    void LoadScene(std::variant<std::string, std::reference_wrapper<rapidjson::Document>>&& scenePathOrDocument,
        std::function<void(void)>&& callback,
        bool mantainCurrentScene /* = false */)
    {
        if (LoadInProcess())
        {
            LOG_ERROR("There is already a scene loading");
            return;
        }

        loadingConfig = LoadingConfig(std::move(callback), mantainCurrentScene);
        if (std::holds_alternative<std::string>(scenePathOrDocument))
        {
            loadingConfig->scenePath = std::move(std::get<std::string>(scenePathOrDocument));
        }
        else
        {
            loadingConfig->doc = std::move(std::get<std::reference_wrapper<rapidjson::Document>>(scenePathOrDocument));
        }

        // Async Load
        std::thread startLoadThread = std::thread(&StartLoading);
        startLoadThread.detach();
    }

    bool LoadInProcess()
    {
        return loadingConfig.has_value();
    }
}
