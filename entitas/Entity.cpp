// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT
#include <algorithm>
#include "Entity.hpp"


namespace entitas
{
    Entity::Entity(std::map<ComponentId, std::stack<IComponent*>>* componentPools)
    {
		mComponentPools = componentPools;
    }

    auto Entity::addComponent(const ComponentId index, IComponent* component) -> EntityPtr
    {
		if (! enabled_)
		{
			throw std::runtime_error("Error, cannot add component to entity, entity has already been destroyed.");
		}

		if (hasComponent(index))
		{
			throw std::runtime_error("Error, cannot add component to entity, component already exists");
		}

		components_[index] = component;

		onComponentAdded(mInstance.lock(), index, component);

		return mInstance.lock();
    }

    auto Entity::removeComponent(const ComponentId index) -> EntityPtr
    {
		if (! enabled_)
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
		if (! enabled_)
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
        bool r2 = std::all_of(begin(indices), end(indices), [this](auto i){ return hasComponent(i);});

        for(const ComponentId &index : indices)
		{
			if (!hasComponent(index))
			{
                if (r2)
                    throw std::runtime_error("all_off different");
				return false;
			}
		}


		return true;
    }

	bool Entity::hasAnyComponent(const std::vector<ComponentId>& indices) const
	{
        return std::any_of(begin(indices), end(indices), [this](auto i){return hasComponent(i);});
    }

    auto Entity::getComponentsCount() const -> unsigned int
    {
		return static_cast<unsigned>(components_.size());
    }

    void Entity::removeAllComponents()
    {
		{
            auto componentsIdTemp = std::vector<ComponentId>(components_.size());

            for(const auto &pair : components_)
            {
                componentsIdTemp.push_back(pair.first);
            }

            while(! components_.empty())
            {
                replace(componentsIdTemp.back(), nullptr);
                componentsIdTemp.pop_back();
            }
		}
    }

    auto Entity::getUuid() const -> const unsigned int
    {
		return uuid_;
    }

    bool Entity::isEnabled()
    {
		return enabled_;
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
		mInstance = instance;
    }

    void Entity::destroy()
    {
		removeAllComponents();
		onComponentAdded.clear();
		onComponentReplaced.clear();
		onComponentRemoved.clear();
		enabled_ = false;
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
            onComponentReplaced(mInstance.lock(), index, previousComponent, replacement);
	}
	else
	{
            getComponentPool(index)->push(previousComponent);

            if(replacement == nullptr)
            {
                components_.erase(index);
                onComponentRemoved(mInstance.lock(), index, previousComponent);
            }
            else
            {
                components_[index] = replacement;
                onComponentReplaced(mInstance.lock(), index, previousComponent, replacement);
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
