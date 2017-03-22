// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ComponentTypeId.hpp"
#include "Entity.hpp"
#include "Group.hpp"
#include "GroupEventType.hpp"
#include <functional>
#include <unordered_set>
#include <vector>

namespace entitas {

class Indexed {
public:
    Indexed()
        : index_(s_index++)
    {
    }
    size_t index() const { return index_; }

protected:
    const size_t index_;
    static size_t s_index;
};

/*
      The Entity Collector provides an easy way to react to changes in a group over time. Let's say you want to collect and process all the entities where a PositionComponent was added or replaced.

      auto group = pool->GetGroup(Matcher_AllOf(Position));
      auto collector = group->CreateCollector(GroupEventType::OnEntityAdded);
*/

/// A Collector can observe one or more groups from the same context
/// and collects changed entities based on the specified groupEvent.
class Collector : public Indexed {
public:
    using CollectedEntities = std::unordered_set<EntityPtr>;
    /// Creates a Collector and will collect changed entities
    /// based on the specified eventType.
    Collector(Group::SharedPtr group, const GroupEventType eventType);

    /// Creates a Collector and will collect changed entities
    /// based on the specified eventTypes.
    Collector(std::vector<Group::SharedPtr> groups, std::vector<GroupEventType> eventTypes);
    ~Collector();

    void activate();
    void deactivate();

    /// Returns all collected entities.
    /// Call collector.ClearCollectedEntities()
    /// once you processed all entities.
    const CollectedEntities& getCollectedEntities() const;
    const CollectedEntities& collectedEntities() const { return getCollectedEntities(); }

    void clearCollectedEntities();

private:
    void addEntity(Group::SharedPtr group, EntityPtr entity, ComponentId index, IComponent* component);
    /// We store collected entities here
    CollectedEntities collectedEntities_;
    std::vector<Group::SharedPtr> groups_;
    std::vector<GroupEventType> eventTypes_;
    /// This is a callback that will be called by group and will save changes in 'collectedEntities_'
    std::function<void(Group::SharedPtr, EntityPtr, ComponentId, IComponent*)> addEntityCache_;
};
}
