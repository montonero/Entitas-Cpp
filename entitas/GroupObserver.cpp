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
	mGroups.push_back(group);
	mEventTypes.push_back(eventType);
	mAddEntityCache = std::bind(&GroupObserver::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

GroupObserver::GroupObserver(std::vector<std::shared_ptr<Group>> groups, std::vector<GroupEventType> eventTypes)
{
	mGroups = groups;
	mEventTypes = eventTypes;
	mAddEntityCache = std::bind(&GroupObserver::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

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
	for(unsigned int i = 0, groupCount = mGroups.size(); i < groupCount; ++i)
	{
		auto g = mGroups[i];
		auto eventType = mEventTypes[i];

		if(eventType == GroupEventType::OnEntityAdded)
		{
			g->onEntityAdded -= mAddEntityCache;
			g->onEntityAdded += mAddEntityCache;
		}
		else if(eventType == GroupEventType::OnEntityRemoved)
		{
			g->onEntityRemoved -= mAddEntityCache;
			g->onEntityRemoved += mAddEntityCache;
		}
		else if(eventType == GroupEventType::OnEntityAddedOrRemoved)
		{
			g->onEntityAdded -= mAddEntityCache;
			g->onEntityAdded += mAddEntityCache;

			g->onEntityRemoved -= mAddEntityCache;
			g->onEntityRemoved += mAddEntityCache;
		}
	}
}

void GroupObserver::deactivate()
{
	for(const auto &g : mGroups)
	{
		g->onEntityAdded -= mAddEntityCache;
		g->onEntityRemoved -= mAddEntityCache;
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
