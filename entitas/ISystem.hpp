// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "Matcher.hpp"
#include "TriggerOnEvent.hpp"
#include <vector>

namespace entitas
{
class Pool;

class ISystem
{
	protected:
		ISystem() = default;

	public:
		virtual ~ISystem() = default;
};

class ISetPoolSystem
{
	protected:
		ISetPoolSystem() = default;

	public:
		virtual ~ISetPoolSystem() = default;

		virtual void setPool(Pool* pool) = 0;
};

class IInitializeSystem
{
	protected:
		IInitializeSystem() = default;

	public:
		virtual ~IInitializeSystem() = default;

		virtual void initialize() = 0;
};

class IExecuteSystem : public ISystem
{
	protected:
		IExecuteSystem() = default;

	public:
		virtual ~IExecuteSystem() = default;

		virtual void execute() = 0;
};

class IFixedExecuteSystem : public ISystem
{
	protected:
		IFixedExecuteSystem() = default;

	public:
		virtual ~IFixedExecuteSystem() = default;

		virtual void fixedExecute() = 0;
};

class IReactiveExecuteSystem : public ISystem
{
	protected:
		IReactiveExecuteSystem() = default;

	public:
		virtual ~IReactiveExecuteSystem() = default;

		virtual void execute(std::vector<EntityPtr>& entities) = 0;
};

class IReactiveSystem : public IReactiveExecuteSystem
{
	public:
		virtual ~IReactiveSystem() = default;

		TriggerOnEvent trigger;
};

class IMultiReactiveSystem : public IReactiveExecuteSystem
{
	public:
		virtual ~IMultiReactiveSystem() = default;

		std::vector<TriggerOnEvent> triggers;
};

class IEnsureComponents
{
	protected:
		IEnsureComponents() = default;

	public:
		Matcher ensureComponents;
};

class IExcludeComponents
{
	protected:
		IExcludeComponents() = default;

	public:
		Matcher excludeComponents;
};

class IClearReactiveSystem
{
	protected:
		IClearReactiveSystem() = default;
};
}
