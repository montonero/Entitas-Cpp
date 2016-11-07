// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#pragma once

#include "IComponent.hpp"
#include <vector>

namespace entitas
{
typedef unsigned int ComponentId;
typedef std::vector<ComponentId> ComponentIdList;

struct ComponentTypeId
{
	public:
		template<typename T>
		static const ComponentId get()
		{
			static_assert((std::is_base_of<IComponent, T>::value && ! std::is_same<IComponent, T>::value),
				"Class type must be derived from IComponent");

			static ComponentId id = mCounter++;
			return id;
		}

		static unsigned int count()
		{
			return mCounter;
		}

	private:
		static unsigned int mCounter;
};
}
