#ifndef KEY_EVENT_SYSTEM_HPP
#define KEY_EVENT_SYSTEM_HPP

#include "../ECS/ECS.hpp"
#include "../Components.hpp"
#include "../utils/math.hpp"

class KeyEventSystem {
private:
    std::shared_ptr<ECS> ecs;
    sf::RenderWindow* window;
    Entity controlledEntity;
    bool& paused;
public:
    KeyEventSystem(std::shared_ptr<ECS> ecs, sf::RenderWindow* window, bool& paused) : ecs(ecs), window(window), paused(paused) {}

    void handleKeyPressedEvent(const sf::Event::KeyPressed* keyEvent) {
        if (!keyEvent) return;
        Velocity& velocity = ecs->getData<Velocity>(controlledEntity);

        if (keyEvent->code == sf::Keyboard::Key::P) {
            paused = !paused;
        } else if (keyEvent->code == sf::Keyboard::Key::Space) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] += Vec2f(0, -25.0f);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::Left) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] += Vec2f(-5.0f, 0);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::Right) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] += Vec2f(5.0f, 0);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::Down) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] += Vec2f(0, 10.0f);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::Up) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] += Vec2f(0, -15.0f);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::R) {
            resetInitialPositions();
        } else if (keyEvent->code == sf::Keyboard::Key::S) {
            for (int i = 0; i < velocity.velocities.size(); i++) {
                velocity.velocities[i] = Vec2f(0, 0);
            }
        } else if (keyEvent->code == sf::Keyboard::Key::LShift) {
            controlNextEntity();
        }
    }

    void resetInitialPositions() {
        for (Entity e = 0; e < MAX_ENTITIES; e++) {
            if (ecs->hasComponent<InitialPosition>(e)) {
                InitialPosition& initialPosition = ecs->getData<InitialPosition>(e);
                Position& position = ecs->getData<Position>(e);
                std::vector<Vec2f>& positions = position.positions;
                std::vector<Vec2f>& initialPositions = initialPosition.initialPositions;
                for (int i = 0; i < positions.size(); i++) {
                    positions[i] = initialPositions[i];
                }
                position.updateCenter();
                Velocity& velocity = ecs->getData<Velocity>(e);
                for (int i = 0; i < velocity.velocities.size(); i++) {
                    velocity.velocities[i] = Vec2f(0, 0);
                }
                Acceleration& acceleration = ecs->getData<Acceleration>(e);
                for (int i = 0; i < acceleration.accelerations.size(); i++) {
                    acceleration.accelerations[i] = Vec2f(0, 0);
                }
                PredictedPosition& predictedPosition = ecs->getData<PredictedPosition>(e);
                for (int i = 0; i < predictedPosition.predictedPositions.size(); i++) {
                    predictedPosition.predictedPositions[i] = initialPositions[i];
                }
            }
        }
    }

    void controlNextEntity() {
        Entity temp = controlledEntity;
        controlledEntity++;
        if (controlledEntity >= MAX_ENTITIES) {
            controlledEntity = 0;
        }
        bool found = false;
        int counter = 0; // to avoid infinite loop in case of no dynamic entities
        while (!found) {
            if (ecs->hasComponent<Mass>(controlledEntity)) {
                Mass& mass = ecs->getData<Mass>(controlledEntity);
                if (mass.m != 0.0f) {
                    found = true;
                    ecs->addComponent<ControlledEntity>(controlledEntity, ControlledEntity());
                    ecs->removeComponent<ControlledEntity>(temp, ControlledEntity());
                    continue;
                }
            }
            controlledEntity++;
            if (controlledEntity >= MAX_ENTITIES) {
                controlledEntity = 0;
            }
            counter++;
            if (counter > MAX_ENTITIES) {
                controlledEntity = temp;
                break;
            }
        }
    }

    void setControlledEntity(Entity entity) {
        controlledEntity = entity;
    }

    void updateControlledEntity() {
        for (Entity e = 0; e < MAX_ENTITIES; e++) {
            if (ecs->hasComponent<ControlledEntity>(e)) {
                controlledEntity = e;
                return;
            }
        }
    }

    Entity getControlledEntity() {
        return controlledEntity;
    }
};

#endif // KEY_EVENT_SYSTEM_HPP