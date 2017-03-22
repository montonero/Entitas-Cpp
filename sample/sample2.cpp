#include "entitas/ISystem.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include "entitas/SystemContainer.hpp"
#include "entitas/Collector.hpp"

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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDLpp.h>

#include "Rectangle.h"

constexpr int kScreenWidth = 600;
constexpr int kScreenHeight = 1000;

using namespace entitas;

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

class Position : public IComponent {
public:
    void reset(Vec2&& v) { position_ = v; }
    Vec2 position_;
};

class Appearance : public IComponent {
public:
    void reset(Vec2&& v) { size_ = v; }
    Vec2 size_;
};

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

struct RenderComponent : public IComponent {
    void reset(Material m) { material = m; }
    Material material;
};

/* -------------------------------------------------------------------------- */

class MoveSystem : public IExecuteSystem, public ISetPoolSystem {
    Group::SharedPtr _group;

public:
    void setPool(Pool* pool)
    {
        auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(Move), COMPONENT_GET_TYPE_ID(Position) });
        _group = pool->getGroup(matcher);
    }

    void execute()
    {
        for (auto& e : _group->getEntities()) {
            auto move = e->get<Move>();
            auto pos = e->get<Position>();
            // e->replace<Position>(pos->x, pos->y + move->speed, pos->z);
        }
    }
};

class RenderPositionSystem : public IReactiveSystem {
    //TriggerOnEvent trigger;

public:
    RenderPositionSystem()
    {
        // auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(Move), COMPONENT_GET_TYPE_ID(Position) });
        // trigger = Matcher_AllOf(Position, View)->OnEntityAdded();
        trigger = (Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(Position) })).onEntityAdded();
    }

    void execute(std::vector<EntityPtr>& entities) override
    {
        // Gets executed only if the observed group changed.
        // Changed entities are passed as an argument
        for (auto& e : entities) {
            auto pos = e->get<Position>();
            // NOTE: Unity-only example, but this maybe could be the code if Unity were compatible with C++
            // e->Get<View>()->gameObject.transform.position = new Vector3(pos->x, pos->y, pos->z);
        }
    }
};

/* -------------------------------------------------------------------------- */

void
renderMat(sdl::Renderer* renderer, sdl::Color c, const Vec2& v, const Vec2& s)
{
    auto rect = sdl::makeRect(v, s);
    renderer->draw(rect, c);
}

sdl::Color randomColor()
{
    using namespace std;
    static random_device rd;
    static mt19937 engineMt(rd());
    uniform_int_distribution<int> uniform_int(50, 254);
    //auto random_int = bind(uniform_int, engineMt);
    // static const unsigned sizeVec{ 10 };
    sdl::Color c(uniform_int(engineMt), uniform_int(engineMt), uniform_int(engineMt));
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
    Vec2 v{ unifW(engineMt), unifH(engineMt) };
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

class MySystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem {
public:
    void setPool(Pool* pool)
    {
        pool_ = pool;
        //group_ = pool_->getGroup(Matcher_allOf(Position, RenderComponent));
        auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(Position) });
        group_ = pool_->getGroup(matcher);
        collector_ = group_->createCollector(GroupEventType::OnEntityAdded);
        collector_->activate();
        collector_->activate();
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
        for (auto& e : es) {
            auto mat = e->get<RenderComponent>();
            auto pos = e->get<Position>();
            auto appearance = e->get<Appearance>();
            renderMat(renderer_, mat->material.color, pos->position_, appearance->size_);
        }
        // std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;
        for (auto& e : (collector_->getCollectedEntities()) )
        {
            std::cout << "ent";
        }
    }

    void setRenderer(sdl::Renderer& r) { renderer_ = &r; }

private:
    sdl::Renderer* renderer_{ nullptr };
    Pool* pool_{ nullptr };
    Group::SharedPtr group_;
    // test
    std::shared_ptr<Collector> collector_;
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

