// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "Matcher.hpp"
#include "GroupEventType.hpp"
#include <unordered_set>

namespace entitas
{
class GroupObserver;

class Group
{
	friend class Pool;

	public:
		Group(const Matcher& matcher);
		auto count() const -> const unsigned int;
		auto getEntities() -> std::vector<EntityPtr>;
		auto getSingleEntity() const -> EntityPtr;
		bool containsEntity(const EntityPtr& entity) const;
		auto getMatcher() const -> Matcher;
		auto createObserver(const GroupEventType eventType) -> std::shared_ptr<GroupObserver>;

		using GroupChanged = Delegate<void(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component)>;
		using GroupUpdated = Delegate<void(std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)>;

		GroupChanged onEntityAdded;
		GroupUpdated onEntityUpdated;
		GroupChanged onEntityRemoved;

	protected:
		void setInstance(std::shared_ptr<Group> instance);
		auto handleEntity(EntityPtr entity) -> GroupChanged*;
		void handleEntitySilently(EntityPtr entity);
		void handleEntity(EntityPtr entity, ComponentId index, IComponent* component);
		void updateEntity(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent);
		void removeAllEventHandlers();

	private:
		bool addEntitySilently(EntityPtr entity);
		void addEntity(EntityPtr entity, ComponentId index, IComponent* component);
		auto addEntity(EntityPtr entity) -> GroupChanged*;
		bool removeEntitySilently(EntityPtr entity);
		void removeEntity(EntityPtr entity, ComponentId index, IComponent* component);
		auto removeEntity(EntityPtr entity) -> GroupChanged*;

		std::weak_ptr<Group> mInstance;
		Matcher mMatcher;
		std::unordered_set<EntityPtr> mEntities;
		std::vector<EntityPtr> mEntitiesCache;
};
}