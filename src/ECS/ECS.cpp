#include "ECS.h"

// --- Entity Method Implementations ---

void Entity::addGroup(Group mGroup) {
    if (mGroup < maxGroups) { // Check bounds
        groupBitset[mGroup] = true;
        manager.AddToGroup(this, mGroup); // Notify manager
    } else {
        // Handle error: group index out of bounds
        std::cerr << "Error: Attempted to add entity to invalid group index: " << mGroup << std::endl;
    }
}