struct MainLoopContext {
    sdl::Window window;
    sdl::Renderer* renderer;

    std::shared_ptr<SystemContainer> systems;
    std::shared_ptr<Pool> pool;
    std::shared_ptr<ISystem> mySystem;

    int done{ 0 };
    std::shared_ptr<sdl::Sprite> sprite;
};

void mainLoop(void* vctx)
{
    auto ctx = (MainLoopContext*)vctx;

    SDL_Event event;
    //std::cout << "Main loop entered.\n";

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            std::cout << "Quitting\n";
            ctx->done = 1;
        }
        if (event.type == SDL_KEYDOWN) {
            std::cout << "Hello\n";
            addRandomEntity(ctx->pool.get());
        }
    }

    ctx->renderer->clear(sdl::Colors::Black);
    ctx->systems->execute();

    ctx->renderer->drawCircle({ 100, 100 }, 50.f, sdl::Colors::Blue);
    ctx->renderer->drawLine({ 0, 0 }, { 100, 100 }, sdl::Colors::White);

    // Construct a view
    // sdl::View view{sdl::Vector2f{0.f, 0.f}, sdl::Vector2f{800.f, 600.f}};
    sdl::View view{ sdl::Vector2f{ 0.f, 0.f }, sdl::Vector2f{ 200.f, 150.f } };
    auto renderer = ctx->renderer;
    auto snowSprite = *ctx->sprite.get();

    renderer->SetView(view);

    renderer->Draw(snowSprite);

    view.Move({ -100.f, -200.f });
    renderer->SetView(view);

    renderer->Draw(snowSprite);
    // snowSprite.scale(0.5f);
    // view.Move({-400.f, -600.f});
    // renderer->SetView(view);
    // renderer->Draw(snowSprite);

    // Render the rect to the screen
    // SDL_RenderPresent(ctx->renderer);
    ctx->renderer->Present();
    SDL_Delay(100);
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
    systems->add(pool->createSystem<RenderPositionSystem>());
    systems->initialize();

    //for(unsigned int i = 0; i < 2; ++i) {
    //  systems->execute();
    //}

    std::cout << "All systems initilized.\n";

    auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(Position) });
    auto entities = pool->getEntities(matcher); // *Some magic preprocessor involved*
#if 0
    for (auto& e : entities) { // e is a shared_ptr of Entity
        // do something
    }
#endif
    std::cout << "sdl::Init()\n";
    sdl::Init();

    /* seed random number generator */
    srand(time(NULL));

    std::cout << "sdl::Init() successfully!\n";
    sdl::Window w{ "Test window", kScreenWidth, kScreenHeight };
    auto renderer = w.CreateRenderer();
    ((MySystem*)mySystem.get())->setRenderer(*renderer);

//ctx->renderer = ctx->window.CreateRenderer();
//((MySystem*)mySystem.get())->setRenderer(*ctx->renderer);

#ifdef __EMSCRIPTEN__
    const std::string kAssetsFolder = "/";
    // const std::string kAssetsTexturesFolder = kAssetsFolder + "textures/";
    const std::string kAssetsTexturesFolder = kAssetsFolder + "textures/";
#else
    const std::string kAssetsFolder = "../assets/";
    const std::string kAssetsTexturesFolder = kAssetsFolder + "textures/";
#endif

    auto texture = renderer->CreateTexture(kAssetsTexturesFolder + "tiles_snow/light.png");
    sdl::Sprite snowSprite(*texture);
    snowSprite.scale(2.0f);

    auto ctx = new MainLoopContext{
        std::move(w), renderer,
        systems, pool, mySystem,
        0, std::make_shared<sdl::Sprite>(std::move(snowSprite))
    };
    std::cout << "Context created.\n";

/* Enter render loop, waiting for user to quit */
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, (void*)ctx, 0, 0);
#else
    while (ctx->done == 0) {
        mainLoop(ctx);
    }
    std::cout << "Done.\n";
    delete ctx;
#endif

    return 0;
}
