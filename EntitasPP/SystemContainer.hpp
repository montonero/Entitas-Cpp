// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "ISystem.hpp"
#include <vector>

namespace EntitasPP
{
class SystemContainer : public IinitializeSystem, public IexecuteSystem, public IfixedExecuteSystem
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
		std::vector<std::shared_ptr<IinitializeSystem>> mInitializeSystems;
		std::vector<std::shared_ptr<IexecuteSystem>> mExecuteSystems;
		std::vector<std::shared_ptr<IfixedExecuteSystem>> mFixedExecuteSystems;
};

template <typename T>
auto SystemContainer::add() -> SystemContainer*
{
	return add(std::shared_ptr<T>(new T()));
}
}
