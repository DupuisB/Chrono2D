#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

#include <set>

#include "ComponentArray.hpp"

class EntityManager {
private:
    int numEntities = 0;
    std::set<Entity> availableIds;

public:
    Entity createEntity() {
        if (numEntities >= MAX_ENTITIES) {
            throw std::out_of_range("Maximum number of entities reached");
        }
        Entity id;
        if (!availableIds.empty()) {
            id = *availableIds.begin();
            availableIds.erase(availableIds.begin());
        } else {
            id = numEntities;
            numEntities++;
        }
        return id;
    }

    void destroyEntity(Entity entity) {
        if (entity >= MAX_ENTITIES) {
            throw std::out_of_range("Entity index out of range");
        }
        numEntities--;
        availableIds.insert(entity);
    }
};

#endif // ENTITY_MANAGER_HPP