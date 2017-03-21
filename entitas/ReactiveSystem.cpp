// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "ReactiveSystem.hpp"
#include "Pool.hpp"
#include "TriggerOnEvent.hpp"

namespace entitas
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

	if(std::dynamic_pointer_cast<IClearReactiveSystem>(subsystem) != nullptr)
	{
		mClearAfterExecute = true;
	}

	auto triggersLength = triggers.size();
	auto groups = std::vector<std::shared_ptr<Group>>(triggersLength);
	auto eventTypes = std::vector<GroupEventType>(triggersLength);

	for(unsigned int i = 0; i < triggersLength; ++i)
	{
		auto trigger = triggers[i];
		groups[i] = pool->getGroup(trigger.trigger);
		eventTypes[i] = trigger.eventType;
	}

	collector_ = new Collector(groups, eventTypes);
}

ReactiveSystem::~ReactiveSystem ()
{
	deactivate();
	delete collector_;
}

auto ReactiveSystem::getSubsystem() const -> std::shared_ptr<IReactiveExecuteSystem>
{
	return mSubsystem;
}

void ReactiveSystem::activate()
{
	collector_->activate();
}

void ReactiveSystem::deactivate()
{
	collector_->deactivate();
}

void ReactiveSystem::clear()
{
	collector_->clearCollectedEntities();
}

void ReactiveSystem::execute()
{
	if(collector_->getCollectedEntities().size() != 0)
	{
		if(! mEnsureComponents.isEmpty())
		{
			if(! mExcludeComponents.isEmpty())
			{
				for(const auto &e : collector_->getCollectedEntities())
				{
					if(mEnsureComponents.matches(e) && ! mExcludeComponents.matches(e))
					{
						mEntityBuffer.push_back(e);
					}
				}
			}
			else
			{
				for(const auto &e : collector_->getCollectedEntities())
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
			for(const auto &e : collector_->getCollectedEntities())
			{
				if(! mExcludeComponents.matches(e))
				{
					mEntityBuffer.push_back(e);
				}
			}
		}
		else
		{
			for(const auto &e : collector_->getCollectedEntities())
			{
				mEntityBuffer.push_back(e);
			}
		}

		collector_->clearCollectedEntities();

		if(mEntityBuffer.size() != 0)
		{
			mSubsystem->execute(mEntityBuffer);
			mEntityBuffer.clear();

			if(mClearAfterExecute)
			{
				collector_->clearCollectedEntities();
			}
		}
	}
}
}
