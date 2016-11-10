// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Pool.hpp"
#include "Entity.hpp"
#include "ISystem.hpp"
#include "ReactiveSystem.hpp"
#include <algorithm>

namespace entitas
{
    Pool::Pool(const unsigned int startCreationIndex)
    {
		creationIndex_ = startCreationIndex;
		mOnEntityReleasedCache = std::bind(&Pool::onEntityReleased, this, std::placeholders::_1);
    }

    Pool::~Pool()
    {
		reset();

		// TODO: Components don't get destroyed.

		if(! retainedEntities_.empty())
		{
				// Warning, some entities remain undestroyed in the pool destruction !"
		}

		while(! mReusableEntities.empty())
		{
			delete mReusableEntities.top();
			mReusableEntities.pop();
		}

		for(auto &pair : mComponentPools)
		{
			auto componentPool = pair.second;

			while(! componentPool.empty())
			{
				delete componentPool.top();
				componentPool.pop();
			}
		}
    }

    auto Pool::createEntity() -> EntityPtr
    {
		EntityPtr entity;

		if(mReusableEntities.size() > 0)
		{
			entity = EntityPtr(mReusableEntities.top());
			mReusableEntities.pop();
		}
		else
		{
			entity = EntityPtr(new Entity(&mComponentPools), [](Entity* entity)
								{
									entity->onEntityReleased(entity);
								});
		}

		entity->setInstance(entity);
		entity->enabled_ = true;
		entity->uuid_ = creationIndex_++;

		entities_.insert(entity);
		entitiesCache_.clear();

		entity->onComponentAdded += [this](EntityPtr entity, ComponentId index, IComponent* component)
		{
			updateGroupsComponentAddedOrRemoved(entity, index, component);
		};
		entity->onComponentRemoved += [this](EntityPtr entity, ComponentId index, IComponent* component)
		{
			updateGroupsComponentAddedOrRemoved(entity, index, component);
		};
		entity->onComponentReplaced += [this](EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)
		{
			updateGroupsComponentReplaced(entity, index, previousComponent, newComponent);
		};

		entity->onEntityReleased.clear();
		entity->onEntityReleased += mOnEntityReleasedCache;

		onEntityCreated(this, entity);

		return entity;
    }

    bool Pool::hasEntity(const EntityPtr& entity) const
    {
		return std::find(entities_.begin(), entities_.end(), std::weak_ptr<Entity>(entity)) != entities_.end();
    }

    void Pool::destroyEntity(EntityPtr entity)
    {
		auto removed = entities_.erase(entity);

		if (! removed)
		{
			throw std::runtime_error("Error, cannot destroy entity. Pool does not contain entity.");
		}

		entitiesCache_.clear();

		onEntityWillBeDestroyed(this, entity);
		entity->destroy();
		onEntityDestroyed(this, entity);

		if (entity.use_count() == 1)
		{
			entity->onEntityReleased -= mOnEntityReleasedCache;
			mReusableEntities.push(entity.get());
		}
		else
		{
			retainedEntities_.insert(entity.get());
		}
    }

    void Pool::destroyAllEntities()
    {
		{
			auto entitiesTemp = std::vector<EntityPtr>(entities_.begin(), entities_.end());

			while(! entities_.empty())
			{
				destroyEntity(entitiesTemp.back());
				entitiesTemp.pop_back();
			}
		}

		entities_.clear();

		if (! retainedEntities_.empty())
		{
			// Try calling Pool.clearGroups() and SystemContainer.clearReactiveSystems() before calling pool.destroyAllEntities() to avoid memory leaks
			throw std::runtime_error("Error, pool detected retained entities although all entities got destroyed. Did you release all entities?");
		}
    }

    auto Pool::getEntities() -> std::vector<EntityPtr>
    {
		if(entitiesCache_.empty())
		{
			entitiesCache_ = std::vector<EntityPtr>(entities_.begin(), entities_.end());
		}
		return entitiesCache_;
    }

    auto Pool::getEntities(const Matcher matcher) -> std::vector<EntityPtr>
    {
		return getGroup(std::move(matcher))->getEntities();
    }

