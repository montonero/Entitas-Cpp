// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "GroupEventType.hpp"
#include "Matcher.hpp"
#include <unordered_set>

namespace entitas {
class Collector;

using Entities = std::vector<EntityPtr>;
/// Use context.GetGroup(matcher) to get a group of entities which match
/// the specified matcher. Calling context.GetGroup(matcher) with the
/// same matcher will always return the same instance of the group.
/// The created group is managed by the context and will always be up to date.
/// It will automatically add entities that match the matcher or
/// remove entities as soon as they don't match the matcher anymore.
class Group {
    friend class Pool;

public:
    using SharedPtr = std::shared_ptr<Group>;
    Group(const Matcher& matcher);
    auto count() const -> unsigned int;

    /// Returns all entities which are currently in this group.
    Entities& getEntities();
    
    /// Returns the only entity in this group. It will return nullptr
    /// if the group is empty. It will throw an exception if the group
    /// has more than one entity.
    auto getSingleEntity() const -> EntityPtr;
    
    bool containsEntity(const EntityPtr& entity) const;
    auto getMatcher() const -> Matcher;
    auto createCollector(const GroupEventType eventType) -> std::shared_ptr<Collector>;

    using GroupChanged = Delegate<void(SharedPtr group, EntityPtr entity, ComponentId index, IComponent* component)>;
    using GroupUpdated = Delegate<void(SharedPtr group, EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)>;

    /// Occurs when an entity gets added.
    GroupChanged onEntityAdded;
    /// Occurs when a component of an entity in the group gets replaced.
    GroupUpdated onEntityUpdated;
    /// Occurs when an entity gets removed.
    GroupChanged onEntityRemoved;

protected:
    void setInstance(SharedPtr instance);
    // Returns callback or nullptr if entity was not added nor removed
    auto handleEntity(EntityPtr entity) -> GroupChanged*;
    // Does not call callback
    void handleEntitySilently(EntityPtr entity);
    void handleEntity(EntityPtr entity, ComponentId index, IComponent* component);
    /// Called by context/pool
    void updateEntity(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent);
    void removeAllEventHandlers();

private:
    bool addEntitySilently(EntityPtr entity); ///< Returns true if a given entity was added
    void addEntity(EntityPtr entity, ComponentId index, IComponent* component);
    auto addEntity(EntityPtr entity) -> GroupChanged*;
    bool removeEntitySilently(EntityPtr entity); ///< Returns true if a given entity was removed
    void removeEntity(EntityPtr entity, ComponentId index, IComponent* component);
    auto removeEntity(EntityPtr entity) -> GroupChanged*;

    std::weak_ptr<Group> instance_;
    Matcher matcher_;
    std::unordered_set<EntityPtr> entities_;
    Entities entitiesCache_;
};
}
