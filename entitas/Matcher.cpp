// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "Matcher.hpp"
#include "TriggerOnEvent.hpp"
#include <algorithm>

namespace entitas
{
	Matcher Matcher::allOf(const ComponentIdList indices)
    {
		Matcher matcher;
		matcher.indicesAllOf_ = distinctIndices(indices);
		matcher.calculateHash();

		return matcher;
    }

    auto Matcher::allOf(const MatcherList matchers) -> const Matcher
    {
		return Matcher::allOf(mergeIndices(matchers));
    }

    auto Matcher::anyOf(const ComponentIdList indices) -> const Matcher
    {
		auto matcher = Matcher();
		matcher.indicesAnyOf_ = distinctIndices(indices);
		matcher.calculateHash();

		return matcher;
    }

    auto Matcher::anyOf(const MatcherList matchers) -> const Matcher
    {
		return Matcher::anyOf(mergeIndices(matchers));
    }

    auto Matcher::noneOf(const ComponentIdList indices) -> const Matcher
    {
		auto matcher = Matcher();
		matcher.indicesNoneOf_ = distinctIndices(indices);
		matcher.calculateHash();

		return matcher;
    }

    auto Matcher::noneOf(const MatcherList matchers) -> const Matcher
    {
		return Matcher::noneOf(mergeIndices(matchers));
    }

    bool Matcher::isEmpty() const
    {
		return (indicesAllOf_.empty() && indicesAnyOf_.empty() && indicesNoneOf_.empty());
    }

    bool Matcher::matches(const EntityPtr& entity)
    {
		auto matchesAllOf = indicesAllOf_.empty() || entity->hasComponents(indicesAllOf_);
		auto matchesAnyOf = indicesAnyOf_.empty() || entity->hasAnyComponent(indicesAnyOf_);
		auto matchesNoneOf = indicesNoneOf_.empty() || ! entity->hasAnyComponent(indicesNoneOf_);

		return matchesAllOf && matchesAnyOf && matchesNoneOf;
    }

    auto Matcher::getIndices() -> const ComponentIdList&
    {
		if(indices_.empty())
		{
			indices_ = mergeIndices();
		}

		return indices_;
    }

    auto Matcher::getAllOfIndices() const -> const ComponentIdList
    {
		return indicesAllOf_;
    }

    auto Matcher::getAnyOfIndices() const -> const ComponentIdList
    {
		return indicesAnyOf_;
    }

    auto Matcher::getNoneOfIndices() const -> const ComponentIdList
    {
		return indicesNoneOf_;
    }

    auto Matcher::getHashCode() const -> unsigned int
    {
		return mCachedHash;
    }

    bool Matcher::compareIndices(const Matcher& matcher) const
    {
		if(matcher.isEmpty())
		{
			return false;
		}

		auto leftIndices = this->mergeIndices();
		auto rightIndices = matcher.mergeIndices();

		if(leftIndices.size() != rightIndices.size())
		{
			return false;
		}

        for(size_t i = 0, count = leftIndices.size(); i < count; ++i)
		{
			if(leftIndices[i] != rightIndices[i])
			{
				return false;
			}
		}

		return true;
    }

    // TODO need to implement component related changes
    auto Matcher::onEntityAdded() -> const TriggerOnEvent
    {
		return TriggerOnEvent(*this, GroupEventType::OnEntityAdded);
    }

    auto Matcher::onEntityRemoved() -> const TriggerOnEvent
    {
		return TriggerOnEvent(*this, GroupEventType::OnEntityRemoved);
    }

    auto Matcher::onEntityAddedOrRemoved() -> const TriggerOnEvent
    {
		return TriggerOnEvent(*this, GroupEventType::OnEntityAddedOrRemoved);
    }

    bool Matcher::operator ==(const Matcher right) const
    {
		return this->getHashCode() == right.getHashCode() && this->compareIndices(right);
    }

    auto Matcher::mergeIndices() const -> ComponentIdList
    {
		ComponentIdList indicesList;
		indicesList.reserve(indicesAllOf_.size() + indicesAnyOf_.size() + indicesNoneOf_.size());

		for(const auto &id : indicesAllOf_)
		{
			indicesList.push_back(id);
		}

		for(const auto &id : indicesAnyOf_)
		{
			indicesList.push_back(id);
		}

		for(const auto &id : indicesNoneOf_)
		{
			indicesList.push_back(id);
		}

		return distinctIndices(indicesList);
    }

    void Matcher::calculateHash()
    {
		unsigned int hash = (unsigned)typeid(Matcher).hash_code();

		hash = applyHash(hash, indicesAllOf_, 3, 53);
		hash = applyHash(hash, indicesAnyOf_, 307, 367);
		hash = applyHash(hash, indicesNoneOf_, 647, 683);

		mCachedHash = hash;
    }

    auto Matcher::applyHash(unsigned int hash, const ComponentIdList indices, int i1, int i2) const -> unsigned int
    {
		if (indices.size() > 0)
		{
			for (size_t i = 0, indicesLength = indices.size(); i < indicesLength; i++)
			{
				hash ^= indices[i] * i1;
			}

			hash ^= indices.size() * i2;
		}

		return hash;
    }

    auto Matcher::mergeIndices(MatcherList matchers) -> ComponentIdList
    {
		unsigned int totalIndices = 0;

		for(auto &matcher : matchers)
		{
			totalIndices += matcher.getIndices().size();
		}

		auto indices = ComponentIdList();
		indices.reserve(totalIndices);

		for(auto &matcher : matchers)
		{
			for(const auto &id : matcher.getIndices())
			{
				indices.push_back(id);
			}
		}

		return indices;
    }

    auto Matcher::distinctIndices(ComponentIdList indices) -> ComponentIdList
    {
		std::sort(indices.begin(), indices.end());
		indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

		return indices;
    }
}
