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

inline ComponentID getNewComponentTypeID() {
    static ComponentID lastID = 0u;
    return lastID++;
}

template <typename T> inline ComponentID getComponentTypeID() noexcept {
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
    bool isDestroyed = false; // Add this flag
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
            // std::cout << "Entity Destroyed!" << std::endl; // Add this log
        } else {
            // std::cout << "Entity already destroyed!" << std::endl; // Add this log
        }
    }

    bool hasGroup(Group mGroup) {
        return groupBitset[mGroup];
    }
    void addGroup(Group mGroup);
    void delGroup(Group mGroup) {
        groupBitset[mGroup] = false;
    }

    template <typename T> bool hasComponent() const {
        return componentBitSet[getComponentTypeID<T>()];
    }
    
    template <typename T, typename... TArgs>
    T& addComponent(TArgs&&... mArgs) {
        T* c(new T(std::forward<TArgs>(mArgs)...));
        c->entity = this;
        std::unique_ptr<Component> uPtr{ c };
        components.emplace_back(std::move(uPtr));

        componentArray[getComponentTypeID<T>()] = c;
        componentBitSet[getComponentTypeID<T>()] = true;

        c->init();
        return *c;
    }
    
    template <typename T> T& getComponent() const {
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
                // Check if unique_ptr itself is valid before calling update
                if (e) {
                    e->update();
                }
            }
        }
        void draw() {
             for (auto& e : entities) {
                 // Check if unique_ptr itself is valid before calling draw
                 if (e) {
                     e->draw();
                 }
             }
        }

        void refresh() {
            // --- Refresh main entities list FIRST ---
            // This removes inactive entities AND deletes the Entity objects via unique_ptr destructor.
            entities.erase(
                std::remove_if(std::begin(entities), std::end(entities),
                    [](const std::unique_ptr<Entity> &mEntity) {
                        // Condition checks if unique_ptr is null OR entity is inactive
                        return !mEntity || !mEntity->isActive();
                    }),
                std::end(entities)
            );

            // --- THEN, Refresh grouped entities (using raw pointers) ---
            // Now iterate through groups and remove pointers to entities that are
            // no longer active OR no longer belong to the group.
            // Since the owning unique_ptrs might have been deleted above,
            // checking isActive() here on a raw pointer IS DANGEROUS if the entity was deleted.
            // The check should primarily be based on whether the entity *still exists*
            // in the main `entities` list (which is complex) or better, rely on
            // components adding/removing themselves from groups correctly when needed.
            // A simpler, often sufficient approach if group management is reliable:
            // Only remove pointers if the entity is no longer in that specific group.
            // Entities that became inactive were already deleted above. We just need
            // to remove the now-dangling pointers or pointers to entities moved out of the group.
            // SAFER APPROACH: Rebuild groupedEntities from scratch based on active entities.

            // --- SAFEST REFRESH LOGIC: Rebuild groupedEntities ---
            for (auto& v : groupedEntities) {
                v.clear(); // Clear all existing (potentially dangling) raw pointers
            }
            // Repopulate groups based on currently active entities in the main list
            for (const auto& entityPtr : entities) {
                 if (entityPtr && entityPtr->isActive()) { // Check unique_ptr and entity status
                     for (Group group = 0; group < maxGroups; ++group) {
                         if (entityPtr->hasGroup(group)) {
                             groupedEntities[group].push_back(entityPtr.get()); // Add raw pointer back
                         }
                     }
                 }
            }
            // --- End SAFEST REFRESH LOGIC ---


            /* --- ORIGINAL groupedEntities Refresh Logic (Potentially Unsafe) ---
               This logic is prone to crashes if an entity was deleted in the
               entities.erase() step above, because mEntity would be a dangling pointer.

            for (auto i(0u); i < maxGroups; i++) {
                auto& v(groupedEntities[i]); // v is std::vector<Entity*>&

                v.erase(
                    std::remove_if(std::begin(v), std::end(v), [i](Entity* mEntity) {
                        // DANGER: mEntity might be dangling pointer here!
                        // Checking isActive() or hasGroup() can crash.
                         return !mEntity || !mEntity->isActive() || !mEntity->hasGroup(i);
                    }),
                    std::end(v)
                );
            }
            */

        } // End refresh()

        void AddToGroup(Entity* mEntity, Group mGroup) {
            // Check if entity is already in the group vector to prevent duplicates
             auto& groupVec = groupedEntities[mGroup];
             if (std::find(groupVec.begin(), groupVec.end(), mEntity) == groupVec.end()) {
                 groupVec.emplace_back(mEntity);
             }
        }
        std::vector<Entity*>& getGroup(Group mGroup) {
            // Bounds check might be useful here if maxGroups is dynamic
            return groupedEntities[mGroup];
        }

        Entity& addEntity() {
            Entity* e = new Entity(*this);
            std::unique_ptr<Entity> uPtr{ e };
            entities.emplace_back(std::move(uPtr));
            return *e;
        }
}; // End Manager class
