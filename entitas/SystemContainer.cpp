// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "SystemContainer.hpp"
#include "ReactiveSystem.hpp"

#include "Functional.hpp"

#include <memory>

namespace entitas {
using std::dynamic_pointer_cast;
auto SystemContainer::add(std::shared_ptr<ISystem> system) -> SystemContainer*
{
    if (auto systemReactive = dynamic_pointer_cast<ReactiveSystem>(system)) {
        if (auto initSystem = dynamic_pointer_cast<IInitializeSystem>(systemReactive->getSubsystem())) {
            initializeSystems_.emplace_back(initSystem);
        }
        if (auto cleanupSystem = dynamic_pointer_cast<ICleanupSystem>(systemReactive->getSubsystem())) {
            cleanupSystems_.emplace_back(cleanupSystem);
        }
        if (auto teardownSystem = dynamic_pointer_cast<ITearDownSystem>(systemReactive->getSubsystem())) {
            teardownSystems_.emplace_back(teardownSystem);
        }
    } else {
        if (auto systemInitialize = dynamic_pointer_cast<IInitializeSystem>(system)) {
            initializeSystems_.emplace_back(systemInitialize);
        }
        if (auto cleanupSystem = dynamic_pointer_cast<ICleanupSystem>(system)) {
            cleanupSystems_.emplace_back(cleanupSystem);
        }
        if (auto teardownSystem = dynamic_pointer_cast<ITearDownSystem>(system)) {
            teardownSystems_.emplace_back(teardownSystem);
        }
    }

    if (auto systemExecute = dynamic_pointer_cast<IExecuteSystem>(system)) {
        executeSystems_.push_back(systemExecute);
    }

    return this;
}

void SystemContainer::initialize()
{
    for_each(initializeSystems_, std::mem_fn(&IInitializeSystem::initialize));
}

void SystemContainer::execute()
{
    for_each(executeSystems_, std::mem_fn(&IExecuteSystem::execute));
}

void SystemContainer::cleanup()
{
    for_each(cleanupSystems_, std::mem_fn(&ICleanupSystem::cleanup));
}

void SystemContainer::teardown()
{
    for_each(teardownSystems_, std::mem_fn(&ITearDownSystem::teardown));
}

#if 0
void SystemContainer::fixedExecute()
{
    for (const auto& system : mFixedExecuteSystems) {
        system->fixedExecute();
    }
}
#endif

void SystemContainer::activateReactiveSystems()
{
    for (const auto& system : executeSystems_) {
        if (dynamic_pointer_cast<ReactiveSystem>(system) != nullptr) {
            (dynamic_pointer_cast<ReactiveSystem>(system))->activate();
        }

        if (dynamic_pointer_cast<SystemContainer>(system) != nullptr) {
            (dynamic_pointer_cast<SystemContainer>(system))->activateReactiveSystems();
        }
    }
}

void SystemContainer::deactivateReactiveSystems()
{
    for (const auto& system : executeSystems_) {
        if (auto systemReactive = dynamic_pointer_cast<ReactiveSystem>(system)) {
            systemReactive->deactivate();
        }

        if (auto systemContainer = dynamic_pointer_cast<SystemContainer>(system)) {
            systemContainer->deactivateReactiveSystems();
        }
    }
}

void SystemContainer::clearReactiveSystems()
{
    for_each(executeSystems_, [&](const auto& system) {
        if (auto systemReactive = dynamic_pointer_cast<ReactiveSystem>(system)) {
            systemReactive->clear();
        }

        if (auto systemContainer = dynamic_pointer_cast<SystemContainer>(system)) {
            systemContainer->clearReactiveSystems();
        }
    });
}
}
