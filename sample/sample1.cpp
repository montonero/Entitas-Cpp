#include "entitas/ISystem.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include "entitas/SystemContainer.hpp"
#include <iostream>

#include <string>

#include "Rectangle.h"

using namespace entitas;

class DemoComponent : public IComponent {
public:
    void reset(const std::string& name1, const std::string& name2)
    {
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

class Position : public IComponent {
public:
    // You must provide at least ONE public "Reset" method with any parameters you want
    void reset(Vec2 pos)
    {
        pos_ = pos;
    }

    Vec2 pos_;
};

class Move : public IComponent {
public:
    void reset(Vec2 d, float s)
    {
        direction = d;
        speed = s;
    }

    Vec2 direction;
    float speed{ 0.f };
};

struct RenderComponent : public IComponent {
    void reset(Material m) { material = m; }
    Material material;
};

class MoveSystem : public IExecuteSystem, public ISetPoolSystem {
    Group::SharedPtr _group;

public:
    void setPool(Pool* pool)
    {
        //_group = pool->getGroup(Matcher_allOf(Move, Position));
        auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(Move), COMPONENT_GET_TYPE_ID(Position) });
        _group = pool->getGroup(matcher);
    }

    void execute()
    {
        for (auto& e : _group->getEntities()) {
            auto move = e->get<Move>();
            auto pos = e->get<Position>()->pos_;
            e->replace<Position>(Vec2{ pos.x(), pos.y() + move->speed });
        }
    }
};


int main(const int argc, const char* argv[])
{
    auto systems = std::make_shared<SystemContainer>();
    auto pool = std::make_shared<Pool>();

    systems->add(pool->createSystem<DemoSystem>());
    systems->initialize();

    for (unsigned int i = 0; i < 2; ++i) {
        systems->execute();
    }

#ifndef WIN32
    auto entities = pool->getEntities(Matcher_allOf(RenderComponent, Position)); // *Some magic preprocessor involved*
    for (auto& e : entities) { // e is a shared_ptr of Entity
        // do something
    }
#endif
    return 0;
}
