#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <iostream>
#include <memory>
#include <vector>

class Component;
class Entity;
class Manager;

using ComponentID = std::size_t;
using Group = std::size_t;

inline ComponentID getNewComponentTypeID() {
    static ComponentID lastID = 0u;
    return lastID++;
}

template <typename T>
inline ComponentID getComponentTypeID() noexcept {
    static_assert(std::is_base_of<Component, T>::value, "");
    static ComponentID typeID = getNewComponentTypeID();
    return typeID;
}

constexpr std::size_t maxComponents = 32;
constexpr std::size_t maxGroups = 32;

using ComponentBitSet = std::bitset<maxComponents>;
using GroupBitset = std::bitset<maxGroups>;
using ComponentArray = std::array<Component*, maxComponents>;

class Component {
   public:
    Entity* entity;

    virtual void init() {}
    virtual void update() {}
    virtual void draw() {}

    virtual ~Component() {}
};

class Entity {
   private:
    Manager& manager;
    bool active = true;
    std::vector<std::unique_ptr<Component>> components;

    ComponentArray componentArray;
    ComponentBitSet componentBitSet;
    GroupBitset groupBitset;

   public:
    Entity(Manager& mManager) : manager(mManager) {}
    bool isDestroyed = false;
    const std::vector<std::unique_ptr<Component>>& getAllComponents() const {
        return components;
    }
    void update() {
        for (auto& c : components) c->update();
    }
    void draw() {
        for (auto& c : components) c->draw();
    }
    bool isActive() const { return active; }
    void destroy() {
        if (!isDestroyed) {
            active = false;

        } else {
        }
    }

    bool hasGroup(Group mGroup) { return groupBitset[mGroup]; }
    void addGroup(Group mGroup);
    void delGroup(Group mGroup) { groupBitset[mGroup] = false; }

    template <typename T>
    bool hasComponent() const {
        return componentBitSet[getComponentTypeID<T>()];
    }

    template <typename T, typename... TArgs>
    T& addComponent(TArgs&&... mArgs) {
        T* c(new T(std::forward<TArgs>(mArgs)...));
        c->entity = this;
        std::unique_ptr<Component> uPtr{c};
        components.emplace_back(std::move(uPtr));

        componentArray[getComponentTypeID<T>()] = c;
        componentBitSet[getComponentTypeID<T>()] = true;

        c->init();
        return *c;
    }

    template <typename T>
    T& getComponent() const {
        auto ptr(componentArray[getComponentTypeID<T>()]);
        return *static_cast<T*>(ptr);
    }
};
class Manager {
   private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::array<std::vector<Entity*>, maxGroups> groupedEntities;

   public:
    void update() {
        for (auto& e : entities) {
            if (e) {
                e->update();
            }
        }
    }
    void draw() {
        for (auto& e : entities) {
            if (e) {
                e->draw();
            }
        }
    }

    void refresh() {
        entities.erase(
            std::remove_if(std::begin(entities), std::end(entities),
                           [](const std::unique_ptr<Entity>& mEntity) {
                               return !mEntity || !mEntity->isActive();
                           }),
            std::end(entities));

        for (auto& v : groupedEntities) {
            v.clear();
        }

        for (const auto& entityPtr : entities) {
            if (entityPtr && entityPtr->isActive()) {
                for (Group group = 0; group < maxGroups; ++group) {
                    if (entityPtr->hasGroup(group)) {
                        groupedEntities[group].push_back(entityPtr.get());
                    }
                }
            }
        }
    }

    void AddToGroup(Entity* mEntity, Group mGroup) {
        auto& groupVec = groupedEntities[mGroup];
        if (std::find(groupVec.begin(), groupVec.end(), mEntity) ==
            groupVec.end()) {
            groupVec.emplace_back(mEntity);
        }
    }
    std::vector<Entity*>& getGroup(Group mGroup) {
        return groupedEntities[mGroup];
    }

    Entity& addEntity() {
        Entity* e = new Entity(*this);
        std::unique_ptr<Entity> uPtr{e};
        entities.emplace_back(std::move(uPtr));
        return *e;
    }
};