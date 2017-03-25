// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT
#include "Entity.hpp"
#include <algorithm>

namespace entitas {
//Entity::Entity(ComponentPools* componentPools)
//{
//	componentPools_ = componentPools;
//}

auto Entity::addComponent(const ComponentId index, IComponent* component) -> EntityPtr
{
    if (!enabled_) {
        throw std::runtime_error("Error, cannot add component to entity, entity has already been destroyed.");
    }

    if (hasComponent(index)) {
        throw std::runtime_error("Error, cannot add component to entity, component already exists");
    }

    components_[index] = component;

    onComponentAdded(instance_.lock(), index, component);

    return instance_.lock();
}

auto Entity::removeComponent(const ComponentId index) -> EntityPtr
{
    if (!enabled_) {
        throw std::runtime_error("Error, cannot remove component to entity, entity has already been destroyed.");
    }

    if (!hasComponent(index)) {
        throw std::runtime_error("Error, cannot remove component to entity, component not exists");
    }

    replace(index, nullptr);

    return instance_.lock();
}

auto Entity::replaceComponent(const ComponentId index, IComponent* component) -> EntityPtr
{
    if (!enabled_) {
        throw std::runtime_error("Error, cannot replace component to entity, entity has already been destroyed.");
    }

    if (hasComponent(index)) {
        replace(index, component);
    } else if (component != nullptr) {
        addComponent(index, component);
    }

    return instance_.lock();
}

auto Entity::getComponent(const ComponentId index) const -> IComponent*
{
    if (!hasComponent(index)) {
        throw std::runtime_error("Error, cannot get component from entity, component does not exists");
    }

    return components_.at(index);
}

bool Entity::hasComponent(const ComponentId index) const
{
    return (components_.find(index) != components_.end());
}

bool Entity::hasComponents(const std::vector<ComponentId>& indices) const
{
    bool ret = std::all_of(begin(indices), end(indices), [this](auto i) { return this->hasComponent(i); });
    return ret;
}

bool Entity::hasAnyComponent(const std::vector<ComponentId>& indices) const
{
    return std::any_of(begin(indices), end(indices), [this](auto i) { return this->hasComponent(i); });
}

auto Entity::getComponentsCount() const -> unsigned int
{
    return static_cast<unsigned>(components_.size());
}

void Entity::removeAllComponents()
{
    std::vector<ComponentId> componentsIds;
    //for (const auto& pair : components_) {
    //    componentsIdTemp.push_back(pair.first);
    //}
    using namespace std;
    transform(begin(components_), end(components_), back_inserter(componentsIds), [](auto& p) { return p.first; });

    while (!components_.empty()) {
        // Replacing with nullptr removes it
        replace(componentsIds.back(), nullptr);
        componentsIds.pop_back();
    }
}

auto Entity::getUuid() const -> unsigned int
{
    return uuid_;
}

bool Entity::isEnabled()
{
    return enabled_;
}

bool Entity::operator==(const EntityPtr& right) const
{
    return this->getUuid() == right->getUuid();
}

bool Entity::operator==(const Entity right) const
{
    return this->getUuid() == right.getUuid();
}

void Entity::setInstance(EntityPtr instance)
{
    instance_ = instance;
}

void Entity::destroy()
{
    removeAllComponents();
    onComponentAdded.clear();
    onComponentReplaced.clear();
    onComponentRemoved.clear();
    enabled_ = false;
}

auto Entity::getComponentPool(const ComponentId index) const -> ComponentPool&
{
    return (componentPools_[index]);
}

void Entity::replace(const ComponentId index, IComponent* replacement)
{
    auto previousComponent = getComponent(index);

    if (previousComponent == replacement) {
        onComponentReplaced(instance_.lock(), index, previousComponent, replacement);
    } else {
        // Save 'replaced' component to the pool for later reuse
        getComponentPool(index).push(previousComponent);

        if (replacement == nullptr) {
            components_.erase(index);
            onComponentRemoved(instance_.lock(), index, previousComponent);
        } else {
            components_[index] = replacement;
            onComponentReplaced(instance_.lock(), index, previousComponent, replacement);
        }
    }
}
}

namespace std {
bool operator==(entitas::EntityPtrWeak left, entitas::EntityPtrWeak right)
{
    return left.lock().get() == right.lock().get();
}
}
