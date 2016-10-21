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

namespace EntitasPP
{
class Group;

class GroupObserver
{
	public:
		GroupObserver(std::shared_ptr<Group> group, const GroupEventType eventType);
		GroupObserver(std::vector<std::shared_ptr<Group>> groups, std::vector<GroupEventType> eventTypes);
		~GroupObserver();

		void activate();
		void deactivate();
		auto getCollectedEntities() -> std::unordered_set<EntityPtr>;
		void clearCollectedEntities();

	private:
		void addEntity(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component);

		std::unordered_set<EntityPtr> mCollectedEntities;
		std::vector<std::shared_ptr<Group>> mGroups;
		std::vector<GroupEventType> mEventTypes;
		std::function<void(std::shared_ptr<Group>, EntityPtr, ComponentId, IComponent*)> mAddEntityCache;
};
}
