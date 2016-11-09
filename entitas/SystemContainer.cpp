// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "SystemContainer.hpp"
#include "ReactiveSystem.hpp"
#include <memory>

namespace entitas
{
    auto SystemContainer::add(std::shared_ptr<ISystem> system) -> SystemContainer*
    {
		if(std::dynamic_pointer_cast<ReactiveSystem>(system) != nullptr)
		{
			if(std::dynamic_pointer_cast<IInitializeSystem>((std::dynamic_pointer_cast<ReactiveSystem>(system))->getSubsystem()) != nullptr)
			{
				mInitializeSystems.push_back(std::dynamic_pointer_cast<IInitializeSystem>((std::dynamic_pointer_cast<ReactiveSystem>(system))->getSubsystem()));
			}
		}
		else
		{
			if(std::dynamic_pointer_cast<IInitializeSystem>(system) != nullptr)
			{
				mInitializeSystems.push_back(std::dynamic_pointer_cast<IInitializeSystem>(system));
			}
		}

		if(std::dynamic_pointer_cast<IExecuteSystem>(system) != nullptr)
		{
			mExecuteSystems.push_back(std::dynamic_pointer_cast<IExecuteSystem>(system));
		}

		if(std::dynamic_pointer_cast<IFixedExecuteSystem>(system) != nullptr)
		{
			mFixedExecuteSystems.push_back(std::dynamic_pointer_cast<IFixedExecuteSystem>(system));
		}

		return this;
    }

    void SystemContainer::initialize()
    {
		for(const auto &system : mInitializeSystems)
		{
				system->initialize();
		}
    }

    void SystemContainer::execute()
    {
		for(const auto &system : mExecuteSystems)
		{
				system->execute();
		}
    }

    void SystemContainer::fixedExecute()
    {
		for(const auto &system : mFixedExecuteSystems)
		{
				system->fixedExecute();
		}
    }

    void SystemContainer::activateReactiveSystems()
    {
		for(const auto &system : mExecuteSystems)
		{
			if(std::dynamic_pointer_cast<ReactiveSystem>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<ReactiveSystem>(system))->activate();
			}

			if(std::dynamic_pointer_cast<SystemContainer>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<SystemContainer>(system))->activateReactiveSystems();
			}
		}
    }

    void SystemContainer::deactivateReactiveSystems()
    {
		for(const auto &system : mExecuteSystems)
		{
			if(std::dynamic_pointer_cast<ReactiveSystem>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<ReactiveSystem>(system))->deactivate();
			}

			if(std::dynamic_pointer_cast<SystemContainer>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<SystemContainer>(system))->deactivateReactiveSystems();
			}
		}
    }

    void SystemContainer::clearReactiveSystems()
    {
		for(const auto &system : mExecuteSystems)
		{
			if(std::dynamic_pointer_cast<ReactiveSystem>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<ReactiveSystem>(system))->clear();
			}

			if(std::dynamic_pointer_cast<SystemContainer>(system) != nullptr)
			{
				(std::dynamic_pointer_cast<SystemContainer>(system))->clearReactiveSystems();
			}
		}
    }
}
