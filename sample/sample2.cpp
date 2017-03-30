#include "entitas/Collector.hpp"
#include "entitas/ISystem.hpp"
#include "entitas/Matcher.hpp"
#include "entitas/Pool.hpp"
#include "entitas/SystemContainer.hpp"

//#include <iostream>
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

#include <fmt/format.h>

#include <SDLpp.h>

// #include <tinyheaders/tinyc2.h>
#include <tinyc2.h>

#include "Rectangle.h"

constexpr int kScreenWidth = 600;
constexpr int kScreenHeight = 800;

using namespace entitas;

/* -------------------------------------------------------------------------- */

class DemoComponent : public IComponent {
public:
    void reset(const std::string& name1, const std::string& name2)
    {
        //std::cout << "Created new entity: " << name1 << "," << name2 << std::endl;
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
        //std::cout << "DemoSystem initialized" << std::endl;
    }

    void execute()
    {
        mPool->createEntity()->add<DemoComponent>("foo", "bar");

        auto entitiesCount = mPool->getGroup(Matcher_allOf(DemoComponent))->count();
        //std::cout << "There are " << entitiesCount << " entities with the component 'DemoComponent'" << std::endl;
        // std::cout << "DemoSystem executed" << std::endl;
    }

private:
    Pool* mPool;
};

/* -------------------------------------------------------------------------- */

class PhysicsComponent : public IComponent {
public:
    void reset(Vec2 pos, Vec2 dims)
    {
        position_ = pos;
        dimension_ = dims;
    }
    Vec2 position_;
    Vec2 dimension_;
};

class AppearanceComponent : public IComponent {
public:
    void reset(Vec2 pos)
    {
        position_ = pos;
    }

    void reset(Vec2 pos, Vec2 size)
    {
        position_ = pos;
        size_ = size;
    }
    Vec2 position_;
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

class LifeComponent : public IComponent {
public:
    void reset(int val) { value_ = val; }
    int value_{ 0 };
};
/* -------------------------------------------------------------------------- */

class ClickComponent : public IComponent {
public:
    void reset(Vec2&& v) { position_ = v; }
    Vec2 position_;
};

/* -------------------------------------------------------------------------- */

struct RenderComponent : public IComponent {
    void reset(Material m) { material = m; }
    Material material;
    Vec2 position;
};

/* -------------------------------------------------------------------------- */

class MoveSystem : public IExecuteSystem, public ISetPoolSystem {
    Group::SharedPtr _group;

public:
    void setPool(Pool* pool)
    {
        auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(Move), COMPONENT_GET_TYPE_ID(AppearanceComponent) });
    }

    void execute()
    {
        for (auto& e : _group->getEntities()) {
            auto move = e->get<Move>();
            auto appear = e->get<AppearanceComponent>();
            Vec2 newPos = move->direction * move->speed + appear->position_;
            e->replace<AppearanceComponent>(std::move(newPos));
        }
    }
};

/* -------------------------------------------------------------------------- */

// Updates appearance from physics
class PhysicsAppearanceSystem : public IReactiveSystem {
public:
    PhysicsAppearanceSystem()
    {
        trigger = (Matcher::allOf({ COMPONENT_GET_TYPE_ID(PhysicsComponent), COMPONENT_GET_TYPE_ID(AppearanceComponent) })).onEntityAdded();
    }

    void execute(std::vector<EntityPtr>& entities) override
    {
        // Gets executed only if the observed group changed.
        // Changed entities are passed as an argument
        for (auto& e : entities) {
            //auto apComp= e->get<AppearanceComponent>();
            auto physComp = e->get<PhysicsComponent>();
            //ren->position = pos->position_;
            e->replace<AppearanceComponent>(physComp->position_, physComp->dimension_);
        }
    }
};

/* -------------------------------------------------------------------------- */

// Updates render position
class RenderAppearanceSystem : public IReactiveSystem {
public:
    RenderAppearanceSystem()
    {
        trigger = (Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(AppearanceComponent) })).onEntityAdded();
    }

    void execute(std::vector<EntityPtr>& entities) override
    {
        // Gets executed only if the observed group changed.
        // Changed entities are passed as an argument
        for (auto& e : entities) {
            auto appear = e->get<AppearanceComponent>();
            auto ren = e->get<RenderComponent>();
            // NOTE we don't call 'replace'
            ren->position = appear->position_;
        }
    }
};

/* -------------------------------------------------------------------------- */

// React on added clicks
class ClickSystem : public IInitializeSystem, public IReactiveSystem, public ISetPoolSystem {
protected:
    Group::SharedPtr group_;

public:
    Pool* pool_{ nullptr };
    ClickSystem()
    {
        trigger = (Matcher::allOf({ COMPONENT_GET_TYPE_ID(ClickComponent) })).onEntityAdded();
        //group_ = pool_->getGroup(Matcher::allOf({ COMPONENT_GET_TYPE_ID(AppearanceComponent) }));
    }
    void initialize() override
    {
        group_ = pool_->getGroup(Matcher::allOf({ COMPONENT_GET_TYPE_ID(AppearanceComponent) }));
    }

