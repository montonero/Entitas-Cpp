// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Collector.hpp"
#include "ISystem.hpp"

namespace entitas {
class ReactiveSystem : public IExecuteSystem {
public:
    ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveSystem> subsystem);
    ReactiveSystem(Pool* pool, std::shared_ptr<IMultiReactiveSystem> subsystem);
    ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveExecuteSystem> subsystem, std::vector<TriggerOnEvent> triggers);
    ~ReactiveSystem();

    auto getSubsystem() const -> std::shared_ptr<IReactiveExecuteSystem>;
    /// Activates the ReactiveSystem and starts observing changes
    /// based on the specified Collector.
    /// ReactiveSystem are activated by default.
    void activate();
    /// Deactivates the ReactiveSystem.
    /// No changes will be tracked while deactivated.
    /// This will also clear the ReactiveSystem.
    /// ReactiveSystem are activated by default.
    void deactivate();
    /// Clears all accumulated changes.
    void clear();
    /// Will call execute(entities) with changed entities
    /// if there are any. Otherwise it will not call execute(entities).
    void execute();

private:
    std::shared_ptr<IReactiveExecuteSystem> mSubsystem;
    Collector* collector_{ nullptr };
    Matcher mEnsureComponents;
    Matcher mExcludeComponents;
    bool mClearAfterExecute{ false };
    std::vector<EntityPtr> mEntityBuffer;
};
}
