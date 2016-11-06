// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

// Based on:
// https://gist.github.com/sim642/4525268

#pragma once

#include <mutex>
#include <list>
#include <vector>
#include <memory>

namespace EntitasPP
{
template<typename>
class Delegate;

namespace DelegateImpl
{
template <typename TReturnType, typename... TArgs>
struct invoker
{
	using ReturnType = std::vector<TReturnType>;

	public:
		static ReturnType invoke(Delegate<TReturnType(TArgs...)> &delegate, TArgs... params)
		{
			std::lock_guard<std::mutex> lock(delegate.mMutex);
			ReturnType returnValues;

			for (const auto &functionPtr : delegate.mFunctionList)
			{
				returnValues.push_back((*functionPtr)(params...));
			}

			return returnValues;
		}
};

template <typename... TArgs>
struct invoker<void, TArgs...>
{
	using ReturnType = void;

	public:
		static void invoke(Delegate<void(TArgs...)> &delegate, TArgs... params)
		{
			std::lock_guard<std::mutex> lock(delegate.mMutex);

			for (const auto &functionPtr : delegate.mFunctionList)
			{
				(*functionPtr)(params...);
			}
		}
};
}

template<typename TReturnType, typename... TArgs>
class Delegate<TReturnType(TArgs...)>
{
	using invoker = DelegateImpl::invoker<TReturnType, TArgs...>;
	using functionType = std::function<TReturnType(TArgs...)>;

	friend invoker;

	public:
		Delegate() {}
		~Delegate() {}

		Delegate(const Delegate&) = delete;
		const Delegate& operator =(const Delegate&) = delete;

		Delegate& Connect(const functionType &function)
		{
			std::lock_guard<std::mutex> lock(this->mMutex);

			this->mFunctionList.push_back(std::make_shared<functionType>(function));

			return *this;
		}

		Delegate& remove(const functionType &function)
		{
			std::lock_guard<std::mutex> lock(this->mMutex);

			this->mFunctionList.remove_if([&](std::shared_ptr<functionType> &functionPtr)
			{
				return hash(function) == hash(*functionPtr);
			});

			return *this;
		}

		inline typename Invoker::ReturnType invoke(TArgs... args)
		{
			return Invoker::invoke(*this, args...);
		}

		Delegate& clear()
		{
			std::lock_guard<std::mutex> lock(this->mMutex);

			this->mFunctionList.clear();

			return *this;
		}

		inline Delegate& operator +=(const functionType &function)
		{
			return Connect(function);
		}

		inline Delegate& operator -=(const functionType &function)
		{
			return remove(function);
		}

		inline typename invoker::ReturnType operator ()(TArgs... args)
		{
			return Invoker::invoke(*this, args...);
		}

	private:
		std::mutex mMutex;
		std::list<std::shared_ptr<functionType>> mFunctionList;

		inline constexpr size_t hash(const functionType &function) const
		{
			return function.target_type().hash_code();
		}
};
}
