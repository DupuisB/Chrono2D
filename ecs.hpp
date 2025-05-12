#ifndef ECS_HPP
#define ECS_HPP

#include <memory>

#include "System.hpp"
#include "Entity.hpp"
#include "Component.hpp"

class ECS {
    private:
        std::unique_ptr<EntityManager> mEntityManager;
        std::unique_ptr<ComponentManager> mComponentManager;
        std::unique_ptr<SystemManager> mSystemManager;

    public:
        ECS() {
            mEntityManager = std::make_unique<EntityManager>();
            mComponentManager = std::make_unique<ComponentManager>();
            mSystemManager = std::make_unique<SystemManager>();
        }

        ~ECS() = default;

        int createEntity() {
            return mEntityManager->createEntity();
        }

        void destroyEntity(int entity) {
            mEntityManager->destroyEntity(entity);
            mComponentManager->entityDestroyed(entity);
            mSystemManager->entityDestroyed(entity);
        }

        template <typename T>
        void registerComponent() {
            mComponentManager->registerComponent<T>();
        }

        template <typename T>
        void addComponent(int entity, T component) {
            mComponentManager->addComponent(entity, component);
            mEntityManager->setSignature(entity, mComponentManager->getComponentType<T>());
            mSystemManager->entitySignatureChanged(entity, mEntityManager->getSignature(entity));
        }

        template <typename T>
        void removeComponent(int entity) {
            mComponentManager->removeComponent<T>(entity);
            auto signature = mEntityManager->getSignature(entity);
            signature.set(mComponentManager->getComponentType<T>(), false);
            mEntityManager->setSignature(entity, signature);
            mSystemManager->entitySignatureChanged(entity, signature);
        }

        template <typename T>
        T& getComponent(int entity) {
            return mComponentManager->getComponent<T>(entity);
        }

        template <typename T>
        int getComponentType() {
            return mComponentManager->getComponentType<T>();
        }

        template <typename T>
        std::shared_ptr<T> registerSystem() {
            return mSystemManager->registerSystem<T>();
        }

        template <typename T>
        void setSystemSignature(std::bitset<MAX_COMPONENTS> signature) {
            mSystemManager->setSignature<T>(signature);
        }
};

#endif // ECS_HPP