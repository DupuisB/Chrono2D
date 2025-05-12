#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "utils/math.hpp"
#include <array>
#include <unordered_map>

enum class ShapeType {
    CIRCLE,
    RECTANGLE,
    LINE
};

struct position {
    vec2f position;
};

struct rotation {
    float angle;
    float angular_velocity;
    float angular_acceleration;
    float moment_of_inertia;
};

struct momentum {
    vec2f velocity;
    vec2f acceleration;
    float mass;
};

struct Shape {
    vec2f size;
    ShapeType type;
};

class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void EntityDestroyed(int entity) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
    private:
        // Array to hold the components data
        std::array<T, MAX_ENTITIES> mComponentArray{};
        // Map that associates entity IDs with their index
        // tells us the index of the component in the array
        std::unordered_map<int, int> mEntityToIndex{};
        // Map that associates index with entity IDs
        // tells us the entity ID of the component in the array
        std::unordered_map<int, int> mIndexToEntity{};
        int mSize{0};
    public:
        void InsertData(int entity, T component) {
            if (mEntityToIndex.find(entity) != mEntityToIndex.end()) {
                throw std::runtime_error("Component already exists for this entity");
            }

            int index = mSize;
            mComponentArray[index] = component;
            mEntityToIndex[entity] = index;
            mIndexToEntity[index] = entity;
            mSize++;
        }

        void RemoveData(int entity) {
            if (mEntityToIndex.find(entity) == mEntityToIndex.end()) {
                throw std::runtime_error("Component does not exist for this entity");
            }

            // put the last element in the place of the deleted element
            int index = mEntityToIndex[entity];
            int lastIndex = mSize - 1;
            mComponentArray[index] = mComponentArray[lastIndex];
            
            // update the maps
            int lastEntity = mIndexToEntity[lastIndex];
            mEntityToIndex[lastEntity] = index;
            mIndexToEntity[index] = lastEntity;
            mEntityToIndex.erase(entity);
            mIndexToEntity.erase(lastIndex);
            mSize--;
        }
        
        T& GetData(int entity) {
            if (mEntityToIndex.find(entity) == mEntityToIndex.end()) {
                throw std::runtime_error("Component does not exist for this entity");
            }
            return mComponentArray[mEntityToIndex[entity]];
        }

        void EntityDestroyed(int entity) override {
            if (mEntityToIndex.find(entity) != mEntityToIndex.end()) {
                RemoveData(entity);
            }
        }
};

class ComponentManager {
    private:
        std::unordered_map<const char*, int> mComponentTypes{};
        std::unordered_map<const char*, std::shared_ptr<IComponentArray>> mComponentArrays{};
        int mNextComponentType{0};

        template <typename T>
        std::shared_ptr<ComponentArray<T>> getComponentArray() {
            const char* typeName = typeid(T).name();

            if (mComponentTypes.find(typeName) == mComponentTypes.end()) {
                throw std::runtime_error("Component type not registered");
            }
            
            return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[typeName]);
        }

    public:
        template <typename T>
        void registerComponent() {
            const char* typeName = typeid(T).name();

            if (mComponentTypes.find(typeName) != mComponentTypes.end()) {
                throw std::runtime_error("Component type already registered");
            }

            mComponentTypes[typeName] = mNextComponentType;
            mComponentArrays[typeName] = std::make_shared<ComponentArray<T>>();
            mNextComponentType++;
        }

        template <typename T>
        int getComponentType() {
            const char* typeName = typeid(T).name();

            if (mComponentTypes.find(typeName) == mComponentTypes.end()) {
                throw std::runtime_error("Component type not registered");
            }

            return mComponentTypes[typeName];
        }

        template <typename T>
        void addComponent(int entity, T component) {
            const char* typeName = typeid(T).name();

            if (mComponentTypes.find(typeName) == mComponentTypes.end()) {
                throw std::runtime_error("Component type not registered");
            }

            getComponentArray<T>()->InsertData(entity, component);
        }

        template <typename T>
        void removeComponent(int entity) {
            getComponentArray<T>()->RemoveData(entity);
        }

        template <typename T>
        T& getComponent(int entity) {
            return getComponentArray<T>()->GetData(entity);
        }

        void entityDestroyed(int entity) {
            for (auto const& pair : mComponentArrays) {
                pair.second->EntityDestroyed(entity);
            }
        }

};

#endif // COMPONENT_HPP