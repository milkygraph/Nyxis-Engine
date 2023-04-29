#include "Scene/Scene.hpp"
#include "Events/MouseEvents.hpp"
#include "Utils/ThreadPool.hpp"
#include "Utils/Utils.hpp"

#include <json/json.hpp>

namespace Nyxis
{
    Scene::Scene()
    {
        m_CameraEntity = createEntity("Camera");
        auto& transform = addComponent<TransformComponent>(m_CameraEntity, glm::vec3{ 0, 0, -14 });
        m_Camera = new Camera(transform);
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
            SaveScene();
        }
        if (LoadSceneFlag)
        {
            LoadSceneFlag = false;
            vkDeviceWaitIdle(device.device());
            LoadScene(SceneName);
        }

    	if (m_Camera->getType() == CameraType::Perspective)
    	{
            if (aspect > 0)
                m_Camera->setPerspectiveProjection(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
    	}
        else
            m_Camera->setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, 0.1f, 1000.0f);


        if (!Input::isMouseButtonPressed(MouseCodes::MouseButtonRight))
        {
            Input::setCursorMode(CursorMode::CursorNormal);
            SetCameraControl(false);
            m_Camera->setMousePosition(Input::getMousePosition());
        }

        if(m_CameraControl)
			m_Camera->OnUpdate(dt);

    	GetComponentView<Player, TransformComponent>().each([&](auto entity, auto &player, auto &transform)
        {
	        player.OnUpdate(dt, transform);
        });

        if(!m_DeletionQueue.empty())
        {
            vkDeviceWaitIdle(device.device());
            for(int i = 0; i < m_DeletionQueue.size(); i++)
            {
	            const auto entity = m_DeletionQueue.front();
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
		OBJModel::ReleaseModels();
    }

    void Scene::SaveScene(const std::string &filename)
    {
    }

    void Scene::LoadScene(const std::string &filename)
    {
    }
} // namespace Nyxis