    void setPool(Pool* pool) override
    {
        pool_ = pool;
    }
    void execute(std::vector<EntityPtr>& entities) override
    {
        for (auto& e : entities) {
            // we should only get one at a time

            auto pos = e->get<ClickComponent>()->position_;

            // now iterate through all entities with a position component
            for (auto& ep : group_->getEntities()) {
                auto posE = ep->get<AppearanceComponent>()->position_;
                auto sizeE = ep->get<AppearanceComponent>()->size_;
                auto botRight = posE + sizeE;
                if ((pos.x() >= posE.x()) && (pos.x() <= botRight.x()) && (pos.y() >= posE.y()) && (pos.y() <= botRight.y()))
                    pool_->destroyEntity(ep);
            }

            pool_->destroyEntity(e);
            // auto ren = e->get<RenderComponent>();
            // ren->position = pos->position_;
        }
    }
};

/* -------------------------------------------------------------------------- */

void renderMat(sdl::Renderer* renderer, sdl::Color c, const Vec2& v, const Vec2& s)
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
// Random Entity with Color, AppearanceComponent and Size
void addRandomEntity(Pool* pool)
{
    auto e = pool->createEntity();
    e->add<RenderComponent>(randomColor());
    e->add<PhysicsComponent>(randomVec2Pos(), randomVec2Size());
    e->add<AppearanceComponent>(randomVec2Pos(), randomVec2Size());
    //e->add<Appearance>(randomVec2Size());
    e->add<LifeComponent>(1);
    auto r = pool->hasEntity(e);
    //std::cout << r;
}

template <typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g)
{
    std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
    std::advance(start, dis(g));
    return start;
}

template <typename Iter>
Iter select_randomly(Iter start, Iter end)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    return select_randomly(start, end, gen);
}

void changeRandomEntity(Pool* p)
{
    auto es = p->getEntities();
    auto randomEntity = *select_randomly(es.begin(), es.end());
    randomEntity->replace<AppearanceComponent>(randomVec2Pos());
}

// This is actually render system as it renders our quads
class MySystem : public IInitializeSystem, public IExecuteSystem, public ISetPoolSystem {
public:
    void setPool(Pool* pool)
    {
        pool_ = pool;
        auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent) });
        group_ = pool_->getGroup(matcher);
        collector_ = group_->createCollector(GroupEventType::OnEntityAdded);
        //collector_->activate();
        fmt::print("MySystem::setPool called\n");
    }

    void initialize()
    {
        addRandomEntity(pool_);
        fmt::print("MySystem initialized\n");
    }

    void execute()
    {
        auto es = group_->getEntities();
        for (auto& e : es) {
            auto ren = e->get<RenderComponent>();
            auto appearance = e->get<AppearanceComponent>();
            renderMat(renderer_, ren->material.color, ren->position, appearance->size_);
        }

        for (auto& e : (collector_->getCollectedEntities())) {
            //std::cout << "ent";
        }
        collector_->clearCollectedEntities();
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

/* -------------------------------------------------------------------------- */

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
            //std::cout << "Quitting\n";
            ctx->done = 1;
        }
        if (event.type == SDL_KEYDOWN) {
            //std::cout << "Hello\n";
            if (event.key.keysym.scancode == SDL_SCANCODE_A)
                addRandomEntity(ctx->pool.get());
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            //changeRandomEntity(ctx->pool.get());
        }
        if (event.type == SDL_MOUSEBUTTONUP) {
            auto x = event.button.x;
            auto y = event.button.y;
            auto pool = ctx->pool.get();
            auto e = pool->createEntity();
            e->add<ClickComponent>(Vec2(x, y));
            fmt::print("Button clicked at {} {}\n", x, y);
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
    auto clickSystem = pool->createSystem<ClickSystem>();
    systems->add(pool->createSystem<RenderAppearanceSystem>());
    systems->add(mySystem);
    systems->add(clickSystem);
    systems->initialize();

    //for(unsigned int i = 0; i < 2; ++i) {
    //  systems->execute();
    //}

    //std::cout << "All systems initilized.\n";

    auto matcher = Matcher::allOf({ COMPONENT_GET_TYPE_ID(RenderComponent), COMPONENT_GET_TYPE_ID(AppearanceComponent) });
    auto entities = pool->getEntities(matcher);
#if 0
    for (auto& e : entities) { // e is a shared_ptr of Entity
        // do something
    }
#endif
    //std::cout << "sdl::Init()\n";
    sdl::Init();

    /* seed random number generator */
    srand(time(NULL));

    //std::cout << "sdl::Init() successfully!\n";
    sdl::Window w{ "Test window", kScreenWidth, kScreenHeight };
    auto renderer = w.CreateRenderer();
    ((MySystem*)mySystem.get())->setRenderer(*renderer);

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
//std::cout << "Context created.\n";

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, (void*)ctx, 0, 0);
#else
    while (ctx->done == 0) {
        mainLoop(ctx);
    }
    //std::cout << "Done.\n";
    delete ctx;
#endif

    return 0;
}
