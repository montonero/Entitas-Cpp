// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Collector.hpp"
#include "Group.hpp"
#include <functional>

namespace entitas
{
Collector::Collector(std::shared_ptr<Group> group, const GroupEventType eventType)
{
	groups_.push_back(group);
	eventTypes_.push_back(eventType);
	addEntityCache_ = std::bind(&Collector::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

Collector::Collector(std::vector<std::shared_ptr<Group>> groups, std::vector<GroupEventType> eventTypes)
{
	groups_ = groups;
	eventTypes_ = eventTypes;
	addEntityCache_ = std::bind(&Collector::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	if(groups.size() != eventTypes.size())
	{
		throw std::runtime_error("Error, group and eventType vector counts must be equal");
	}

	activate();
}

Collector::~Collector()
{
	deactivate();
}

void Collector::activate()
{
	for(unsigned int i = 0, groupCount = groups_.size(); i < groupCount; ++i)
	{
		auto& g = groups_[i];
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

void Collector::deactivate()
{
	for(const auto &g : groups_)
	{
		g->onEntityAdded -= addEntityCache_;
		g->onEntityRemoved -= addEntityCache_;
	}

	clearCollectedEntities();
}

Collector::CollectedEntities Collector::getCollectedEntities() {
	return collectedEntities_;
}

void Collector::clearCollectedEntities()
{
	collectedEntities_.clear();
}

void Collector::addEntity(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component)
{
	collectedEntities_.insert(entity);
}
}
