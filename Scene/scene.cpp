#include "scene.hpp"
#include <future>
namespace ve
{
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
        m_Registry.destroy(entity);
        m_EntityCount--;
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
        addComponent<veModel>(entity, model_path + filename);
		auto &model = getComponent<veModel>(entity);
		model.loadModel();
		return {name, entity};
    }

	void Scene::loadModels()
	{
		auto view = getComponentView<veModel>();
		std::vector<std::future<void>> futures;
		for(auto& entity : view)
		{
			futures.emplace_back(std::async(std::launch::async, [&]()
			{
				auto& model = getComponent<veModel>(entity);
				model.loadModel();
			}));
		}
	}
} // namespace ve