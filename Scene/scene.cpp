#include "scene.hpp"
#include "ThreadPool.hpp"
#include "utils.hpp"

#include "json/json.hpp"

namespace Nyxis
{
    Scene::Scene()
    {
        m_CameraEntity = createEntity("Camera");
        auto& transform = addComponent<TransformComponent>(m_CameraEntity, glm::vec3{ 0, 0, -14 });
        auto& rigidBody = addComponent<RigidBody>(m_CameraEntity);
        m_Camera = new Camera(rigidBody);
        m_Camera->getCameraController().setCameraType(CameraType::Perspective);
    }

    Scene::~Scene()
    {
        m_Registry.clear();
    }

    Entity Scene::createEntity(const std::string &name)
    {
        auto entity = m_Registry.create();
        m_Registry.emplace<TagComponent>(entity, name);
        m_EntityCount++;
        return entity;
    }

    void Scene::destroyEntity(Entity entity)
    {
        m_DeletionQueue.emplace(entity);
    }
    /**
     * @note - It makes sense to use this function only for imgui interface which will have its own class in the future
     *
     * @param filename - path to the file with .obj extension
     * @return std::pair<std::string, Entity> - pair of the name of the object and the entity
     */
    std::pair<std::string, Entity> Scene::addEntity(const std::string &filename)
    {
        auto name = filename.substr(0, filename.find_last_of('.'));
        auto entity = createEntity(name);
        addComponent<TransformComponent>(entity, glm::vec3(0.f, 0.f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(1.f, 1.f, 1.f), 0.0f);
        addComponent<MeshComponent>(entity, model_path + filename);
        auto &model = getComponent<MeshComponent>(entity);
        model.model->loadModel();
        return {name, entity};
    }

    void Scene::LoadModels()
    {
        auto &models = veModel::GetModels();

        if (models.empty())
            return;

        ThreadPool pool;

        for (auto &model : models)
        {
            pool.enqueue([&]()
                         { model.second->loadModel(); });
        }
    }
    void Scene::OnUpdate(float dt, float aspect)
    {
        if (SaveSceneFlag)
        {
            SaveSceneFlag = false;
            vkDeviceWaitIdle(device.device());
            SaveScene();
        }
        if (LoadSceneFlag)
        {
            LoadSceneFlag = false;
            vkDeviceWaitIdle(device.device());
            LoadScene(SceneName);
        }

        if (m_Camera->getType() == CameraType::Perspective)
            if (aspect > 0)
				m_Camera->setPerspectiveProjection(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
        else
            m_Camera->setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.1f, 1000.0f);

        m_Camera->OnUpdate(dt);
        getComponentView<Player, RigidBody>().each([&](auto entity, auto &player, auto &rigidBody)
                                                            { player.OnUpdate(dt, rigidBody); });

        if(!m_DeletionQueue.empty())
        {
            vkDeviceWaitIdle(device.device());
            for(int i = 0; i < m_DeletionQueue.size(); i++)
            {
                auto entity = m_DeletionQueue.front();
                m_DeletionQueue.pop();
                m_Registry.destroy(entity);
                m_EntityCount--;
            }

        }
    }

    /**
     * @brief - Clear the scene by destroying all entities and components
     */
    void Scene::ClearScene()
    {
		m_Registry.each([&](auto entity)
			{ m_DeletionQueue.emplace(entity); });
		veModel::ReleaseModels();
    }

    void Scene::SaveScene(const std::string &filename)
    {
        std::ofstream file(filename);

        nlohmann::json j;

        // for each entity add all components to json
        m_Registry.each([&](auto entity)
                        {
                            nlohmann::json entity_json;
                            if (m_Registry.all_of<TagComponent>(entity))
                            {
                                auto &tag = m_Registry.get<TagComponent>(entity);
                                tag.ToJson(entity_json);
                            }

                            if (m_Registry.all_of<TransformComponent>(entity))
                            {
                                auto &transform = m_Registry.get<TransformComponent>(entity);
                                transform.ToJson(entity_json);
                            }

                            if (m_Registry.all_of<MeshComponent>(entity))
                            {
                                auto &mesh = m_Registry.get<MeshComponent>(entity);
                                mesh.ToJson(entity_json);
                            }

                            if (m_Registry.all_of<Player>(entity))
                            {
                                auto &player = m_Registry.get<Player>(entity);
                                player.ToJson(entity_json);
                            }

                            j["entities"].push_back(entity_json);
                        });

        file << j.dump(4);
        file.close();
        LOG_INFO("Scene saved to file: {}", SceneName);
    }

    void Scene::LoadScene(const std::string &filename)
    {
        // ClearScene();
        // veModel::ReleaseModels();
        std::ifstream file(filename);
        nlohmann::json j;
        file >> j;

        for (auto &entity : j["entities"])
        {
            auto entity1 = m_Registry.create();
            m_EntityCount++;
            if (entity.contains("Tag"))
            {
                std::string Tag = entity["Tag"];
                m_Registry.emplace<TagComponent>(entity1, Tag);
            }

            if (entity.contains("Transform"))
            {
                TransformComponent transform{};

                transform.translation.x = entity["Transform"][0][0];
                transform.translation.y = entity["Transform"][0][1];
                transform.translation.z = entity["Transform"][0][2];

                transform.rotation.x = entity["Transform"][1][0];
                transform.rotation.y = entity["Transform"][1][1];
                transform.rotation.z = entity["Transform"][1][2];

                transform.scale.x = entity["Transform"][2][0];
                transform.scale.y = entity["Transform"][2][1];
                transform.scale.z = entity["Transform"][2][2];

                transform.roughness = entity["Transform"][3][0];
                m_Registry.emplace<TransformComponent>(entity1, transform.translation, transform.rotation, transform.scale, transform.roughness);
            }
            if (entity.contains("MeshComponent"))
            {
                std::string filepath = entity["MeshComponent"]["filepath"];
                m_Registry.emplace<MeshComponent>(entity1, filepath);
            }
            if (entity.contains("Player"))
            {
                m_Registry.emplace<Player>(entity1, Player{});
            }
        }

        file.close();
        LoadModels();
        LOG_INFO("Scene {} loaded", SceneName);
    }
} // namespace Nyxis