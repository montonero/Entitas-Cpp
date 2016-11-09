// Copyright (c) 2016 Juan Delgado (JuDelCo)
// License: MIT License
// MIT License web page: https://opensource.org/licenses/MIT

#include "entitas/SystemContainer.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include <iostream>

#include <string>

using namespace entitas;

class DemoComponent : public IComponent {
public:
  void reset(const std::string& name1, const std::string& name2) {
    std::cout << "Created new entity: " << name1 << "," << name2 << std::endl;
  }
};

class DemoSystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem {
public:
  void setPool(Pool* pool) {
    mPool = pool;
  }
  void initialize() {
    mPool->createEntity()->add<DemoComponent>("foo", "bar");
    std::cout << "DemoSystem initialized" << std::endl;
  }
  void execute() {
    mPool->createEntity()->add<DemoComponent>("foo", "bar");

    auto entitiesCount = mPool->getGroup(Matcher_allOf(DemoComponent))->count();
    std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;

    std::cout << "DemoSystem executed" << std::endl;
  }

private:
  Pool* mPool;
};

int main(const int argc, const char* argv[]) {
  auto systems = std::make_shared<SystemContainer>();
  auto pool = std::make_shared<Pool>();

  systems->add(pool->createSystem<DemoSystem>());
  systems->initialize();

  for(unsigned int i = 0; i < 2; ++i) {
    systems->execute();
  }

  return 0;
}
