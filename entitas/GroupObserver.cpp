// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "GroupObserver.hpp"
#include "Group.hpp"
#include <functional>

namespace entitas
{
GroupObserver::GroupObserver(std::shared_ptr<Group> group, const GroupEventType eventType)
{
	groups_.push_back(group);
	eventTypes_.push_back(eventType);
	addEntityCache_ = std::bind(&GroupObserver::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

GroupObserver::GroupObserver(std::vector<std::shared_ptr<Group>> groups, std::vector<GroupEventType> eventTypes)
{
	groups_ = groups;
	eventTypes_ = eventTypes;
	addEntityCache_ = std::bind(&GroupObserver::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	if(groups.size() != eventTypes.size())
	{
		throw std::runtime_error("Error, group and eventType vector counts must be equal");
	}

	activate();
}

GroupObserver::~GroupObserver()
{
	deactivate();
}

void GroupObserver::activate()
{
	for(unsigned int i = 0, groupCount = groups_.size(); i < groupCount; ++i)
	{
		auto g = groups_[i];
		auto eventType = eventTypes_[i];

		if(eventType == GroupEventType::OnEntityAdded)
		{
			g->onEntityAdded -= addEntityCache_;
			g->onEntityAdded += addEntityCache_;
		}
		else if(eventType == GroupEventType::OnEntityRemoved)
		{
			g->onEntityRemoved -= addEntityCache_;
			g->onEntityRemoved += addEntityCache_;
		}
		else if(eventType == GroupEventType::OnEntityAddedOrRemoved)
		{
			g->onEntityAdded -= addEntityCache_;
			g->onEntityAdded += addEntityCache_;

			g->onEntityRemoved -= addEntityCache_;
			g->onEntityRemoved += addEntityCache_;
		}
	}
}

void GroupObserver::deactivate()
{
	for(const auto &g : groups_)
	{
		g->onEntityAdded -= addEntityCache_;
		g->onEntityRemoved -= addEntityCache_;
	}

	clearCollectedEntities();
}

auto GroupObserver::getCollectedEntities() -> std::unordered_set<EntityPtr>
{
	return mCollectedEntities;
}

void GroupObserver::clearCollectedEntities()
{
	mCollectedEntities.clear();
}

void GroupObserver::addEntity(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component)
{
	mCollectedEntities.insert(entity);
}
}
