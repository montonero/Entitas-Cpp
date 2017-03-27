// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ComponentTypeId.hpp"
#include "Delegate.hpp"
#include <map>
#include <stack>

#include <fmt/format.h>

namespace entitas {
class Entity;
using EntityPtr = std::shared_ptr<Entity>;
using EntityPtrWeak = std::weak_ptr<Entity>;
using ComponentPool = std::stack<IComponent*>;
using ComponentPools = std::map<ComponentId, ComponentPool>;

/* -------------------------------------------------------------------------- */

/// Use context.CreateEntity() to create a new entity and
/// context.DestroyEntity() to destroy it.
/// You can add, replace and remove IComponent to an entity.
class Entity {
    friend class Pool;

public:
    Entity(ComponentPools& componentPools)
        : componentPools_{ componentPools } {};
    
    ~Entity()
    {
        fmt::print("~Entity() id: {}", getUuid() );
    }

    template <typename T, typename... TArgs>
    inline auto add(TArgs&&... args) -> EntityPtr;
    template <typename T>
    inline auto remove() -> EntityPtr;
    template <typename T, typename... TArgs>
    inline auto replace(TArgs&&... args) -> EntityPtr;
    template <typename T>
    inline auto refresh() -> EntityPtr;

    template <typename T>
    inline auto get() const -> T*;
    template <typename T>
    inline auto use() -> T*;
    template <typename T>
    inline bool has() const;

    // Whether Entity has all of the components in the 'indices'
    bool hasComponents(const std::vector<ComponentId>& indices) const;
    // Whether Entity has any of the components
    bool hasAnyComponent(const std::vector<ComponentId>& indices) const;
    auto getComponentsCount() const -> unsigned int;
    void removeAllComponents();
    auto getUuid() const -> unsigned int;
    bool isEnabled();

    bool operator==(const EntityPtr& right) const;
    bool operator==(const Entity right) const;

    using EntityChanged = Delegate<void(EntityPtr entity, ComponentId index, IComponent* component)>;
    using ComponentReplaced = Delegate<void(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)>;
    using EntityReleased = Delegate<void(Entity* entity)>;

    EntityChanged onComponentAdded;
    ComponentReplaced onComponentReplaced;
    EntityChanged onComponentRemoved;
    /// Occurs when an entity gets released and is not retained anymore.
    /// All event handlers will be removed when
    /// the entity gets destroyed by the context.
    EntityReleased onEntityReleased;

protected:
    void setInstance(EntityPtr instance);
    /// Adds a component at the specified index.
    /// You can only have one component at an index.
    /// Each component type must have its own constant index.
    auto addComponent(const ComponentId index, IComponent* component) -> EntityPtr;
    auto removeComponent(const ComponentId index) -> EntityPtr;
    auto replaceComponent(const ComponentId index, IComponent* component) -> EntityPtr;
    auto getComponent(const ComponentId index) const -> IComponent*;
    bool hasComponent(const ComponentId index) const;
    void destroy();

    template <typename T, typename... TArgs>
    inline auto createComponent(TArgs&&... args) -> IComponent*;

    unsigned int uuid_{ 0 };
    bool enabled_{ true };

private:
    ComponentPool& getComponentPool(const ComponentId index) const;
    /// Replace a given component
    void replace(const ComponentId index, IComponent* replacement);

    EntityPtrWeak instance_;
    std::map<ComponentId, IComponent*> components_;
    /// componentPools is set by the context which created the entity and
    /// is used to reuse removed components.
    /// Removed components will be pushed to the componentPool.
    /// Use entity.CreateComponent(index, type) to get a new or
    /// reusable component from the componentPool.
    /// Use entity.GetComponentPool(index) to get a componentPool for
    /// a specific component index.
    ComponentPools& componentPools_;
};

/* -------------------------------------------------------------------------- */

template <typename T, typename... TArgs>
auto Entity::createComponent(TArgs&&... args) -> IComponent*
{
    auto componentPool = getComponentPool(ComponentTypeId::get<T>());
    IComponent* component = nullptr;

    if (componentPool.size() > 0) {
        component = componentPool.top();
        componentPool.pop();
    } else {
        component = new T();
    }

    (static_cast<T*>(component))->reset(std::forward<TArgs>(args)...);

    return component;
}

template <typename T, typename... TArgs>
auto Entity::add(TArgs&&... args) -> EntityPtr
{
    return addComponent(ComponentTypeId::get<T>(), createComponent<T>(std::forward<TArgs>(args)...));
}

template <typename T>
auto Entity::remove() -> EntityPtr
{
    return removeComponent(ComponentTypeId::get<T>());
}

template <typename T, typename... TArgs>
auto Entity::replace(TArgs&&... args) -> EntityPtr
{
    return replaceComponent(ComponentTypeId::get<T>(), createComponent<T>(std::forward<TArgs>(args)...));
}

template <typename T>
auto Entity::refresh() -> EntityPtr
{
    return replaceComponent(ComponentTypeId::get<T>(), get<T>());
}

template <typename T>
auto Entity::get() const -> T*
{
    return static_cast<T*>(getComponent(ComponentTypeId::get<T>()));
}

template <typename T>
auto Entity::use() -> T*
{
    refresh<T>();
    return static_cast<T*>(getComponent(ComponentTypeId::get<T>()));
}

template <typename T>
bool Entity::has() const
{
    return hasComponent(ComponentTypeId::get<T>());
}
}

namespace std {
template <>
struct hash<typename entitas::EntityPtrWeak> {
    std::size_t operator()(const entitas::EntityPtrWeak& ptr) const
    {
        return hash<unsigned int>()(ptr.lock()->getUuid());
    }
};

bool operator==(entitas::EntityPtrWeak left, entitas::EntityPtrWeak right);
}
