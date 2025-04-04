#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;
class Manager;

using ComponentID = std::size_t;
using Group = std::size_t;

// --- Component ID Management ---
inline ComponentID getNewComponentTypeID() {
    static ComponentID lastID = 0u;
    return lastID++;
}

template <typename T> inline ComponentID getComponentTypeID() noexcept {
    static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
    static ComponentID typeID = getNewComponentTypeID();
    return typeID;
}

// --- Limits ---
constexpr std::size_t maxComponents = 32;
constexpr std::size_t maxGroups = 32;

// --- Type Aliases ---
using ComponentBitSet = std::bitset<maxComponents>;
using GroupBitset = std::bitset<maxGroups>;
using ComponentArray = std::array<Component*, maxComponents>; // Raw pointer for fast access

// --- Base Component Class ---
class Component {
public:
    Entity* entity = nullptr; // Pointer back to the owner entity

    virtual void init() {}
    virtual void update() {}
    virtual void draw() {}

    virtual ~Component() = default; // Default virtual destructor
};

// --- Entity Class ---
class Entity {
private:
    Manager& manager; // Reference to the manager
    bool active = true;
    std::vector<std::unique_ptr<Component>> components; // Owning pointers

    ComponentArray componentArray{}; // Initialize to nullptrs
    ComponentBitSet componentBitSet{};
    GroupBitset groupBitset{};

public:
    Entity(Manager& mManager) : manager(mManager) {}

    void update() {
        if (!active) return;
        for (auto& c : components) c->update();
    }

    void draw() {
        if (!active) return;
        for (auto& c : components) c->draw();
    }

    bool isActive() const { return active; }

    void destroy() {
        active = false; // Mark as inactive for removal by Manager
    }

    bool hasGroup(Group mGroup) const {
        return groupBitset[mGroup];
    }

    void addGroup(Group mGroup); // Implementation in ECS.cpp

    void delGroup(Group mGroup) {
        groupBitset[mGroup] = false;
        // Manager should handle removing from group vector during refresh
    }

    template <typename T> bool hasComponent() const {
        return componentBitSet[getComponentTypeID<T>()];
    }

    template <typename T, typename... TArgs>
    T& addComponent(TArgs&&... mArgs) {
        // Ensure component type T derives from Component
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        ComponentID cID = getComponentTypeID<T>();
        if (componentBitSet[cID]) {
             // Optionally return existing component or log warning/error
             // For now, returning existing:
             return *static_cast<T*>(componentArray[cID]);
        }

        T* c = new T(std::forward<TArgs>(mArgs)...);
        c->entity = this;
        std::unique_ptr<Component> uPtr{ c };
        components.emplace_back(std::move(uPtr));

        componentArray[cID] = c;
        componentBitSet[cID] = true;

        c->init(); // Initialize the component after adding
        return *c;
    }

    template <typename T> T& getComponent() const {
        // Ensure component type T derives from Component
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

        auto ptr(componentArray[getComponentTypeID<T>()]);
        if (!ptr) {
            throw std::runtime_error("Attempted to get non-existent component.");
        }
        return *static_cast<T*>(ptr);
    }

    const std::vector<std::unique_ptr<Component>>& getAllComponents() const {
        return components;
    }
};

// --- Manager Class ---
class Manager {
private:
    std::vector<std::unique_ptr<Entity>> entities; // Owning pointers
    std::array<std::vector<Entity*>, maxGroups> groupedEntities; // Non-owning pointers for fast group access

public:
    void update() {
        for (auto& e : entities) {
            if (e && e->isActive()) { // Check unique_ptr and entity status
                e->update();
            }
        }
    }

    void draw() {
        for (auto& e : entities) {
            if (e && e->isActive()) { // Check unique_ptr and entity status
                e->draw();
            }
        }
    }

    void refresh() {
        // Remove inactive entities from the main list
        entities.erase(
            std::remove_if(std::begin(entities), std::end(entities),
                [](const std::unique_ptr<Entity>& mEntity) {
                    return !mEntity || !mEntity->isActive();
                }),
            std::end(entities)
        );

        // Refresh group lists (safer approach: rebuild)
        for (auto& v : groupedEntities) {
            v.clear();
        }
        for (const auto& entityPtr : entities) {
            if (entityPtr && entityPtr->isActive()) { // Ensure entity is valid and active
                for (Group group = 0; group < maxGroups; ++group) {
                    if (entityPtr->hasGroup(group)) {
                        groupedEntities[group].push_back(entityPtr.get());
                    }
                }
            }
        }
    }

    void AddToGroup(Entity* mEntity, Group mGroup) {
        if (mGroup < maxGroups) {
            auto& groupVec = groupedEntities[mGroup];
            // Prevent duplicates if AddToGroup is called multiple times
            if (std::find(groupVec.begin(), groupVec.end(), mEntity) == groupVec.end()) {
                groupVec.emplace_back(mEntity);
            }
        }
    }

    std::vector<Entity*>& getGroup(Group mGroup) {
        if (mGroup >= maxGroups) {
            throw std::out_of_range("Group index out of range.");
        }
        return groupedEntities[mGroup];
    }

    Entity& addEntity() {
        Entity* e = new Entity(*this);
        std::unique_ptr<Entity> uPtr{ e };
        entities.emplace_back(std::move(uPtr));
        return *e;
    }
}; // End Manager class