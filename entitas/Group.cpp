// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Group.hpp"
#include "Collector.hpp"
#include "Functional.hpp"
#include "Matcher.hpp"
#include <algorithm>

namespace entitas {

Group::Group(const Matcher& matcher)
    : matcher_(matcher)
{
}

auto Group::count() const -> unsigned int
{
    return static_cast<unsigned>(entities_.size());
}

auto Group::getEntities() -> Entities&
{
    if (entitiesCache_.empty() && !entities_.empty()) {
        entitiesCache_ = Entities(entities_.begin(), entities_.end());
    }

    return entitiesCache_;
}

auto Group::getSingleEntity() const -> EntityPtr
{
    auto c = count();
    if (c == 1) {
        return *(entities_.begin());
    } else if (c == 0) {
        return nullptr;
    } else {
        throw std::runtime_error("Error, cannot get the single entity from group. Group contains more than one entity.");
    }
    return nullptr;
}

bool Group::containsEntity(const EntityPtr& entity) const
{
    return doesExist(entities_, entity);
}

auto Group::getMatcher() const -> Matcher
{
    return matcher_;
}

auto Group::createCollector(const GroupEventType eventType) -> std::shared_ptr<Collector>
{
    return std::make_shared<Collector>(instance_.lock(), eventType);
}

void Group::setInstance(std::shared_ptr<Group> instance)
{
    instance_ = instance;
}

auto Group::handleEntity(EntityPtr entity) -> GroupChanged*
{
    return matcher_.matches(entity) ? addEntity(entity) : removeEntity(entity);
}

void Group::handleEntitySilently(EntityPtr entity)
{
    if (matcher_.matches(entity)) {
        addEntitySilently(entity);
    } else {
        removeEntitySilently(entity);
    }
}

void Group::handleEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
    if (matcher_.matches(entity)) {
        addEntity(entity, index, component);
    } else {
        removeEntity(entity, index, component);
    }
}

void Group::updateEntity(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)
{
    if (containsEntity(entity)) {
        onEntityRemoved(instance_.lock(), entity, index, previousComponent);
        onEntityAdded(instance_.lock(), entity, index, newComponent);
        onEntityUpdated(instance_.lock(), entity, index, previousComponent, newComponent);
    }
}

/// This is called by context.Reset() and context.ClearGroups() to remove
/// all event handlers.
/// This is useful when you want to soft-restart your application.
void Group::removeAllEventHandlers()
{
    onEntityAdded.clear();
    onEntityRemoved.clear();
    onEntityUpdated.clear();
}

bool Group::addEntitySilently(EntityPtr entity)
{
    // True if insertion took place
    if (entities_.insert(entity).second) {
        // Since entity was added we must update cache
        entitiesCache_.clear();
        return true;
    }

    return false;
}

void Group::addEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
    if (addEntitySilently(entity)) {
        onEntityAdded(instance_.lock(), entity, index, component);
    }
}

auto Group::addEntity(EntityPtr entity) -> GroupChanged*
{
    return addEntitySilently(entity) ? &onEntityAdded : nullptr;
}

bool Group::removeEntitySilently(EntityPtr entity)
{
    if (entities_.erase(entity)) {
        entitiesCache_.clear();
        return true;
    }
    // No entities were removed
    return false;
}

void Group::removeEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
    if (removeEntitySilently(entity)) {
        onEntityRemoved(instance_.lock(), entity, index, component);
    }
}

auto Group::removeEntity(EntityPtr entity) -> GroupChanged*
{
    return removeEntitySilently(entity) ? &onEntityRemoved : nullptr;
}
}
