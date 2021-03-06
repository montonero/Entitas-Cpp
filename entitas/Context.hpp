// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "Group.hpp"
#include <map>
#include <unordered_map>

namespace entitas {
class ISystem;

class Context {
public:
    static const unsigned kStartCreationIndex = 1;
    Context(const unsigned int startCreationIndex = kStartCreationIndex);
    ~Context();

    auto createEntity() -> EntityPtr;
    bool hasEntity(const EntityPtr& entity) const;
    void destroyEntity(EntityPtr entity);
    void destroyAllEntities();

    Entities& getEntities();
    Entities& getEntities(const Matcher matcher);

    /// Returns a group for the specified matcher.
    /// Calling context.GetGroup(matcher) with the same matcher will always
    /// return the same instance of the group.
    auto getGroup(Matcher matcher) -> Group::SharedPtr;

    void clearGroups();
    void resetCreationIndex();
    void clearComponentPool(const ComponentId index);
    void clearComponentPools();
    void reset();

    auto count() const -> unsigned int;
    /// Returns the number of entities in the internal ObjectPool
    /// for entities which can be reused.
    auto getReusableEntitiesCount() const -> unsigned int;
    auto getRetainedEntitiesCount() const -> unsigned int;

    auto createSystem(std::shared_ptr<ISystem> system) -> std::shared_ptr<ISystem>;
    template <typename T>
    inline auto createSystem() -> std::shared_ptr<ISystem>;

    using EntityChanged = Delegate<void(Context* context, EntityPtr entity)>;
    using GroupChanged = Delegate<void(Context* context, Group::SharedPtr group)>;

    EntityChanged onEntityCreated;
    EntityChanged onEntityWillBeDestroyed;
    EntityChanged onEntityDestroyed;

    GroupChanged onGroupCreated;
    GroupChanged onGroupCleared;

private:
    void updateGroupsComponentAddedOrRemoved(EntityPtr entity, ComponentId index, IComponent* component);
    void updateGroupsComponentReplaced(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent);
    void onEntityReleased(Entity* entity);

    unsigned int creationIndex_; ///< Index that is used as uuid for Entities
    std::unordered_set<EntityPtr> entities_;
    std::unordered_map<Matcher, Group::SharedPtr> groups_;
    std::stack<Entity*> reusableEntities_;

    std::unordered_set<Entity*> retainedEntities_;

    ComponentPools componentPools_;
    /// ComponentId to corresponding groups map
    /// Used to quickly find groups when modifying components
    std::map<ComponentId, std::vector<std::weak_ptr<Group>>> groupsForIndex_;

    Entities entitiesCache_;
    // TODO cache other functions too
    // so that we don't have to construct lambda everytime
    std::function<void(Entity*)> onEntityReleasedCache_;
};

template <typename T>
auto Context::createSystem() -> std::shared_ptr<ISystem>
{
    return createSystem(std::dynamic_pointer_cast<ISystem>(std::make_shared<T>()));
}
} // namespace entitas
