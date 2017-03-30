// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "ReactiveSystem.hpp"
#include "Pool.hpp"
#include "TriggerOnEvent.hpp"

namespace entitas {
ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveSystem> subsystem)
    : ReactiveSystem(pool, subsystem, std::vector<TriggerOnEvent>{ subsystem->trigger })
{
}

ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IMultiReactiveSystem> subsystem)
    : ReactiveSystem(pool, subsystem, subsystem->triggers)
{
}

ReactiveSystem::ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveExecuteSystem> subsystem, std::vector<TriggerOnEvent> triggers)
    : subsystem_{ subsystem }
{
    using std::dynamic_pointer_cast;
    if (auto subsystemEnsure = dynamic_pointer_cast<IEnsureComponents>(subsystem)) {
        ensureComponents_ = subsystemEnsure->ensureComponents;
    }

    if (auto subsystemExclude = dynamic_pointer_cast<IExcludeComponents>(subsystem)) {
        excludeComponents_ = subsystemExclude->excludeComponents;
    }

    
    if (dynamic_pointer_cast<IClearReactiveSystem>(subsystem) != nullptr) {
        clearAfterExecute_ = true;
    }

    auto triggersLength = triggers.size();
    auto groups = std::vector<Group::SharedPtr>(triggersLength);
    auto eventTypes = std::vector<GroupEventType>(triggersLength);

    for (unsigned int i = 0; i < triggersLength; ++i) {
        auto trigger = triggers[i];
        groups[i] = pool->getGroup(trigger.trigger);
        eventTypes[i] = trigger.eventType;
    }

#if 0
    for_each(triggers, [&](auto& trigger)
             {
                 groups.push_back(pool->getGroup(trigger));
                 eventTypes.push_back(trigger.eventType);
             });
#endif

    collector_ = new Collector(std::move(groups), std::move(eventTypes));
}

ReactiveSystem::~ReactiveSystem()
{
    deactivate();
    delete collector_;
}

auto ReactiveSystem::getSubsystem() const -> std::shared_ptr<IReactiveExecuteSystem>
{
    return subsystem_;
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
    if (collector_->getCollectedEntities().size() != 0) {
        if (!ensureComponents_.isEmpty()) {
            if (!excludeComponents_.isEmpty()) {
                for (const auto& e : collector_->getCollectedEntities()) {
                    if (ensureComponents_.matches(e) && !excludeComponents_.matches(e)) {
                        entityBuffer_.push_back(e);
                    }
                }
            } else {
                for (const auto& e : collector_->getCollectedEntities()) {
                    if (ensureComponents_.matches(e)) {
                        entityBuffer_.push_back(e);
                    }
                }
            }
        } else if (!excludeComponents_.isEmpty()) {
            for (const auto& e : collector_->getCollectedEntities()) {
                if (!excludeComponents_.matches(e)) {
                    entityBuffer_.push_back(e);
                }
            }
        } else {
            for (const auto& e : collector_->getCollectedEntities()) {
                entityBuffer_.push_back(e);
            }
        }

        collector_->clearCollectedEntities();
        
        if (!entityBuffer_.empty()) {
            subsystem_->execute(entityBuffer_);
            entityBuffer_.clear();

            // FIXME we are already clearing?!
            if (clearAfterExecute_) {
                collector_->clearCollectedEntities();
            }
        }
    }
}
}
