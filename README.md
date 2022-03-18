# Community Mass Sample
My very WIP understanding of Unreal Engine 5's experimental ECS plugin with a small sample project. I am not affiliated with Epic Games and this system is actively being changed often so this information might not be accurate.
If something is wrong feel free to PR!

I will write more soon.

<!--- Introduce here table of contents -->
<a name="tocs"></a>
## Table of Contents
> 1. [The sample](#sample)  
> 2. [Entity Component System](#ecs)   
> 2.1. [Test indent 1](#tocs)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.1.1. [Test indent 2](#tocs)
> 3. [Mass](#mass)

<a name="sample"></a>
## The sample 
Currently, the sample features the following:

- A bare minimum movement processor to show how to set up processors
- An entity spawner that uses a special mass-specific data asset to spawn entities in a circle defined in an EQS
- A Mass-simulated crowd of cones that parades around the level following a ZoneGraph shape with lanes


<a name="ecs"></a>
## Entity Component System 
Mass is an archetype-based Entity Componenet System (ECS). If you already know what that is you can skip ahead to the next section.

In Mass, some ECS terminology differs from the norm in order to not get confused with existing unreal code:
| ECS | Mass |
| ----------- | ----------- |
| Entity | Entity |
| Component | Fragment | 
| System | Processor | 

<!-- FIXME: This is repeated information also found below. Can use this to index further sections. Missing Traits. -->
<!--- Entities: simple unique identifiers that can have components.-->
<!--- Fragments: data-only structs that can be removed or added at runtime -->
<!--- Processors: functions that operate on specific components they query for-->

Typical Unreal Engine game code is expressed as actor objects that inherit from parent classes to change their data and functionality based on what they ***are***. 
In an ECS, an entity is only composed of fragments that get manipulated by processors based on which ECS components they ***have***. 

An entity is really just a small unique identifier that points to some fragments. A Processor defines a query that filters only for entities that have specific fragments. For example, a basic "movement" Processor could query for entities that have a transform and velocity component to add the velocity to their current transform position. 

Fragments are stored in memory as tightly packed arrays of other identical fragment arrangements called archetypes. Because of this, the aforementioned movement processor can be incredibly high performance because it does a simple operation on a small amount of data all at once. New functionality can easily be added by creating new fragments and processors.

Internally, Mass is similar to the existing [Unity DOTS](https://docs.unity3d.com/Packages/com.unity.entities@0.17/manual/index.html) and [FLECS](https://github.com/SanderMertens/flecs) archetype-based ECS libraries. There are many more!


<!-- FIXME: Let's figure out first an index to later fill with content if you agree. -->
<!-- FIXME: I'd say we can keep the majority of content we have in here, but we should define first an index. -->

<a name="mass"></a>
## Mass
Mass is Unreal's new in-house ECS framework! Technically, [Sequencer](https://docs.unrealengine.com/4.26/en-US/AnimatingObjects/Sequencer/Overview/) already used one internally but it wasn't intended for gameplay code. Mass was created by the AI team at Epic Games to facilitate massive crowd simulations but has grown to include many other features as well. It was featured in the new [Matrix demo](https://www.unrealengine.com/en-US/blog/introducing-the-matrix-awakens-an-unreal-engine-5-experience) Epic released recently.

I'll start with a quick overview of the important stuff for now. 

#### Entities
Unique identifiers for individual entities.
#### Fragments
Raw data-only UScriptStructs that entities can own and processors can query on. Stored in chunked archetype arrays for quick processing.
#### Tags
Fragments that have no data to only be used as tags for filtering. Just bits on an archetype internally.
#### Processors
The main way fragments are operated on in Mass. Combine one more user-defined queries with functions that operate on the data inside them. They can also include rules that define in which order they are called in. Automatically registered with Mass by default. 
#### Queries
A collection of fragments and tags combined with rules to filter for. Can exclude certain fragments or even include them optionally. This section will be expanded on soon!

<!-- FIXME: Might be nice minimal code samples for relevant parts + cross ref the simple use case. -->


#### Traits

Traits are C++ defined objects that declare a set of fragments and data to use for authoring new entities in a data-driven way. They usually contain fragments and data that go well together to make defining different kinds of entities in the editor simple. 

To start using traits, simply create a DataAsset that inherits from 
MassEntityConfigAsset and add new traits to it. Each trait can be expanded to set properties if it has any. There are many built-in traits including an "assorted fragments" trait to add specific fragments without making a new trait. You can also define a parent MassEntityConfigAsset to inherit the fragments from another DataAsset.

Traits are often used to add SharedFragments in the form of settings.

#### Shared Fragments

Shared Fragments (FMassSharedFragment) are fragments that multiple entities can point to. This is often used for configuration that won't change for a group of entities at runtime. The archetype only needs to store one copy for many of sharing entities.

### Plugins and Modules
The Mass framework is divided into many different plugins and modules. Here's my quick overview:
##### MassEntity
The core plugin+modules manages all entity creation and storage. You should store a pointer to this subsystem in your code.
##### MassGameplay 
A plugin with many modules with many useful fragments and processors is used throughout the framework. 
- **MassCommon**
Basic fragments like FTransformFragment.
- **MassMovement**
Features an important "UMassApplyMovementProcessor" processor that moves entities based on their velocity and force. Also includes a very "hello world" sample.
- **MassRepresentation**
Processors and fragments for rendering entities in the world. They generally use an ISMC to do so.
- **MassSpawner** 
A highly configurable actor type that can spawn specific entities where you want. 
- **MassActors**
A bridge between the general UE5 actor framework and Mass. A type of fragment that turns entities into "Agents" that can exchange data in either direction (or both).
- **MassLOD**
LOD Processors that can manage different kinds of levels of detail, from rendering to ticking at different rates based on fragment settings.
- **MassReplication**
Replication support for Mass! Other modules override UMassReplicatorBase to replicate stuff. Entities are given a separate Network ID that gets
- **MassSignals** 
A system that lets entities send named signals to each other.
- **MassSmartObjects** 
Lets entities "claim" SmartObjects to interact with them.

There are more modules that implement AI features that I will document with more detail when I get around to it.

- **Zonegraph**
In-level splines and shapes that use config defined lanes to direct crowd entities around! Think sidewalks, roads etc.
- **StateTree**
A new lightweight AI statemachine that can work in conjunction with Mass Crowds. One of them is used to give movement targets to the cones in the parade in the sample.







