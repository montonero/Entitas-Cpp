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

namespace entitas {
template <typename>
class Delegate;

/* -------------------------------------------------------------------------- */

namespace DelegateImpl {
    template <typename TReturnType, typename... TArgs>
    struct Invoker {
        using ReturnType = std::vector<TReturnType>;

    public:
        static ReturnType invoke(Delegate<TReturnType(TArgs...)>& delegate,
            TArgs... params)
        {
            std::lock_guard<std::mutex> lock(delegate.mMutex);
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
            std::lock_guard<std::mutex> lock(delegate.mMutex);

            for (const auto& functionPtr : delegate.functionList_) {
                functionPtr(params...);
            }
        }
    };
}

/* -------------------------------------------------------------------------- */

template <typename TReturnType, typename... TArgs>
class Delegate<TReturnType(TArgs...)> {
    using Invoker = DelegateImpl::Invoker<TReturnType, TArgs...>;
    using FunctionType = std::function<TReturnType(TArgs...)>;
    // using FunctionPointer = std::shared_ptr<FunctionType>;
    using FunctionPointer = FunctionType;
    friend Invoker;

public:
public:
    Delegate() {}
    ~Delegate() {}

    Delegate(const Delegate&) = delete;
    const Delegate& operator=(const Delegate&) = delete;

    Delegate& Connect(const FunctionType& function)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        functionList_.push_back(function);
        return *this;
    }

    Delegate& remove(const FunctionType& function)
    {
        using namespace std;
        std::lock_guard<std::mutex> lock(mMutex);
        if (functionList_.size() > 0)
            functionList_.erase(remove_if(begin(functionList_), end(functionList_),
                bind(areEqual, function, placeholders::_1)
                // [&](FunctionPointer functionPtr) {
                // auto h1 = hash(function);
                // auto h2 = hash(functionPtr);
                // return h1 == h2;
                // }
                ));

        return *this;
    }

    inline typename Invoker::ReturnType invoke(TArgs... args)
    {
        return Invoker::invoke(*this, args...);
    }

    Delegate& clear()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        functionList_.clear();
        return *this;
    }

    inline Delegate& operator+=(const FunctionType& function)
    {
        return Connect(function);
    }

    inline Delegate& operator-=(const FunctionType& function)
    {
        return remove(function);
    }

    inline typename Invoker::ReturnType operator()(TArgs... args)
    {
        return Invoker::invoke(*this, args...);
    }
    static inline bool areEqual(const FunctionType& f1, const FunctionType& f2)
    {
        return (hash(f1) == hash(f2));
        FunctionType& ff1 = const_cast<FunctionType&>(f1);
        auto h = ff1.template target<FunctionType>();
        auto h2 = f2.template target<FunctionType>();
        return h == h2;
        if (hash(f1) == hash(f2)) {
        }
    }

    static inline constexpr size_t hash(const FunctionType& function)
    {
        return function.target_type().hash_code();
    }


private:
    std::mutex mMutex;
    std::vector<FunctionPointer> functionList_;
};
}
