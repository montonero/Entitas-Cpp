#include "EntitasPP/SystemContainer.hpp"
#include "EntitasPP/Matcher.hpp"
#include "EntitasPP/Pool.hpp"
#include "EntitasPP/ISystem.hpp"
#include <iostream>

#include "Rectangle.h"

using namespace EntitasPP;

class DemoComponent : public IComponent
{
public:
  void Reset(const std::string& name1, const std::string& name2)
  {
    std::cout << "Created new entity: " << name1 << "," << name2 << std::endl;
  }
};

class DemoSystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem
{
public:
  void SetPool(Pool* pool)
  {
    mPool = pool;
  }

  void Initialize()
  {
    mPool->CreateEntity()->Add<DemoComponent>("foo", "bar");
    std::cout << "DemoSystem initialized" << std::endl;
  }

  void Execute()
  {
    mPool->CreateEntity()->Add<DemoComponent>("foo", "bar");

    auto entitiesCount = mPool->GetGroup(Matcher_AllOf(DemoComponent))->Count();
    std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;

    std::cout << "DemoSystem executed" << std::endl;
  }

private:
  Pool* mPool;
};

class Position : public IComponent
{
public:
  // You must provide at least ONE public "Reset" method with any parameters you want
  void Reset(float px, float py, float pz)
  {
    x = px;
    y = py;
    z = pz;
  }

  float x;
  float y;
  float z;
};


class Move : public IComponent
{
public:
  void Reset(Vec2 d, float s)
  {
    direction = d;
    speed = s;
  }

  Vec2 direction;
  float speed {0.f};
};

class MoveSystem : public IExecuteSystem, public ISetPoolSystem
{
    std::shared_ptr<Group> _group;

    public:
        void SetPool(Pool* pool) {
            _group = pool->GetGroup(Matcher_AllOf(Move, Position));
        }

        void Execute() {
            for (auto &e : _group->GetEntities()) {
                auto move = e->Get<Move>();
                auto pos = e->Get<Position>();
                e->Replace<Position>(pos->x, pos->y + move->speed, pos->z);
            }
        }
};



int main(const int argc, const char* argv[])
{
  auto systems = std::make_shared<SystemContainer>();
  auto pool = std::make_shared<Pool>();

  systems->Add(pool->CreateSystem<DemoSystem>());
  systems->Initialize();

  for(unsigned int i = 0; i < 2; ++i) {
    systems->Execute();
  }

  return 0;
}
