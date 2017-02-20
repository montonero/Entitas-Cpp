#include "entitas/SystemContainer.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include "entitas/ISystem.hpp"
#include <iostream>
#include <random>

#ifdef WIN32
#include <string>
#endif

#ifdef __APPLE__
    #ifdef _SDL2
        // waf
        #include "SDL.h"
    #else
        #include "SDL2/SDL.h"
    #endif
#else
    #include "SDL.h"
#endif

#include "Rectangle.h"



constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 480;


using namespace entitas;

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

class Position : public IComponent
{
public:
    void reset(Vec2 v) { position_ = v; }
    Vec2 position_;
};

class Appearance : public IComponent
{
public:
    void reset(Vec2 v) { size_ = v; }
    Vec2 size_;
};

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

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
		auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(Move), COMPONENT_GET_TYPE_ID(Position) });
        _group = pool->getGroup(matcher);
    }

    void execute()
    {
        for (auto &e : _group->getEntities())
        {
            auto move = e->get<Move>();
            auto pos = e->get<Position>();
            // e->replace<Position>(pos->x, pos->y + move->speed, pos->z);
        }
    }
};

/* -------------------------------------------------------------------------- */

SDL_Renderer* gSdlRenderer;

void renderMat(SDL_Renderer* renderer, Color c, Vec2 v, Vec2 s)
{
    // auto sr = toSdlRect(r);
    SDL_Rect sr;
    sr.x = v.x();
    sr.y = v.y();
    sr.w = s.x();
    sr.h = s.y();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(renderer, &sr);
}  

Color randomColor()
{
    using namespace std;
    static random_device rd;
    static mt19937 engineMt(rd());
    uniform_int_distribution<int> uniform_int(50, 254);
    //auto random_int = bind(uniform_int, engineMt);
    // static const unsigned sizeVec{ 10 };
    Color c(uniform_int(engineMt), uniform_int(engineMt), uniform_int(engineMt));
    return c;
}

/* -------------------------------------------------------------------------- */

Vec2 randomVec2(int x, int y, int mx, int my)
{
    using namespace std;
    static random_device rd;
    static mt19937 engineMt(rd());
    uniform_real_distribution<float> unifW(x, mx);    
    uniform_real_distribution<float> unifH(y, my);
    Vec2 v(unifW(engineMt), unifH(engineMt));
    return v;
}

/* -------------------------------------------------------------------------- */

Vec2 randomVec2Pos()
{
    return randomVec2(0, 0, kScreenWidth, kScreenHeight);
}

/* -------------------------------------------------------------------------- */

Vec2 randomVec2Size()
{
    return randomVec2(20, 20, 80, 100);
}

/* -------------------------------------------------------------------------- */
// Random Entity with Color, Position and Size
void addRandomEntity(Pool* pool)
{
    auto e = pool->createEntity();
    e->add<RenderComponent>(randomColor());
    // e->add<RenderComponent>(Material::yellow());
    // e->add<Position>(100, 100, 10);
    e->add<Position>(randomVec2Pos());
    e->add<Appearance>(randomVec2Size());
}


class MySystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem
{
public:
    void setPool(Pool* pool)
    {
        pool_ = pool;
        //group_ = pool_->getGroup(Matcher_allOf(Position, RenderComponent));
		auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(Position) });
		group_ = pool_->getGroup(matcher); 
        std::cout << "MySystem::setPool called" << std::endl;
    }
    
    void initialize()
    {
        addRandomEntity(pool_);
        std::cout << "MySystem initialized" << std::endl;
    }

    void execute()
    {
        auto es = group_->getEntities();
        for (auto &e : es)
        {
            auto mat = e->get<RenderComponent>();
            auto pos = e->get<Position>();
            auto appearance = e->get<Appearance>();
            renderMat(gSdlRenderer, mat->material.color, pos->position_, appearance->size_);
        }
        // std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;
    }

private:
    Pool* pool_{nullptr};
    std::shared_ptr<Group> group_;
};

/* -------------------------------------------------------------------------- */

SDL_Rect toSdlRect(Rectangle r)
{
    SDL_Rect sr;
    sr.x = r.position.x();
    sr.y = r.position.y();
    sr.w = r.size.x();
    sr.h = r.size.y();
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



int mainLoop()
{

    return 0;
}

/* -------------------------------------------------------------------------- */

#undef main
int main(const int argc, const char* argv[])
{
  auto systems = std::make_shared<SystemContainer>();
  auto pool = std::make_shared<Pool>();

  //systems->add(pool->createSystem<DemoSystem>());
  auto mySystem = pool->createSystem<MySystem>();
  systems->add(mySystem);
  systems->initialize();

  for(unsigned int i = 0; i < 2; ++i) {
    systems->execute();
  }

  auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(Position) });
  auto entities = pool->getEntities(matcher); // *Some magic preprocessor involved*
  for (auto &e : entities) { // e is a shared_ptr of Entity
      // do something
  }

/////
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
    auto window = SDL_CreateWindow(NULL, 0, 0, kScreenWidth, kScreenHeight, SDL_WINDOW_OPENGL);
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
    gSdlRenderer = renderer;
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
            if (event.type == SDL_KEYDOWN) 
            {
                std::cout << "Hello\n";
                // ((MySystem*)mySystem)->addRandomEntity();
                addRandomEntity(pool.get());
            }

        }
        // string text;
        // cin >> text;
        //render(renderer, rs);

        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderClear( renderer );
        
        systems->execute();
        
        // Render the rect to the screen
        SDL_RenderPresent(renderer);
        
        SDL_Delay(100);
    }


  mainLoop();
  return 0;
}

