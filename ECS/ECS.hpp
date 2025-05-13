#ifndef ECS_HPP
#define ECS_HPP

#include <vector>
#include <unordered_map>
#include <memory>

#include "ComponentArray.hpp"
#include "EntityManager.hpp"

class ECS {
private:
    EntityManager entityManager;
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> components;

    // helper function to get the component array for a specific type
    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        std::string typeName = typeid(T).name();
        if (components.find(typeName) == components.end()) {
            // if this component type does not exist
            std::cout << "Creating new component array for type: " << typeName << std::endl;
            components[typeName] = std::make_shared<ComponentArray<T>>();
        }
        return std::static_pointer_cast<ComponentArray<T>>(components[typeName]);
    }
public:
    void init() {
        this->entityManager = EntityManager();
        this->components.clear();
    }

    Entity createEntity() {
        return entityManager.createEntity();
    }

    // destroys the entity and all its components
    void destroyEntity(Entity entity) {
        entityManager.destroyEntity(entity);
        for (auto& [_, componentArray] : components) {
            componentArray->remove(entity);
        }
    }

    // adds a component to the entity
    // if the component type does not exist, it creates a new component array
    // and adds the component to it
    template <typename T>
    void addComponent(Entity entity, T component) {
        auto componentArray = getComponentArray<T>();
        componentArray->add(entity, component);
    }

    // returns data of the component
    // for the given entity
    template <typename T>
    T& getData(Entity entity) {
        auto componentArray = getComponentArray<T>();
        return componentArray->get(entity);
    }

    // returns true if the entity has the component
    template <typename T>
    bool hasComponent(Entity entity) {
        auto componentArray = getComponentArray<T>();
        return componentArray->has(entity);
    }
};



#endif // ECS_HPP