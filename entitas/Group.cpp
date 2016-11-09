// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Group.hpp"
#include "Matcher.hpp"
#include "GroupObserver.hpp"
#include <algorithm>

namespace entitas
{

Group::Group(const Matcher& matcher) : mMatcher(matcher)
{
}

auto Group::count() const -> const unsigned int
{
	return mEntities.size();
}

auto Group::getEntities() -> std::vector<EntityPtr>
{
	if(mEntitiesCache.empty() && !mEntities.empty())
	{
		mEntitiesCache = std::vector<EntityPtr>(mEntities.begin(), mEntities.end());
	}

	return mEntitiesCache;
}

auto Group::getSingleEntity() const -> EntityPtr
{
	auto c = count();
	if(c == 1)
	{
		return *(mEntities.begin());
	}
	else if(c == 0)
	{
		return nullptr;
	}
	else
	{
		throw std::runtime_error("Error, cannot get the single entity from group. Group contains more than one entity.");
	}
	return nullptr;
}

bool Group::containsEntity(const EntityPtr& entity) const
{
	return std::find(mEntities.begin(), mEntities.end(), entity) != mEntities.end();
}

auto Group::getMatcher() const -> Matcher
{
	return mMatcher;
}

auto Group::createObserver(const GroupEventType eventType) -> std::shared_ptr<GroupObserver>
{
	return std::shared_ptr<GroupObserver>(new GroupObserver(mInstance.lock(), eventType));
}

void Group::setInstance(std::shared_ptr<Group> instance)
{
	mInstance = std::weak_ptr<Group>(instance);
}

auto Group::handleEntity(EntityPtr entity) -> GroupChanged*
{
	return mMatcher.matches(entity) ? addEntity(entity) : removeEntity(entity);
}

void Group::handleEntitySilently(EntityPtr entity)
{
	if(mMatcher.matches(entity))
	{
		addEntitySilently(entity);
	}
	else
	{
		removeEntitySilently(entity);
	}
}

void Group::handleEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
	if(mMatcher.matches(entity))
	{
		addEntity(entity, index, component);
	}
	else
	{
		removeEntity(entity, index, component);
	}
}

void Group::updateEntity(EntityPtr entity, ComponentId index, IComponent* previousComponent, IComponent* newComponent)
{
	if(containsEntity(entity))
	{
		onEntityRemoved(mInstance.lock(), entity, index, previousComponent);
		onEntityAdded(mInstance.lock(), entity, index, newComponent);
		onEntityUpdated(mInstance.lock(), entity, index, previousComponent, newComponent);
	}
}

void Group::removeAllEventHandlers()
{
	onEntityAdded.clear();
	onEntityRemoved.clear();
	onEntityUpdated.clear();
}

bool Group::addEntitySilently(EntityPtr entity)
{
	if(mEntities.insert(entity).second)
	{
		mEntitiesCache.clear();
		return true;
	}

	return false;
}

void Group::addEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
	if(addEntitySilently(entity))
	{
		onEntityAdded(mInstance.lock(), entity, index, component);
	}
}

auto Group::addEntity(EntityPtr entity) -> GroupChanged*
{
	return addEntitySilently(entity) ? &onEntityAdded : nullptr;
}

bool Group::removeEntitySilently(EntityPtr entity)
{
	if(mEntities.erase(entity))
	{
		mEntitiesCache.clear();
		return true;
	}

	return false;
}

void Group::removeEntity(EntityPtr entity, ComponentId index, IComponent* component)
{
	if(removeEntitySilently(entity))
	{
		onEntityRemoved(mInstance.lock(), entity, index, component);
	}
}

auto Group::removeEntity(EntityPtr entity) -> GroupChanged*
{
	return removeEntitySilently(entity) ? &onEntityRemoved : nullptr;
}
}
