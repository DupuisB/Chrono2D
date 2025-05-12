#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <set>
#include "Component.hpp"

class System {
    public:
        std::set<int> mEntities;
};

class SystemManager {
    private:
        std::unordered_map<const char*, std::bitset<MAX_COMPONENTS>> mSignatures{};
        std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};

    public:
        template <typename T>
        std::shared_ptr<T> registerSystem() {
            const char* typeName = typeid(T).name();

            if (mSystems.find(typeName) != mSystems.end()) {
                throw std::runtime_error("System already registered");
            }

            auto system = std::make_shared<T>();
            mSystems[typeName] = system;
            return system;
        }

        template <typename T>
        void setSignature(std::bitset<MAX_COMPONENTS> signature) {
            const char* typeName = typeid(T).name();

            if (mSystems.find(typeName) == mSystems.end()) {
                throw std::runtime_error("System not registered");
            }

            mSignatures[typeName] = signature;
        }

        void entityDestroyed(int entity) {
            for (auto const& pair : mSystems) {
                auto const& system = pair.second;
                system->mEntities.erase(entity);
            }
        }

        void entitySignatureChanged(int entity, std::bitset<MAX_COMPONENTS> signature) {
            for (auto const& pair : mSystems) {
                auto const& type = pair.first;
                auto const& system = pair.second;
                auto const& systemSignature = mSignatures[type];
                if ((signature & systemSignature) == systemSignature) {
                    system->mEntities.insert(entity);
                } else {
                    system->mEntities.erase(entity);
                }
            }
        }
};
#endif // SYSTEM_HPP