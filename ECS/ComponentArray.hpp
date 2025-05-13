#ifndef COMPONENT_ARRAY_HPP
#define COMPONENT_ARRAY_HPP

#include <array>

using Entity = unsigned int;
const unsigned int MAX_ENTITIES = 100;

class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void remove(Entity entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
    private:
        // Array to hold component data for each entity
        // To get the component data for an entity, use the entity as an index
        std::array<T, MAX_ENTITIES> data;
        std::array<bool, MAX_ENTITIES> used;
        unsigned int size = 0;

    public:

        void add(Entity entity, T component) {
            if (entity >= MAX_ENTITIES) {
                throw std::out_of_range("Entity index out of range");
            }
            data[entity] = component;
            used[entity] = true;
            size++;
        }

        void remove(Entity entity) {
            if (entity >= MAX_ENTITIES) {
                throw std::out_of_range("Entity index out of range");
            }
            data[entity] = T(); // Reset the component data
            used[entity] = false;
            size--;
        }

        T& get(Entity entity) {
            if (entity >= MAX_ENTITIES) {
                throw std::out_of_range("Entity index out of range");
            }
            if (!used[entity]) {
                throw std::runtime_error("Component not found for entity");
            }
            return data[entity];
        }

        bool has(Entity entity) {
            if (entity >= MAX_ENTITIES) {
                throw std::out_of_range("Entity index out of range");
            }
            return used[entity];
        }
};

#endif // COMPONENT_ARRAY_HPP