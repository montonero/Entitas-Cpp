// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "entitas/SystemContainer.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include <iostream>

#include <string>

using namespace entitas;

/* -------------------------------------------------------------------------- */

class DemoComponent : public IComponent {
public:
  void reset(const std::string& name1, const std::string& name2) {
    std::cout << "Created new entity: " << name1 << "," << name2 << std::endl;
  }
};

class DemoSystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem {
public:
  void setPool(Pool* pool)
  {
    mPool = pool;
  }
  void initialize()
  {
    mPool->createEntity()->add<DemoComponent>("foo", "bar");
    std::cout << "DemoSystem initialized" << std::endl;
  }

  void execute()
  {
    mPool->createEntity()->add<DemoComponent>("foo", "bar");
    auto entitiesCount = mPool->getGroup(Matcher_allOf(DemoComponent))->count();
    std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;
    std::cout << "DemoSystem executed" << std::endl;
  }

private:
  Pool* mPool;
};


/* -------------------------------------------------------------------------- */


class Position : public IComponent
{
public:
	// You must provide at least ONE public "Reset" method with any parameters you want
	void reset(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	float x;
	float y;
	float z;
};

class Position2 : public IComponent
{
public:
	// You must provide at least ONE public "Reset" method with any parameters you want
	void reset(float px, float py)
	{
		x = px;
		y = py;
	}

	float x;
	float y;
	float z;
};



class MySystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem
{
public:
	void setPool(Pool* pool)
	{
		pool_ = pool;
		// #define COMPONENT_GET_TYPE_ID(COMPONENT_CLASS) EntitasPP::ComponentTypeId::Get<COMPONENT_CLASS>()
		// #define Matcher_AllOf(...) (EntitasPP::Matcher)EntitasPP::Matcher::AllOf(std::vector<EntitasPP::ComponentId>({ FOR_EACH(COMPONENT_GET_TYPE_ID, __VA_ARGS__) }))
		using namespace entitas;
		//group_ = pool_->GetGroup(Matcher_AllOf(Position, Position2));
		
		//group_ = pool_->getGroup(Matcher::allOf(std::vector<ComponentId>({ ComponentTypeId::get<Position>(), ComponentTypeId::get<Position2>() })));
		std::cout << "MySystem::setPool called" << std::endl;
	}

	void initialize()
	{
		auto e = pool_->createEntity();
		e->add<Position2>(1, 1);
		e->add<Position>(10, 10, 10);

		std::cout << "MySystem initialized" << std::endl;
	}

	void execute()
	{
		// pool_->createEntity()->add<DemoComponent>("foo", "bar");
		// pool_->createEntity()->add<DemoComponent>("foo", "bar");

		// auto entitiesCount = pool_->getGroup(Matcher_allOf(DemoComponent))->count();
		//auto es = pool_->GetEntities(Matcher_AllOf(Position, Position2));
		group_ = pool_->getGroup(Matcher::allOf(std::vector<ComponentId>({ ComponentTypeId::get<Position>(), ComponentTypeId::get<Position2>() })));
		auto es = group_->getEntities();
		for (auto &e : es)
		{
			auto mat = e->get<Position2>();
			auto pos = e->get<Position>();
			//renderMat(gSdlRenderer, mat->material.color, Vec2(pos->x, pos->y));
		}
		// std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;

		// std::cout << "DemoSystem executed" << std::endl;
	}

private:
	Pool* pool_{ nullptr };
	std::shared_ptr<Group> group_;
};

/* -------------------------------------------------------------------------- */

int main(const int argc, const char* argv[])
{
  auto systems = std::make_shared<SystemContainer>();
  auto pool = std::make_shared<Pool>();

  systems->add(pool->createSystem<DemoSystem>());
  systems->add(pool->createSystem<MySystem>());
  systems->initialize();

  for(unsigned int i = 0; i < 2; ++i)
  {
    systems->execute();
  }

  return 0;
}
