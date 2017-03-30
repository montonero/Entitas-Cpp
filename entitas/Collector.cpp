// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Collector.hpp"
#include "Group.hpp"
#include <functional>

namespace entitas {

size_t Indexed::s_index = 0;

Collector::Collector(Group::SharedPtr group, const GroupEventType eventType)
    : Collector(std::vector<Group::SharedPtr>{ group }, { eventType })
{
}

/// This is currently used by Reactive System
Collector::Collector(std::vector<Group::SharedPtr>&& groups, std::vector<GroupEventType>&& eventTypes)
    : groups_{ groups }
    , eventTypes_{ eventTypes }
{
    addEntityCache_ = std::bind(&Collector::addEntity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4); // sizeof find out

    if (groups.size() != eventTypes.size()) {
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
    for (unsigned int i = 0, groupCount = groups_.size(); i < groupCount; ++i) {
        auto& g = groups_[i];
        auto eventType = eventTypes_[i];

        if (eventType == GroupEventType::Added) {
            g->onEntityAdded -= { index(), addEntityCache_ };
            g->onEntityAdded += { index(), addEntityCache_ };
        } else if (eventType == GroupEventType::Removed) {
            g->onEntityRemoved -= { index(), addEntityCache_ };
            g->onEntityRemoved += { index(), addEntityCache_ };
        } else if (eventType == GroupEventType::AddedOrRemoved) {
            g->onEntityAdded -= { index(), addEntityCache_ };
            g->onEntityAdded += { index(), addEntityCache_ };

            g->onEntityRemoved -= { index(), addEntityCache_ };
            g->onEntityRemoved += { index(), addEntityCache_ };
        }
    }
}

void Collector::deactivate()
{
    for (const auto& g : groups_) {
        g->onEntityAdded -= { index(), addEntityCache_ };
        g->onEntityRemoved -= { index(), addEntityCache_ };
    }

    clearCollectedEntities();
}

const Collector::CollectedEntities& Collector::getCollectedEntities() const
{
    return collectedEntities_;
}

void Collector::clearCollectedEntities()
{
    collectedEntities_.clear();
}

void Collector::addEntity(Group::SharedPtr group, EntityPtr entity, ComponentId index, IComponent* component)
{
    collectedEntities_.insert(entity);
}
}
