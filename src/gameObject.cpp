#include "gameObject.hpp"

namespace ve
{
    veGameObject veGameObject::makePointLight(float intensity, float radius, glm::vec3 color)
    {
        veGameObject gameObj = veGameObject::createGameObject();
        gameObj.color = color;
        gameObj.transform.scale.x = radius;
        gameObj.pointLight = std::make_unique<PointLightComponent>();
        gameObj.pointLight->lightIntensity = intensity;
        return gameObj;
    }
}