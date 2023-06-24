#include "Scene/Scene.hpp"
#include "Events/MouseEvents.hpp"
#include "Utils/Path.hpp"
#include "Core/GLTFRenderer.hpp"

namespace Nyxis
{
	struct Model;

	Scene::Scene(const std::string name)
		: m_SceneName(name)
    {
        m_CameraEntity = CreateEntity("Camera");
        m_Camera = new Camera(m_Registry.get<TransformComponent>(m_CameraEntity));
        m_Camera->getCameraController().setCameraType(CameraType::Perspective);
    }

    Scene::~Scene()
    {
        m_Registry.clear();
    }

    Entity Scene::CreateEntity(const std::string &name)
    {
        auto entity = m_Registry.create();
        m_Registry.emplace<TagComponent>(entity, name);
        m_Registry.emplace<TransformComponent>(entity);
        m_EntityCount++;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityDeletionQueue.emplace(entity);
    }
    /**
     * @note - It makes sense to use this function only for imgui interface which will have its own class in the future
     *
     * @param filename - path to the file with .obj extension
     * @return std::pair<std::string, Entity> - pair of the name of the object and the entity
     */
    std::pair<std::string, Entity> Scene::AddEntity(const std::string &filename)
    {
        auto name = filename.substr(0, filename.find_last_of('.'));
        auto entity = CreateEntity(name);
        AddComponent<TransformComponent>(entity, glm::vec3(0.f, 0.f, 0.f), glm::vec3(.0f, .0f, 0.0f), glm::vec3(1.f, 1.f, 1.f), 0.0f);
        AddComponent<MeshComponent>(entity, model_path + filename);
        auto &model = GetComponent<MeshComponent>(entity);
        model.model->loadModel();
        return {name, entity};
    }

    void Scene::OnUpdate(float dt, float aspect)
    {
        if (SaveSceneFlag)
        {
            SaveSceneFlag = false;
            vkDeviceWaitIdle(device.device());
        }
        if (LoadSceneFlag)
        {
            LoadSceneFlag = false;
            vkDeviceWaitIdle(device.device());
        }

    	if (m_Camera->getType() == CameraType::Perspective)
    	{
            if (aspect > 0)
                m_Camera->setPerspectiveProjection(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
    	}
        else
            m_Camera->setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.1f, 1000.0f);


        if (!Input::IsMouseButtonPressed(MouseCodes::MouseButtonRight))
        {
            m_Camera->setMousePosition(Input::GetMousePosition());
        }

        if(m_CameraControl)
			m_Camera->OnUpdate(dt);

    	GetComponentView<Player, TransformComponent>().each([&](auto entity, auto &player, auto &transform)
        {
	        player.OnUpdate(dt, transform);
        });

        if (!m_EntityDeletionQueue.empty())
        {
            vkDeviceWaitIdle(device.device());
            for (int i = 0; i < m_EntityDeletionQueue.size(); i++)
            {
                const auto entity = m_EntityDeletionQueue.front();
                m_EntityDeletionQueue.pop();
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
			{ m_EntityDeletionQueue.emplace(entity); });
		OBJModel::ReleaseModels();
    }

	void Scene::LoadModel(const Entity entity, const std::string &filename)
    {
        vkDeviceWaitIdle(device.device());
        m_Registry.remove<Model>(entity);
		m_Registry.emplace<Model>(entity, filename);
	}
} // namespace Nyxis