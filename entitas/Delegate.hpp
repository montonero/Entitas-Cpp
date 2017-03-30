// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

// Based on:
// https://gist.github.com/sim642/4525268

#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "Functional.hpp"

namespace entitas {
template <typename>
class Delegate;

/* -------------------------------------------------------------------------- */

namespace detail {

    template <typename TReturnType, typename... TArgs>
    struct Invoker {
        using ReturnType = std::vector<TReturnType>;

    public:
        static ReturnType invoke(Delegate<TReturnType(TArgs...)>& delegate,
            TArgs... params)
        {
            std::lock_guard<std::mutex> lock(delegate.mutex_);
            ReturnType returnValues;

            for (const auto& functionPtr : delegate.functionList_) {
                returnValues.push_back((*functionPtr)(params...));
            }

            return returnValues;
        }
    };

    /* -------------------------------------------------------------------------- */

    template <typename... TArgs>
    struct Invoker<void, TArgs...> {
        using ReturnType = void;

    public:
        static void invoke(Delegate<void(TArgs...)>& delegate, TArgs... params)
        {
            std::lock_guard<std::mutex> lock(delegate.mutex_);
            if (delegate.functionList_.empty())
                return;
            for_each(delegate.functionList_,
                [&](auto& fpair) {
                    auto& f = fpair.second;
                    f(params...);
                });
        }
    };
} // namespace detail

/* -------------------------------------------------------------------------- */

template <typename TReturnType, typename... TArgs>
class Delegate<TReturnType(TArgs...)> {
    using Invoker = detail::Invoker<TReturnType, TArgs...>;
    using FunctionType = std::function<TReturnType(TArgs...)>;
    // Use an id alongside function so that we can identify them for later removal
    using FunctionPair = std::pair<size_t, FunctionType>;
    // using FunctionPointer = std::shared_ptr<FunctionType>;
    using FunctionPointer = FunctionType;
    friend Invoker;

public:
public:
    Delegate() {}
    ~Delegate() {}

    Delegate(const Delegate&) = delete;
    const Delegate& operator=(const Delegate&) = delete;

    // Delegate& Connect(const FunctionType& function)
    Delegate& Connect(FunctionPair&& function)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functionList_.emplace_back(function);
        return *this;
    }

    // Delegate& remove(const FunctionType& function)
    Delegate& remove(const FunctionPair function)
    {
        using namespace std;
        lock_guard<mutex> lock(mutex_);
        if (!functionList_.empty())
            functionList_.erase(remove_if(begin(functionList_), end(functionList_),
                                    bind(areEqual1, function.first, placeholders::_1)),
                end(functionList_));

        return *this;
    }

    inline typename Invoker::ReturnType invoke(TArgs... args)
    {
        return Invoker::invoke(*this, args...);
    }

    Delegate& clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functionList_.clear();
        return *this;
    }

    // inline Delegate& operator+=(const FunctionType& function)
    inline Delegate& operator+=(FunctionPair&& function)
    {
        return Connect(std::move(function));
    }

    // inline Delegate& operator-=(const FunctionType& function)
    inline Delegate& operator-=(const FunctionPair function)
    {
        return remove(function);
    }

    inline typename Invoker::ReturnType operator()(TArgs... args)
    {
        return Invoker::invoke(*this, args...);
    }
    static inline bool areEqual1(size_t ind1, const FunctionPair& f2)
    {
        return ind1 == f2.first;
    }
    static inline bool areEqual(const FunctionPair& f1, const FunctionPair& f2)
    {
        return f1.first == f2.first;
    }

    static inline constexpr size_t hash(const FunctionType& function)
    {
        return function.target_type().hash_code();
    }

private:
    std::mutex mutex_;
    std::vector<FunctionPair> functionList_;
};
}
