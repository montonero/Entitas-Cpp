// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ComponentTypeId.hpp"
#include "Entity.hpp"
#include "GroupEventType.hpp"
#include <vector>
#include <unordered_set>
#include <functional>

namespace entitas
{
class Group;

/// A Collector can observe one or more groups from the same context
/// and collects changed entities based on the specified groupEvent.
class Collector
{
	public:
        using CollectedEntities = std::unordered_set<EntityPtr>;
        /// Creates a Collector and will collect changed entities
        /// based on the specified eventType.
		Collector(std::shared_ptr<Group> group, const GroupEventType eventType);
    
        /// Creates a Collector and will collect changed entities
        /// based on the specified eventTypes.
		Collector(std::vector<std::shared_ptr<Group>> groups, std::vector<GroupEventType> eventTypes);
		~Collector();

		void activate();
		void deactivate();
    
        /// Returns all collected entities.
        /// Call collector.ClearCollectedEntities()
        /// once you processed all entities.
		CollectedEntities getCollectedEntities();
        CollectedEntities collectedEntities() { return getCollectedEntities(); }
    
		void clearCollectedEntities();

	private:
		void addEntity(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component);

		CollectedEntities collectedEntities_;
		std::vector<std::shared_ptr<Group>> groups_;
		std::vector<GroupEventType> eventTypes_;
		std::function<void(std::shared_ptr<Group>, EntityPtr, ComponentId, IComponent*)> addEntityCache_;
};
}
