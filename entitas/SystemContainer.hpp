// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ISystem.hpp"
#include <vector>

namespace entitas
{
    class SystemContainer : public IInitializeSystem, public IExecuteSystem, public IFixedExecuteSystem
    {
    public:
        SystemContainer() = default;

        auto add(std::shared_ptr<ISystem> system) -> SystemContainer*;
        template <typename T> inline auto add() -> SystemContainer*;

        void initialize();
        void execute();
        void fixedExecute();

        void activateReactiveSystems();
        void deactivateReactiveSystems();
        void clearReactiveSystems();

    private:
        std::vector<std::shared_ptr<IInitializeSystem>> initializeSystems_;
        std::vector<std::shared_ptr<IExecuteSystem>> executeSystes_;
        std::vector<std::shared_ptr<IFixedExecuteSystem>> mFixedExecuteSystems;
    };

    template <typename T>
    auto SystemContainer::add() -> SystemContainer*
    {
	return add(std::shared_ptr<T>(new T()));
    }
}
