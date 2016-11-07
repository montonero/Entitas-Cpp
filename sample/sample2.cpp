#include "entitas/SystemContainer.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include "entitas/ISystem.hpp"
#include <iostream>

#include "SDL.h"

#include "Rectangle.h"

using namespace entitas;

class DemoComponent : public IComponent
{
public:
  void reset(const std::string& name1, const std::string& name2)
  {
    std::cout << "Created new entity: " << name1 << "," << name2 << std::endl;
  }
};

class DemoSystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem
{
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


class Move : public IComponent
{
public:
  void reset(Vec2 d, float s)
  {
    direction = d;
    speed = s;
  }

  Vec2 direction;
  float speed {0.f};
};


struct RenderComponent : public IComponent
{
    void reset(Material m) { material = m; }
    Material material;
};


/* -------------------------------------------------------------------------- */

class MoveSystem : public IExecuteSystem, public ISetPoolSystem
{
    std::shared_ptr<Group> _group;

public:
    void setPool(Pool* pool)
    {
        _group = pool->getGroup(Matcher_allOf(Move, Position));
    }

    void execute()
    {
        for (auto &e : _group->getEntities())
        {
            auto move = e->get<Move>();
            auto pos = e->get<Position>();
            e->replace<Position>(pos->x, pos->y + move->speed, pos->z);
        }
    }
};

/* -------------------------------------------------------------------------- */
SDL_Rect toSdlRect(Rectangle r)
{
    SDL_Rect sr;
    sr.x = r.position.x;
    sr.y = r.position.y;
    sr.w = r.size.x;
    sr.h = r.size.y;
    return sr;
}


void renderRect(SDL_Renderer* renderer, Rectangle& r)
{
    auto sr = toSdlRect(r);
    SDL_SetRenderDrawColor(renderer, r.color.r, r.color.g, r.color.b, 255);
    SDL_RenderFillRect(renderer, &sr);
}  


/* -------------------------------------------------------------------------- */

/*
class RenderPositionSystem : public IReactiveSystem
{
    // std::unique_ptr<TriggerOnEvent> trigger;
    TriggerOnEvent trigger;

public:
    RenderPositionSystem()
    {
        trigger = Matcher_allOf(Position, RenderComponent)->onEntityAdded();
        // trigger.reset(Matcher_allOf(Position, RenderComponent)->onEntityAdded());
    }

    void execute(std::vector<EntityPtr> entities)
    {
        // Gets executed only if the observed group changed.
        // Changed entities are passed as an argument
        for (auto& e : entities)
        {
            auto pos = e->get<Position>();
            // NOTE: Unity-only example, but this maybe could be the code if Unity were compatible with C++
            // e->get<View>()->gameObject.transform.position = new Vector3(pos->x, pos->y, pos->z);
        }
    }
};
*/

/* -------------------------------------------------------------------------- */


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480


int mainLoop()
{

    int done;
    SDL_Event event;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL\n");
        return 1;
    }

    /* seed random number generator */
    srand(time(NULL));

    /* create window and renderer */
    auto window = SDL_CreateWindow(NULL, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) 
    {
        // printf("Could not initialize Window\n");
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    auto renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) 
    {
        printf("Could not create renderer\n");
        return 1;
    }

    //thread t(readInput);

    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done)
    {
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT) 
            {
                done = 1;
            }
        }
        // string text;
        // cin >> text;
        // render(renderer, rs);
        SDL_Delay(100);
    }
    return 0;
}

/* -------------------------------------------------------------------------- */


int main(const int argc, const char* argv[])
{
  auto systems = std::make_shared<SystemContainer>();
  auto pool = std::make_shared<Pool>();

  systems->add(pool->createSystem<DemoSystem>());
  systems->initialize();

  for(unsigned int i = 0; i < 2; ++i) {
    systems->execute();
  }

  auto entities = pool->getEntities(Matcher_allOf(RenderComponent, Position)); // *Some magic preprocessor involved*
  for (auto &e : entities) { // e is a shared_ptr of Entity
      // do something
  }

  mainLoop();
  return 0;
}

