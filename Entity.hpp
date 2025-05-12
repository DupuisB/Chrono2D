#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <queue>
#include <bitset>
#include <array>

const int MAX_COMPONENTS = 16;
const int MAX_ENTITIES = 1000;

class EntityManager {
    private:
        // A queue to manage available entity IDs
        std::queue<int> mAvailableEntities{};

        // A bitset to represent the signature of each entity
        // (tells which components are attached to the entity)
        std::array<std::bitset<MAX_COMPONENTS>, MAX_ENTITIES> mSignatures{};

        // Active entity counter
        int mEntityCount{0};
    public:
        EntityManager() {
            // Initialize the entity pool
            for (int i = 0; i < MAX_ENTITIES; ++i) {
                mAvailableEntities.push(i);
            }
        }

        // returns an entity ID
        int createEntity() {
            // check if the entity limit has been reached
            if (mEntityCount < MAX_ENTITIES) {
                throw std::runtime_error("Entity limit reached");
            }
            // take the first available entity ID
            int id = mAvailableEntities.front();
            mAvailableEntities.pop();
            mEntityCount++;

            return id;
        }

        void destroyEntity(int entity) {
            // check if the entity ID is valid
            if (entity < 0 || entity >= MAX_ENTITIES) {
                throw std::out_of_range("Invalid entity ID");
            }
            // check if the entity is already destroyed
            if (mSignatures[entity].none()) {
                throw std::runtime_error("Entity already destroyed");
            }
            // remove any components associated with the entity
            mSignatures[entity].reset();
            // add the entity ID back to the available pool
            mAvailableEntities.push(entity);
            mEntityCount--;
        }

        void setSignature(int entity, std::bitset<MAX_COMPONENTS> signature) {
            // check if the entity ID is valid
            if (entity < 0 || entity >= MAX_ENTITIES) {
                throw std::out_of_range("Invalid entity ID");
            }
            // set the signature for the entity
            mSignatures[entity] = signature;
        }

        std::bitset<MAX_COMPONENTS> getSignature(int entity) {
            // check if the entity ID is valid
            if (entity < 0 || entity >= MAX_ENTITIES) {
                throw std::out_of_range("Invalid entity ID");
            }
            // return the signature for the entity
            return mSignatures[entity];
        }
};

#endif // ENTITY_HPP