    auto Pool::getGroup(Matcher matcher) -> std::shared_ptr<Group>
    {
		std::shared_ptr<Group> group;
		auto it = groups_.find(matcher);
		if (it == groups_.end())
		{
			group.reset(new Group(matcher));
			group->setInstance(group);

			// 'Handle' all pool's entities
			auto entities = getEntities();
			for (int i = 0, entitiesLength = entities.size(); i < entitiesLength; i++)
			{
				group->handleEntitySilently(entities[i]);
			}

			groups_[group->getMatcher()] = group;

			for (int i = 0, indicesLength = matcher.getIndices().size(); i < indicesLength; i++)
			{
				groupsForIndex_[matcher.getIndices()[i]].push_back(group);
			}

			onGroupCreated(this, group);
		}
		else
		{
			group = it->second;
		}

		return group;
    }

    void Pool::clearGroups()
    {
		for (const auto &it : groups_)
		{
			it.second->removeAllEventHandlers();
			onGroupCleared(this, it.second);
		}

		groups_.clear();

		for (auto &pair : groupsForIndex_)
		{
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
		while(! mComponentPools.at(index).empty())
		{
			mComponentPools.at(index).pop();
		}
    }

    void Pool::clearComponentPools()
    {
		for(const auto &pair : mComponentPools)
		{
			clearComponentPool(pair.first);
		}
    }

    void Pool::reset()
    {
		clearGroups();
		destroyAllEntities();
		resetCreationIndex();
    }

    auto Pool::getEntityCount() const -> unsigned int
    {
		return entities_.size();
    }

    auto Pool::getReusableEntitiesCount() const -> unsigned int
    {
	return mReusableEntities.size();
    }

    auto Pool::getRetainedEntitiesCount() const -> unsigned int
    {
		return retainedEntities_.size();
    }

    auto Pool::createSystem(std::shared_ptr<ISystem> system) -> std::shared_ptr<ISystem>
    {
		if(std::dynamic_pointer_cast<ISetPoolSystem>(system) != nullptr)
		{
			(std::dynamic_pointer_cast<ISetPoolSystem>(system)->setPool(this));
		}

		if(std::dynamic_pointer_cast<IReactiveSystem>(system) != nullptr)
		{
			return std::shared_ptr<ReactiveSystem>(new ReactiveSystem(this, std::dynamic_pointer_cast<IReactiveSystem>(system)));
		}

		if(std::dynamic_pointer_cast<IMultiReactiveSystem>(system) != nullptr)
		{
			return std::shared_ptr<ReactiveSystem>(new ReactiveSystem(this, std::dynamic_pointer_cast<IMultiReactiveSystem>(system)));
		}

		return system;
    }

    void Pool::updateGroupsComponentAddedOrRemoved(EntityPtr entity, ComponentId index, IComponent* component)
    {
		if(groupsForIndex_.find(index) == groupsForIndex_.end())
		{
			return;
		}

		auto groups = groupsForIndex_[index];

		if (groups.size() > 0)
		{
			auto events = std::vector<Group::GroupChanged*>();

			for (int i = 0, groupsCount = groups.size(); i < groupsCount; ++i)
			{
				events.push_back(groups[i].lock()->handleEntity(entity));
			}

			for (int i = 0, eventsCount = events.size(); i < eventsCount; ++i)
			{
				(*events[i])(groups[i].lock(), entity, index, component);
			}
		}
    }

    void Pool::updateGroupsComponentReplaced(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)
    {
		if(groupsForIndex_.find(index) == groupsForIndex_.end())
		{
			return;
		}

		if (groupsForIndex_[index].size() > 0)
		{
			for(const auto &g : groupsForIndex_[index])
			{
				g.lock()->updateEntity(entity, index, previousComponent, newComponent);
			}
		}
    }

    void Pool::onEntityReleased(Entity* entity)
    {
		if (entity->enabled_)
		{
			throw std::runtime_error("Error, cannot release entity. Entity is not destroyed yet.");
		}

		retainedEntities_.erase(entity);
		mReusableEntities.push(entity);
    }
}
