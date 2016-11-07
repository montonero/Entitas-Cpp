// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Entity.hpp"

namespace entitas
{
Entity::Entity(std::map<ComponentId, std::stack<IComponent*>>* componentPools)
{
	mComponentPools = componentPools;
}

auto Entity::addComponent(const ComponentId index, IComponent* component) -> EntityPtr
{
	if (! mIsEnabled)
	{
		throw std::runtime_error("Error, cannot add component to entity, entity has already been destroyed.");
	}

	if (hasComponent(index))
	{
		throw std::runtime_error("Error, cannot add component to entity, component already exists");
	}

	mComponents[index] = component;

	OnComponentAdded(mInstance.lock(), index, component);

	return mInstance.lock();
}

auto Entity::removeComponent(const ComponentId index) -> EntityPtr
{
	if (! mIsEnabled)
	{
		throw std::runtime_error("Error, cannot remove component to entity, entity has already been destroyed.");
	}

	if (! hasComponent(index))
	{
		throw std::runtime_error("Error, cannot remove component to entity, component not exists");
	}

	replace(index, nullptr);

	return mInstance.lock();
}

auto Entity::replaceComponent(const ComponentId index, IComponent* component) -> EntityPtr
{
	if (! mIsEnabled)
	{
		throw std::runtime_error("Error, cannot replace component to entity, entity has already been destroyed.");
	}

	if (hasComponent(index))
	{
		replace(index, component);
	}
	else if (component != nullptr)
	{
		addComponent(index, component);
	}

	return mInstance.lock();
}

auto Entity::getComponent(const ComponentId index) const -> IComponent*
{
	if (! hasComponent(index))
	{
		throw std::runtime_error("Error, cannot get component from entity, component not exists");
	}

	return mComponents.at(index);
}

bool Entity::hasComponent(const ComponentId index) const
{
	return (mComponents.find(index) != mComponents.end());
}

bool Entity::hasComponents(const std::vector<ComponentId>& indices) const
{
	for(const ComponentId &index : indices)
	{
		if (! hasComponent(index))
		{
			return false;
		}
	}

	return true;
}

bool Entity::hasAnyComponent(const std::vector<ComponentId>& indices) const
{
	for(const ComponentId &index : indices)
	{
		if (hasComponent(index))
		{
			return true;
		}
	}

	return false;
}

auto Entity::getComponentsCount() const -> unsigned int
{
	return mComponents.size();
}

void Entity::removeAllComponents()
{
	{
		auto componentsIdTemp = std::vector<ComponentId>(mComponents.size());

		for(const auto &pair : mComponents)
		{
			componentsIdTemp.push_back(pair.first);
		}

		while(! mComponents.empty())
		{
			replace(componentsIdTemp.back(), nullptr);
			componentsIdTemp.pop_back();
		}
	}
}

auto Entity::getUuid() const -> const unsigned int
{
	return mUuid;
}

bool Entity::isEnabled()
{
	return mIsEnabled;
}

bool Entity::operator ==(const EntityPtr& right) const
{
	return this->getUuid() == right->getUuid();
}

bool Entity::operator ==(const Entity right) const
{
	return this->getUuid() == right.getUuid();
}

void Entity::setInstance(EntityPtr instance)
{
	mInstance = std::weak_ptr<Entity>(instance);
}

void Entity::destroy()
{
	removeAllComponents();
	OnComponentAdded.clear();
	OnComponentReplaced.clear();
	OnComponentRemoved.clear();
	mIsEnabled = false;
}

auto Entity::getComponentPool(const ComponentId index) const -> std::stack<IComponent*>*
{
	return &((*mComponentPools)[index]);
}

void Entity::replace(const ComponentId index, IComponent* replacement)
{
	auto previousComponent = getComponent(index);

	if(previousComponent == replacement)
	{
		OnComponentReplaced(mInstance.lock(), index, previousComponent, replacement);
	}
	else
	{
		getComponentPool(index)->push(previousComponent);

		if(replacement == nullptr)
		{
			mComponents.erase(index);
			OnComponentRemoved(mInstance.lock(), index, previousComponent);
		}
		else
		{
			mComponents[index] = replacement;
			OnComponentReplaced(mInstance.lock(), index, previousComponent, replacement);
		}
	}
}
}

namespace std
{
bool operator ==(weak_ptr<entitas::Entity> left, weak_ptr<entitas::Entity> right)
{
	return left.lock().get() == right.lock().get();
}
}
