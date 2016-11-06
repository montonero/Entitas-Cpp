// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"
#include "Matcher.hpp"
#include "TriggerOnEvent.hpp"
#include <vector>

namespace EntitasPP
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

		virtual void SetPool(Pool* pool) = 0;
};

class IinitializeSystem
{
	protected:
		IinitializeSystem() = default;

	public:
		virtual ~IinitializeSystem() = default;

		virtual void initialize() = 0;
};

class IexecuteSystem : public ISystem
{
	protected:
		IexecuteSystem() = default;

	public:
		virtual ~IexecuteSystem() = default;

		virtual void execute() = 0;
};

class IfixedExecuteSystem : public ISystem
{
	protected:
		IfixedExecuteSystem() = default;

	public:
		virtual ~IfixedExecuteSystem() = default;

		virtual void fixedExecute() = 0;
};

class IReactiveExecuteSystem : public ISystem
{
	protected:
		IReactiveExecuteSystem() = default;

	public:
		virtual ~IReactiveExecuteSystem() = default;

		virtual void execute(std::vector<EntityPtr> entities) = 0;
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

class IclearReactiveSystem
{
	protected:
		IclearReactiveSystem() = default;
};
}
