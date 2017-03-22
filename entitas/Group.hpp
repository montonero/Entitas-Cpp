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
    //using GroupPtr = std::shared_ptr<Group>;
    
class Group {
    friend class Pool;

public:
    using SharedPtr = std::shared_ptr<Group>;
    Group(const Matcher& matcher);
    auto count() const -> unsigned int;
    auto getEntities() -> std::vector<EntityPtr>;
    auto getSingleEntity() const -> EntityPtr;
    bool containsEntity(const EntityPtr& entity) const;
    auto getMatcher() const -> Matcher;
    auto createCollector(const GroupEventType eventType) -> std::shared_ptr<Collector>;

    using GroupChanged = Delegate<void(SharedPtr group, EntityPtr entity, ComponentId index, IComponent* component)>;
    using GroupUpdated = Delegate<void(SharedPtr group, EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)>;

    GroupChanged onEntityAdded;
    GroupUpdated onEntityUpdated;
    GroupChanged onEntityRemoved;

protected:
    void setInstance(SharedPtr instance);
    auto handleEntity(EntityPtr entity) -> GroupChanged*;
    void handleEntitySilently(EntityPtr entity);
    void handleEntity(EntityPtr entity, ComponentId index, IComponent* component);
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
    std::vector<EntityPtr> entitiesCache_;
};
}
