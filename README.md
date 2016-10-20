Entitas++
=====================

Entitas++ is a fast Entity Component System Framework (ECS) C++11 port of [Entitas for C# and Unity](https://github.com/sschmid/Entitas-CSharp).

There are some differences between both, so please check the code and notes below !

The framework will follow a [semantic versioning](http://semver.org/).

Any feedback is welcome !

Documentation
=====================

#### Overview

As in Entitas C#, basic code architecture is the same:

```
+------------------+
|       Pool       |
|------------------|
|    e       e     |      +-----------+
|        e     e---|----> |  Entity   |
|  e        e      |      |-----------|
|     e  e       e |      | Component |
| e            e   |      |           |      +-----------+
|    e     e       |      | Component-|----> | Component |
|  e    e     e    |      |           |      |-----------|
|    e      e    e |      | Component |      |   Data    |
+------------------+      +-----------+      +-----------+
  |
  |
  |     +-------------+  Groups:
  |     |      e      |  Subsets of entities in the pool
  |     |   e     e   |  for blazing fast querying
  +---> |        +------------+
        |     e  |    |       |
        |  e     | e  |  e    |
        +--------|----+    e  |
                 |     e      |
                 |  e     e   |
                 +------------+
```

#### Creating a new component example

###### Entitas C&#35;
```csharp
public class PositionComponent : IComponent {
    public float x;
    public float y;
    public float z;
}

//...

var pool = new Pool(ComponentIds.TotalComponents);
var e = pool.CreateEntity();

e.AddPosition(1f, 2f, 3f);
e.ReplacePosition(3f, 2f, 1f);
e.RemovePosition();

var posX = e.position.x;
var hasPosition = e.hasPosition;
```

###### Entitas++
```cpp
// I suggest you to use structs instead, they are the same and all members/methods are public by default.
class Position : public IComponent {
    public:
        // You must provide at least ONE public "Reset" method with any parameters you want
        void Reset(float px, float py, float pz) {
            x = px;
            y = py;
            z = pz;
        }

        float x;
        float y;
        float z;
};

//...

auto pool = new Pool();
auto e = pool->CreateEntity();

e->Add<Position>(1f, 2f, 3f);
e->Replace<Position>(3f, 2f, 1f);
e->Remove<Position>();

float posX = e->Get<Position>()->x;
bool hasPosition = e->Has<Position>();
```

#### Basic component management

###### Entitas C&#35;
```csharp
entity.AddPosition(3, 7);
entity.AddHealth(100);
entity.isMovable = true;

entity.ReplacePosition(10, 100);
entity.ReplaceHealth(entity.health.value - 1);
entity.isMovable = false;

entity.RemovePosition();

var hasPos = entity.hasPosition;
var movable = entity.isMovable;
```

###### Entitas++
```cpp
entity->Add<Position>(3, 7);
entity->Add<Health>(100);
entity->Replace<Movable>(); // There is no helper in here, you must use "Replace" and "Remove" methods

entity->Replace<Position>(10, 100);
entity->Replace<Health>(entity->Get<Health>()->value - 1);
entity->Remove<Movable>();

entity->Remove<Position>();

bool hasPos = entity->Has<Position>();
bool movable = entity->Has<Movable>(); // Again, you must use "Has" to check if an entity has a component
```

#### Pools

###### Entitas C&#35;
```csharp
// Pools.pool is kindly generated for you by the code generator
var pool = Pools.pool;
var entity = pool.CreateEntity();
entity.isMovable = true;

// Returns all entities having MovableComponent and PositionComponent.
// Matchers are also generated for you.
var entities = pool.GetEntities(Matcher.AllOf(Matcher.Movable, Matcher.Position));
foreach (var e in entities) {
    // do something
}
```

###### Entitas++
```cpp
// No code generator == no "Pools". You must handle yourself your Pools !
auto pool = new Pool();
auto entity = pool->CreateEntity();
entity->Replace<Movable>(); // I used "Replace" but in this case is better to use "Add" directly

// Returns all entities having MovableComponent and PositionComponent.
// Matchers are also generated for you.
auto entities = pool->GetEntities(Matcher_AllOf(Movable, Position)); // *Some magic preprocessor involved*
for (auto &e : entities) { // e is a shared_ptr of Entity
    // do something
}
```

#### Group events

###### Entitas C&#35;
```csharp
pool.GetGroup(Matcher.Position).OnEntityAdded += (group, entity, index, component) => {
    // Do something
};
```

###### Entitas++
```cpp
pool->GetGroup(Matcher_AllOf(Position))->OnEntityAdded += [](std::shared_ptr<Group> group, EntityPtr entity, ComponentId index, IComponent* component) {
    // Do something
};
```

#### Group Observer

###### Entitas C&#35;
```csharp
var group = pool.GetGroup(Matcher.Position);
var observer = group.CreateObserver(GroupEventType.OnEntityAdded);
foreach (var e in observer.collectedEntities) {
    // do something
}
observer.ClearCollectedEntities();
```

###### Entitas++
```cpp
auto group = pool->GetGroup(Matcher_AllOf(Position)); // There are _AnyOf and _NoneOf too !
auto observer = group->CreateObserver(GroupEventType::OnEntityAdded);
for (auto &e : observer->GetCollectedEntities()) {
    // do something
}
observer->ClearCollectedEntities();
```

#### Execute Systems

###### Entitas C&#35;
```csharp
public class MoveSystem : IExecuteSystem, ISetPoolSystem {
    Group _group;

    public void SetPool(Pool pool) {
        _group = pool.GetGroup(Matcher.AllOf(Matcher.Move, Matcher.Position));
    }

    public void Execute() {
        foreach (var e in _group.GetEntities()) {
            var move = e.move;
            var pos = e.position;
            e.ReplacePosition(pos.x, pos.y + move.speed, pos.z);
        }
    }
}
```

###### Entitas++
```cpp
class MoveSystem : public IExecuteSystem, public ISetPoolSystem {
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
```

#### Reactive Systems

###### Entitas C&#35;
```csharp
public class RenderPositionSystem : IReactiveSystem {
    public TriggerOnEvent trigger { get { return Matcher.AllOf(Matcher.Position, Matcher.View).OnEntityAdded(); } }

    public void Execute(List<Entity> entities) {
        // Gets executed only if the observed group changed.
        // Changed entities are passed as an argument
        foreach (var e in entities) {
            var pos = e.position;
            e.view.gameObject.transform.position = new Vector3(pos.x, pos.y, pos.z);
        }
    }
}
```

###### Entitas++
```cpp
class RenderPositionSystem : public IReactiveSystem {
    TriggerOnEvent trigger;

    public:
        RenderPositionSystem() {
        	trigger = Matcher_AllOf(Position, View)->OnEntityAdded();
        }

        void Execute(std::vector<EntityPtr> entities) {
            // Gets executed only if the observed group changed.
            // Changed entities are passed as an argument
            for (auto &e : entities) {
                auto pos = e->Get<Position>();
                // NOTE: Unity-only example, but this maybe could be the code if Unity were compatible with C++
                e->Get<View>()->gameObject.transform.position = new Vector3(pos->x, pos->y, pos->z);
            }
        }
}
```

Notes
=====================

- All code above was written using: ```using namespace EntitasPP;```, if you don't, you must prepend all the classes with the namespace ```EntitasPP::```.
- There is *no* code generator because C++ lacks of [code reflection](https://en.wikipedia.org/wiki/Reflection_(computer_programming)). So all code must be done by you, but there are a lot of templating involved in here to ease the work for you anyways (see code above).
- ['Systems' class](https://github.com/sschmid/Entitas-CSharp/blob/94ab9b172987a65a7facfd4c383b621a4cbb0bca/Entitas/Entitas/Systems.cs#L8) was renamed to 'SystemContainer'. You can see a simple example of use in the provided 'main.cpp'.
- All 'ToString()' methods were removed. If you need to track identifiers between entities I suggest you to use your own custom Component.
- AERC (Automatic Entity Reference Counting) was implemented using smart pointers. (Yeah! Take that C# :stuck_out_tongue_winking_eye:)

If you need more documentation of the architecture of the framework, please go to [Entitas C# Wiki](https://github.com/sschmid/Entitas-CSharp/wiki) since this framework has a lot on common with the original C# one.

Planned
=====================

- Unit Tests
- *Moar* Unit Tests
- Perfomance Tests
- More robust code (more tested :smile:)
- Examples
- Better Readme.md (suggestions?)

The MIT License (MIT)
=====================

Copyright © 2016 Juan Delgado (JuDelCo)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the “Software”), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
