#include "ECS.h"

void Entity::addGroup(Group mGroup) {
    if (mGroup < maxGroups) { 
        groupBitset[mGroup] = true;
        manager.AddToGroup(this, mGroup); 
    } else {

        std::cerr << "Error: Attempted to add entity to invalid group index: " << mGroup << std::endl;
    }
}