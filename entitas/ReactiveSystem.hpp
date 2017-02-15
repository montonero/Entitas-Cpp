// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Collector.hpp"
#include "ISystem.hpp"

namespace entitas
{
class ReactiveSystem : public IExecuteSystem
{
	public:
		ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveSystem> subsystem);
		ReactiveSystem(Pool* pool, std::shared_ptr<IMultiReactiveSystem> subsystem);
		ReactiveSystem(Pool* pool, std::shared_ptr<IReactiveExecuteSystem> subsystem, std::vector<TriggerOnEvent> triggers);
		~ReactiveSystem();

		auto getSubsystem() const -> std::shared_ptr<IReactiveExecuteSystem>;
		void activate();
		void deactivate();
		void clear();
		void execute();

	private:
		std::shared_ptr<IReactiveExecuteSystem> mSubsystem;
		Collector* mObserver;
		Matcher mEnsureComponents;
		Matcher mExcludeComponents;
		bool mClearAfterExecute{false};
		std::vector<EntityPtr> mEntityBuffer;
};
}
