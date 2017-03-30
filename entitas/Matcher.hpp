// Copyright (c) 2017 Igor M
// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "Entity.hpp"

namespace entitas
{
    class Matcher;
    class TriggerOnEvent;
    typedef std::vector<Matcher> MatcherList;

    class Matcher
    {
    public:
        Matcher() = default;
        static Matcher allOf(const ComponentIdList indices);
        static auto allOf(const MatcherList matchers) -> const Matcher;
        static auto anyOf(const ComponentIdList indices) -> const Matcher;
        static auto anyOf(const MatcherList matchers) -> const Matcher;
        static auto noneOf(const ComponentIdList indices) -> const Matcher;
        static auto noneOf(const MatcherList matchers) -> const Matcher;

        bool isEmpty() const;
        bool matches(const EntityPtr& entity);
        auto getIndices() -> const ComponentIdList&;
        auto getAllOfIndices() const -> const ComponentIdList;
        auto getAnyOfIndices() const -> const ComponentIdList;
        auto getNoneOfIndices() const -> const ComponentIdList;

        auto getHashCode() const -> unsigned int;
        bool compareIndices(const Matcher& matcher) const;

        auto onEntityAdded() -> const TriggerOnEvent;
        auto onEntityRemoved() -> const TriggerOnEvent;
        auto onEntityAddedOrRemoved() -> const TriggerOnEvent;

        bool operator ==(const Matcher right) const;

    protected:
        void calculateHash();

        ComponentIdList indices_;
        ComponentIdList indicesAllOf_;
        ComponentIdList indicesAnyOf_;
        ComponentIdList indicesNoneOf_;

    private:
        auto applyHash(unsigned int hash, const ComponentIdList indices, int i1, int i2) const -> unsigned int;
        auto mergeIndices() const -> ComponentIdList;
        static auto mergeIndices(MatcherList matchers) -> ComponentIdList;
        static auto distinctIndices(ComponentIdList indices) -> ComponentIdList;

        unsigned int hashCached_{0};
    };
}

namespace std
{
    template <>
    struct hash<entitas::Matcher>
    {
	std::size_t operator()(const entitas::Matcher& matcher) const
	{
            return hash<unsigned int>()(matcher.getHashCode());
	}
    };
}

namespace
{
#define FUNC_1(MODIFIER, X) MODIFIER(X)
#define FUNC_2(MODIFIER, X, ...) MODIFIER(X), FUNC_1(MODIFIER, __VA_ARGS__)
#define FUNC_3(MODIFIER, X, ...) MODIFIER(X), FUNC_2(MODIFIER, __VA_ARGS__)
#define FUNC_4(MODIFIER, X, ...) MODIFIER(X), FUNC_3(MODIFIER, __VA_ARGS__)
#define FUNC_5(MODIFIER, X, ...) MODIFIER(X), FUNC_4(MODIFIER, __VA_ARGS__)
#define FUNC_6(MODIFIER, X, ...) MODIFIER(X), FUNC_5(MODIFIER, __VA_ARGS__)
#define GET_MACRO(_1, _2, _3, _4, _5, _6, NAME,...) NAME
#define FOR_EACH(MODIFIER,...) GET_MACRO(__VA_ARGS__, FUNC_6, FUNC_5, FUNC_4, FUNC_3, FUNC_2, FUNC_1)(MODIFIER, __VA_ARGS__)


#define Matcher_allOf(...) (entitas::Matcher)entitas::Matcher::allOf(std::vector<entitas::ComponentId>({ FOR_EACH(COMPONENT_GET_TYPE_ID, __VA_ARGS__) }))


#define Matcher_anyOf(...) (entitas::Matcher)entitas::Matcher::anyOf(std::vector<entitas::ComponentId>({ FOR_EACH(COMPONENT_GET_TYPE_ID, __VA_ARGS__) }))
#define Matcher_noneOf(...) (entitas::Matcher)entitas::Matcher::noneOf(std::vector<entitas::ComponentId>({ FOR_EACH(COMPONENT_GET_TYPE_ID, __VA_ARGS__) }))
}
