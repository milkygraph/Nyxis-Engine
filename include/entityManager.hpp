#pragma once



#include <cstdint>
#include <queue> 
#include <bitset>
#include <array>
#include <cassert>

namespace ve
{
    using Entity = std::uint32_t;
    using ComponentType = std::uint32_t;

    constexpr ComponentType MAX_COMPONENTS = 32;
    constexpr Entity MAX_ENTITIES = 5000;

    using Signature = std::bitset<MAX_COMPONENTS>;

    // Basic entity manager class where entities are just integers and components are bitsets corresponding to the entity's components
    // indexed by the entity's id

    class EntityManager
    {
    public:
        EntityManager()
        {
            for(Entity entity = 0; entity < MAX_ENTITIES; entity++)
            {
                pAvailableEntities.push(entity);
            }
        }

        Entity CreateEntity()
        {
            assert(pLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
            // Take an ID from the front of the queue and pop it
            Entity id = pAvailableEntities.front();
            pAvailableEntities.pop();
            pLivingEntityCount++;

            return id;
        }

        void DestroyEntity(Entity entity)
        {
            assert(entity < MAX_ENTITIES || entity > MAX_ENTITIES && "No such entity exists.");

            // Invalidate the destroyed entity's signature
            pSignatures[entity].reset();

            // Put the destroyed ID at the back of the queue
            pAvailableEntities.push(entity);
            pLivingEntityCount--;
        }

        void SetSignature(Entity entity, Signature signature)
        {
            assert(entity < MAX_ENTITIES || entity > MAX_ENTITIES && "No such entity exists.");

            // Put this entity's signature into the array at the entity's index
            pSignatures[entity] = signature;
        }

        Signature GetSignature(Entity entity)
        {
            assert(entity < MAX_ENTITIES || entity > MAX_ENTITIES && "No such entity exists.");

            // Return this entity's signature from the array at the entity's index
            return pSignatures[entity];
        }

    private:
        // a queue of available entities
        std::queue<int> pAvailableEntities;
        // a list of signatures, where the index is the entity id
        std::array<Signature, MAX_ENTITIES> pSignatures{};
        // the total number of living entities
        uint32_t pLivingEntityCount{};
    };
}