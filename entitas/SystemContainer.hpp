// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ISystem.hpp"
#include "Context.hpp"
#include <vector>

namespace entitas {
class SystemContainer : public IInitializeSystem, public IExecuteSystem, public ICleanupSystem, public ITearDownSystem {
public:
    SystemContainer() = default;

    auto add(std::shared_ptr<ISystem> system) -> SystemContainer*;
    template <typename T>
    inline auto add() -> SystemContainer*;
    template <typename T>
    inline SystemContainer* addCreate(std::shared_ptr<Context> context);

    void initialize() override;
    void execute() override;
    void cleanup() override;
    void teardown() override;
    //void fixedExecute();

    void activateReactiveSystems();
    /// Deactivates all ReactiveSystems in the systems list.
    /// This will also clear all ReactiveSystems.
    /// This is useful when you want to soft-restart your application and
    /// want to reuse your existing system instances.
    void deactivateReactiveSystems();
    void clearReactiveSystems();

private:
    template <typename T>
    using SystemsVector = std::vector<std::shared_ptr<T>>;
    SystemsVector<IInitializeSystem> initializeSystems_;
    SystemsVector<IExecuteSystem> executeSystems_;
    SystemsVector<ICleanupSystem> cleanupSystems_;
    SystemsVector<ITearDownSystem> teardownSystems_;
};

template <typename T>
auto SystemContainer::add() -> SystemContainer*
{
    return add(std::make_shared<T>());
}

template <typename T>
SystemContainer* SystemContainer::addCreate(std::shared_ptr<Context> context)
{
    return add(context->createSystem<T>());
}

} // namespace entitas
