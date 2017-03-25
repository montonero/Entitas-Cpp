// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "Group.hpp"
#include <unordered_map>
#include <map>


namespace entitas
{
class ISystem;

class Pool
{
	public:    
		static const unsigned kStartCreationIndex = 1;
		Pool(const unsigned int startCreationIndex = kStartCreationIndex);
		~Pool();

		auto createEntity() -> EntityPtr;
		bool hasEntity(const EntityPtr& entity) const;
		void destroyEntity(EntityPtr entity);
		void destroyAllEntities();

		auto getEntities() ->  std::vector<EntityPtr>&;
		auto getEntities(const Matcher matcher) ->  std::vector<EntityPtr>&;
    
    /// Returns a group for the specified matcher.
    /// Calling context.GetGroup(matcher) with the same matcher will always
    /// return the same instance of the group.
		auto getGroup(Matcher matcher) -> Group::SharedPtr;

		void clearGroups();
		void resetCreationIndex();
		void clearComponentPool(const ComponentId index);
		void clearComponentPools();
		void reset();

		auto getEntityCount() const -> unsigned int;
		auto getReusableEntitiesCount() const -> unsigned int;
		auto getRetainedEntitiesCount() const -> unsigned int;

		auto createSystem(std::shared_ptr<ISystem> system) -> std::shared_ptr<ISystem>;
		template <typename T> inline auto createSystem() -> std::shared_ptr<ISystem>;

		using PoolChanged = Delegate<void(Pool* pool, EntityPtr entity)>;
		using GroupChanged = Delegate<void(Pool* pool, Group::SharedPtr group)>;

		PoolChanged onEntityCreated;
		PoolChanged onEntityWillBeDestroyed;
		PoolChanged onEntityDestroyed;

		GroupChanged onGroupCreated;
		GroupChanged onGroupCleared;

	private:
		void updateGroupsComponentAddedOrRemoved(EntityPtr entity, ComponentId index, IComponent* component);
		void updateGroupsComponentReplaced(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent);
		void onEntityReleased(Entity* entity);

		unsigned int creationIndex_;									///< Index that is used as uuid for Entities
		std::unordered_set<EntityPtr> entities_;
		std::unordered_map<Matcher, Group::SharedPtr> groups_;
		std::stack<Entity*> mReusableEntities;

		std::unordered_set<Entity*> retainedEntities_;
    
    
		ComponentPools componentPools_;
		/// ComponentId to corresponding groups map
		/// Used to quickly find groups when modifying components
		std::map<ComponentId, std::vector<std::weak_ptr<Group>>> groupsForIndex_;

		std::vector<EntityPtr> entitiesCache_;
        // TODO cache other functions too
        // so that we don't have to construct lambda everytime
		std::function<void(Entity*)> onEntityReleasedCache_;
};

template <typename T>
auto Pool::createSystem() -> std::shared_ptr<ISystem>
{
	return createSystem(std::dynamic_pointer_cast<ISystem>(std::make_shared<T>()));
}
}
