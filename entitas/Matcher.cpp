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
		matcher.mAllOfIndices = distinctIndices(indices);
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
		matcher.mAnyOfIndices = distinctIndices(indices);
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
		matcher.mNoneOfIndices = distinctIndices(indices);
		matcher.calculateHash();

		return matcher;
    }

    auto Matcher::noneOf(const MatcherList matchers) -> const Matcher
    {
		return Matcher::noneOf(mergeIndices(matchers));
    }

    bool Matcher::isEmpty() const
    {
		return (mAllOfIndices.empty() && mAnyOfIndices.empty() && mNoneOfIndices.empty());
    }

    bool Matcher::matches(const EntityPtr& entity)
    {
		auto matchesAllOf = mAllOfIndices.empty() || entity->hasComponents(mAllOfIndices);
		auto matchesAnyOf = mAnyOfIndices.empty() || entity->hasAnyComponent(mAnyOfIndices);
		auto matchesNoneOf = mNoneOfIndices.empty() || ! entity->hasAnyComponent(mNoneOfIndices);

		return matchesAllOf && matchesAnyOf && matchesNoneOf;
    }

    auto Matcher::getIndices() -> const ComponentIdList
    {
		if(mIndices.empty())
		{
			mIndices = mergeIndices();
		}

		return mIndices;
    }

    auto Matcher::getAllOfIndices() const -> const ComponentIdList
    {
		return mAllOfIndices;
    }

    auto Matcher::getAnyOfIndices() const -> const ComponentIdList
    {
		return mAnyOfIndices;
    }

    auto Matcher::getNoneOfIndices() const -> const ComponentIdList
    {
		return mNoneOfIndices;
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
		indicesList.reserve(mAllOfIndices.size() + mAnyOfIndices.size() + mNoneOfIndices.size());

		for(const auto &id : mAllOfIndices)
		{
			indicesList.push_back(id);
		}

		for(const auto &id : mAnyOfIndices)
		{
			indicesList.push_back(id);
		}

		for(const auto &id : mNoneOfIndices)
		{
			indicesList.push_back(id);
		}

		return distinctIndices(indicesList);
    }

    void Matcher::calculateHash()
    {
		unsigned int hash = (unsigned)typeid(Matcher).hash_code();

		hash = applyHash(hash, mAllOfIndices, 3, 53);
		hash = applyHash(hash, mAnyOfIndices, 307, 367);
		hash = applyHash(hash, mNoneOfIndices, 647, 683);

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

	// Old Code (delete!)
	/*auto indicesSet = unordered_set<unsigned int>(indices.begin(), indices.end());

          auto uniqueIndices = ComponentIdList();
          uniqueIndices.reserve(indicesSet.size());

          for(const auto &id : indicesSet)
          {
          uniqueIndices.push_back(id);
          }

          std::sort(uniqueIndices.begin(), uniqueIndices.end(), [](unsigned int a, unsigned int b) {
          return b < a;
          });

          return uniqueIndices;*/
    }
}
