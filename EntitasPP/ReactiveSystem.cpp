// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "ReactiveSystem.hpp"
#include "Pool.hpp"
#include "TriggerOnEvent.hpp"

namespace EntitasPP
{
ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveSystem> subsystem) :
	ReactiveSystem(pool, subsystem, std::vector<TriggerOnEvent>(1, subsystem->trigger))
{
}

ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IMultiReactiveSystem> subsystem) :
	ReactiveSystem(pool, subsystem, subsystem->triggers)
{
}

ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveExecuteSystem> subsystem, std::vector<TriggerOnEvent> triggers)
{
	mSubsystem = subsystem;

	if(std::dynamic_pointer_cast<IEnsureComponents>(subsystem) != nullptr)
	{
		mEnsureComponents = (std::dynamic_pointer_cast<IEnsureComponents>(subsystem))->ensureComponents;
	}

	if(std::dynamic_pointer_cast<IExcludeComponents>(subsystem) != nullptr)
	{
		mExcludeComponents = (std::dynamic_pointer_cast<IExcludeComponents>(subsystem))->excludeComponents;
	}

	if(std::dynamic_pointer_cast<IclearReactiveSystem>(subsystem) != nullptr)
	{
		mClearAfterExecute = true;
	}

	unsigned int triggersLength = triggers.size();
	auto groups = std::vector<std::shared_ptr<Group>>(triggersLength);
	auto eventTypes = std::vector<GroupEventType>(triggersLength);

	for(unsigned int i = 0; i < triggersLength; ++i)
	{
		auto trigger = triggers[i];
		groups[i] = pool->getGroup(trigger.trigger);
		eventTypes[i] = trigger.eventType;
	}

	mObserver = new GroupObserver(groups, eventTypes);
}

ReactiveSystem::~ReactiveSystem ()
{
	deactivate();
	delete mObserver;
}

auto ReactiveSystem::getSubsystem() const -> std::shared_ptr<IReactiveExecuteSystem>
{
	return mSubsystem;
}

void ReactiveSystem::activate()
{
	mObserver->activate();
}

void ReactiveSystem::deactivate()
{
	mObserver->deactivate();
}

void ReactiveSystem::clear()
{
	mObserver->clearCollectedEntities();
}

void ReactiveSystem::execute()
{
	if(mObserver->getCollectedEntities().size() != 0)
	{
		if(! mEnsureComponents.isEmpty())
		{
			if(! mExcludeComponents.isEmpty())
			{
				for(const auto &e : mObserver->getCollectedEntities())
				{
					if(mEnsureComponents.matches(e) && ! mExcludeComponents.matches(e))
					{
						mEntityBuffer.push_back(e);
					}
				}
			}
			else
			{
				for(const auto &e : mObserver->getCollectedEntities())
				{
					if(mEnsureComponents.matches(e))
					{
						mEntityBuffer.push_back(e);
					}
				}
			}
		}
		else if(! mExcludeComponents.isEmpty())
		{
			for(const auto &e : mObserver->getCollectedEntities())
			{
				if(! mExcludeComponents.matches(e))
				{
					mEntityBuffer.push_back(e);
				}
			}
		}
		else
		{
			for(const auto &e : mObserver->getCollectedEntities())
			{
				mEntityBuffer.push_back(e);
			}
		}

		mObserver->clearCollectedEntities();

		if(mEntityBuffer.size() != 0)
		{
			mSubsystem->execute(mEntityBuffer);
			mEntityBuffer.clear();

			if(mClearAfterExecute)
			{
				mObserver->clearCollectedEntities();
			}
		}
	}
}
}
