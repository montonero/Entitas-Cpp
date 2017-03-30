// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Pool.hpp"
#include "Entity.hpp"
#include "Functional.hpp"
#include "ISystem.hpp"
#include "ReactiveSystem.hpp"
#include <algorithm>
#include <assert.h>
#include <utility>

namespace entitas {
Pool::Pool(const unsigned int startCreationIndex)
{
    creationIndex_ = startCreationIndex;
    onEntityReleasedCache_ = std::bind(&Pool::onEntityReleased, this, std::placeholders::_1);
}

Pool::~Pool()
{
    reset();

    // TODO: Components don't get destroyed.

    if (!retainedEntities_.empty()) {
        // Warning, some entities remain undestroyed in the pool destruction !"
    }

    while (!reusableEntities_.empty()) {
        delete reusableEntities_.top();
        reusableEntities_.pop();
    }

    for (auto& pair : componentPools_) {
        auto componentPool = pair.second;

        while (!componentPool.empty()) {
            delete componentPool.top();
            componentPool.pop();
        }
    }
}
/// Creates a new entity or gets a reusable entity from the
/// internal ObjectPool for entities.
auto Pool::createEntity() -> EntityPtr
{
    EntityPtr entity;

    if (reusableEntities_.size() > 0) {
        entity = EntityPtr(reusableEntities_.top(), [](Entity* entity) {
            entity->onReleased(entity);
        });
        reusableEntities_.pop();
    } else {
        entity = EntityPtr(new Entity(componentPools_), [](Entity* entity) {
            entity->onReleased(entity);
        });
    }

    entity->setInstance(entity);
    entity->enabled_ = true;
    entity->uuid_ = creationIndex_++;

    entities_.insert(entity);
    entitiesCache_.clear();
    // TODO This can be optimized by making less generic since 'onComponentAdded' will have usually only 1 callback
    entity->onComponentAdded += { entity->uuid_,
        [this](EntityPtr entity, ComponentId index, IComponent* component) {
            updateGroupsComponentAddedOrRemoved(entity, index, component);
        } };
    entity->onComponentRemoved += { entity->uuid_,
        [this](EntityPtr entity, ComponentId index, IComponent* component) {
            updateGroupsComponentAddedOrRemoved(entity, index, component);
        } };
    entity->onComponentReplaced += { entity->uuid_,
        [this](EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent) {
            updateGroupsComponentReplaced(entity, index, previousComponent, newComponent);
        } };

    entity->onReleased.clear();
    entity->onReleased += { entity->uuid_, onEntityReleasedCache_ };

    onEntityCreated(this, entity);

    assert(hasEntity(entity));
    return entity;
}

bool Pool::hasEntity(const EntityPtr& entity) const
{
    //return std::find(entities_.begin(), entities_.end(), entity) != entities_.end();
    return doesExist(entities_, entity);
}

void Pool::destroyEntity(EntityPtr entity)
{
    auto removed = entities_.erase(entity);

    if (!removed) {
        throw std::runtime_error("Error, cannot destroy entity. Pool does not contain entity.");
    }

    entitiesCache_.clear();

    onEntityWillBeDestroyed(this, entity);
    entity->destroy();
    onEntityDestroyed(this, entity);

    if (entity.use_count() == 1) {
        entity->onReleased -= { entity->uuid_, onEntityReleasedCache_ };
        reusableEntities_.push(entity.get());
    } else {
        retainedEntities_.insert(entity.get());
    }
}

void Pool::destroyAllEntities()
{
    {
        auto entitiesTemp = Entities(entities_.begin(), entities_.end());

        while (!entities_.empty()) {
            destroyEntity(entitiesTemp.back());
            entitiesTemp.pop_back();
        }
    }
    // This should be empty by now
    entities_.clear();

    if (!retainedEntities_.empty()) {
        // Try calling Pool.clearGroups() and SystemContainer.clearReactiveSystems() before calling pool.destroyAllEntities() to avoid memory leaks
        //throw std::runtime_error("Error, pool detected retained entities although all entities got destroyed. Did you release all entities?");
    }
}

Entities& Pool::getEntities()
{
    if (entitiesCache_.empty()) {
        entitiesCache_ = Entities(entities_.begin(), entities_.end());
    }
    return entitiesCache_;
}

Entities& Pool::getEntities(const Matcher matcher)
{
    return getGroup(matcher)->getEntities();
}

auto Pool::getGroup(Matcher matcher) -> Group::SharedPtr
{
    Group::SharedPtr group;
    auto it = groups_.find(matcher);
    if (it == groups_.end()) {
        group.reset(new Group(matcher));
        group->setInstance(group);

        // 'Handle' all entities that are already in a pool
        // Thus if the group is created later it will still be able to 'handle'
        // previously created entities
        auto& entities = getEntities();
        for_each(entities,
            [=, &group](auto& e) { group->handleEntitySilently(e); });

        groups_[group->getMatcher()] = group;

        for_each(matcher.getIndices(),
            [&, this](auto index) { groupsForIndex_[index].push_back(group); });

        onGroupCreated(this, group);
    } else {
        group = it->second;
    }

    return group;
}

void Pool::clearGroups()
{
    for (const auto& it : groups_) {
        it.second->removeAllEventHandlers();
        onGroupCleared(this, it.second);
    }

    groups_.clear();

    for (auto& pair : groupsForIndex_) {
        pair.second.clear();
    }

    groupsForIndex_.clear();
}

void Pool::resetCreationIndex()
{
    creationIndex_ = kStartCreationIndex;
}

void Pool::clearComponentPool(const ComponentId index)
{
    while (!componentPools_.at(index).empty()) {
        componentPools_.at(index).pop();
    }
}

void Pool::clearComponentPools()
{
    for_each(componentPools_,
        [this](const auto& pair) { clearComponentPool(pair.first); });
}

void Pool::reset()
{
    clearGroups();
    destroyAllEntities();
    resetCreationIndex();
}

auto Pool::count() const -> unsigned int
{
    return entities_.size();
}

auto Pool::getReusableEntitiesCount() const -> unsigned int
{
    return reusableEntities_.size();
}

auto Pool::getRetainedEntitiesCount() const -> unsigned int
{
    return retainedEntities_.size();
}

auto Pool::createSystem(std::shared_ptr<ISystem> system) -> std::shared_ptr<ISystem>
{
    using std::dynamic_pointer_cast;
    if (auto systemSetPool = dynamic_pointer_cast<ISetPoolSystem>(system)) {
        systemSetPool->setPool(this);
    }

    if (auto systemReactive = dynamic_pointer_cast<IReactiveSystem>(system)) {
        // given system is used as a subsystem
        return std::make_shared<ReactiveSystem>(this, systemReactive);
    }

    if (std::dynamic_pointer_cast<IMultiReactiveSystem>(system) != nullptr) {
        return std::shared_ptr<ReactiveSystem>(new ReactiveSystem(this, std::dynamic_pointer_cast<IMultiReactiveSystem>(system)));
    }

    return system;
}

void Pool::updateGroupsComponentAddedOrRemoved(EntityPtr entity, ComponentId index, IComponent* component)
{
    if (groupsForIndex_.find(index) == groupsForIndex_.end()) {
        return;
    }

    // All groups that contain entities with a given component
    auto& groups = groupsForIndex_[index];

    if (!groups.empty()) {
        // Collect all the events that need to be processed (e.g. onAdded
        std::vector<Group::GroupChanged*> events;
        for (int i = 0, groupsCount = groups.size(); i < groupsCount; ++i) {
            events.push_back(groups[i].lock()->handleEntity(entity));
        }

        for (int i = 0, eventsCount = events.size(); i < eventsCount; ++i) {
            auto cb = events[i];
            if (cb)
                (*cb)(groups[i].lock(), entity, index, component);
        }

#if 0
        for_each(groups,
            [=](const auto& g) {
                auto groupChangedEvent = g.lock()->handleEntity(entity);
                if (groupChangedEvent)
                    (*groupChangedEvent)(g.lock(), entity, index, component);
            });
#endif
    }
}

void Pool::updateGroupsComponentReplaced(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)
{
    if (groupsForIndex_.find(index) == groupsForIndex_.end()) {
        return;
    }
    //using namespace std;
    auto& groups = groupsForIndex_[index];
    for_each(groups,
        [=](const auto& g) { g.lock()->updateEntity(entity, index, previousComponent, newComponent); });
}

void Pool::onEntityReleased(Entity* entity)
{
    if (entity->enabled_) {
        throw std::runtime_error("Error, cannot release entity. Entity is not destroyed yet.");
    }

    retainedEntities_.erase(entity);
    reusableEntities_.push(entity);
}
}
