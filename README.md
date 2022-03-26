# Community Mass Sample
Our very WIP understanding of Unreal Engine 5's experimental Entity Component System (ECS) plugin with a small sample project. We are not affiliated with Epic Games and this system is actively being changed often so this information might not be accurate.
If something is wrong feel free to PR!

Currently built for the Unreal Engine 5.0 preview 2 binary from the Epic launcher.

This documentation will be updated often!

<!--- Introduce here table of contents -->
<a name="tocs"></a>
## Table of Contents
> 1. [Mass](#mass)  
> 2. [Entity Component System](#ecs)  
> 3. [Sample Project](#sample)  
> 3.1 [Test indent 1](#tocs)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3.1.1. [Test indent 2](#tocs)  
> 4. [Mass Concepts](#massconcepts)  
> 4.1 [Entities](#mass-entities)   
> 4.2 [Fragments](#mass-fragments)  
> 4.3 [Tags](#mass-tags)  
> 4.4 [Processors](#mass-processors)  
> 4.5 [Queries](#mass-queries)  
> 4.6 [Traits](#mass-traits)  
> 4.7 [Shared Fragments](#mass-sf)  
> 4.8 [Observers](#mass-o)
> 5. [Mass Plugins and Modules](#mass-pm)  
> 5.1 [MassEntity](#mass-pm-me)  
> 5.2 [MassGameplay](#mass-pm-gp)  
> 5.3 [MassAI](#mass-pm-ai)  

<a name="mass"></a>
## 1. Mass
Mass is Unreal's new in-house ECS framework! Technically, [Sequencer](https://docs.unrealengine.com/4.26/en-US/AnimatingObjects/Sequencer/Overview/) already used one internally but it wasn't intended for gameplay code. Mass was created by the AI team at Epic Games to facilitate massive crowd simulations but has grown to include many other features as well. It was featured in the new [Matrix demo](https://www.unrealengine.com/en-US/blog/introducing-the-matrix-awakens-an-unreal-engine-5-experience) Epic released recently.

<a name="ecs"></a>
## 2. Entity Component System 
Mass is an archetype-based Entity Componenet System. If you already know what that is you can skip ahead to the next section.

In Mass, some ECS terminology differs from the norm in order to not get confused with existing unreal code:
| ECS | Mass |
| ----------- | ----------- |
| Entity | Entity |
| Component | Fragment | 
| System | Processor | 

Typical Unreal Engine game code is expressed as actor objects that inherit from parent classes to change their data and functionality based on what they ***are***. 
In an ECS, an entity is only composed of fragments that get manipulated by processors based on which ECS components they ***have***. 

An entity is really just a small unique identifier that points to some fragments. A Processor defines a query that filters only for entities that have specific fragments. For example, a basic "movement" Processor could query for entities that have a transform and velocity component to add the velocity to their current transform position. 

Fragments are stored in memory as tightly packed arrays of other identical fragment arrangements called archetypes. Because of this, the aforementioned movement processor can be incredibly high performance because it does a simple operation on a small amount of data all at once. New functionality can easily be added by creating new fragments and processors.

Internally, Mass is similar to the existing [Unity DOTS](https://docs.unity3d.com/Packages/com.unity.entities@0.17/manual/index.html) and [FLECS](https://github.com/SanderMertens/flecs) archetype-based ECS libraries. There are many more!

<a name="sample"></a>
## 3. Sample Project
Currently, the sample features the following:

- A bare minimum movement processor to show how to set up processors.
- An entity spawner that uses a special mass-specific data asset to spawn entities in a circle defined in an Environmental Query System (EQS).
- A Mass-simulated crowd of cones that parades around the level following a ZoneGraph shape with lanes.


<!-- FIXME: Let's figure out first an index to later fill with content if you agree. -->
<!-- FIXME: I'd say we can keep the majority of content we have in here, but we should define first an index. -->

<a name="massconcepts"></a>
## 4. Mass Concepts

#### Sections

> 4.1 [Entities](#mass-entities)  
> 4.2 [Fragments](#mass-fragments)  
> 4.3 [Tags](#mass-tags)  
> 4.4 [Processors](#mass-processors)  
> 4.5 [Queries](#mass-queries)  
> 4.6 [Traits](#mass-traits)  
> 4.7 [Shared Fragments](#mass-sf)  

<a name="mass-entities"></a>
### 4.1 Entities
Unique identifiers for individual entities.

<a name="mass-fragments"></a>
### 4.2 Fragments
Raw data-only UScriptStructs that entities can own and processors can query on. Stored in chunked archetype arrays 
for quick processing.

<a name="mass-tags"></a>
### 4.3 Tags
Fragments that have no data to only be used as tags for filtering. Just bits on an archetype internally.

<a name="mass-processors"></a>
### 4.4 Processors
The main way fragments are operated on in Mass. Combine one more user-defined queries with functions that operate on the data inside them. They can also include rules that define in which order they are called in. Automatically registered with Mass by default. 

In their constructor they can define rules for their execution order and which types of game client they execute on:
<!-- FIXME: Is this a good way to describe this? Network mode is something else, right?? I need to look later. -->
```c++
//Using the built-in movement processor group
ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
  
//This executes only on clients and not the dedicated server
ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
```
On initialization, Mass creates a graph of processors using their execution rules so they execute in order.

Remember: you create the queries yourself in the header!


<a name="mass-queries"></a>
### 4.5 Queries
Processors use one or more `FMassEntityQuery` to select entities to iterate on. They are collection of fragments and tags combined with rules to filter for usually defined in `ConfigureQueries` . Queries can exclude certain fragments or even include them optionally.

Here are some basic examples from in which we add rules to a `FMassEntityQuery MoveEntitiesQuery`:
Generally we filter regular fragments by how we access them and tags by their presence only, as they have no data.
```c++	
//Entities must have an FTransformFragment and we are reading and changing it (EMassFragmentAccess::ReadWrite)
MoveEntitiesQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
//Entities must have an FMassForceFragment and we are only reading it (EMassFragmentAccess::ReadOnly)
MoveEntitiesQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
  
//Entities ALL must have an FMoverTag
MoveEntitiesQuery.AddTagRequirement<FMoverTag>(EMassFragmentPresence::All);
//NONE of the Entities may have an FStopTag
MoveEntitiesQuery.AddTagRequirement<FStopTag>(EMassFragmentPresence::None);
```

Non-tag fragments can be filtered by presence like tags with an additional `EMassFragmentPresence` parameter. It is EMassFragmentPresence::All by default.
```c++	
//Don't include entities with a HitLocation fragment!
MoveEntitiesQuery.AddRequirement<FHitLocationFragment>(EMassFragmentAccess::ReadOnly,EMassFragmentPresence::None);
```

 `EMassFragmentPresence::Optional` can be used to get a fragment to iterate on without caring about whether it is present or not.
```c++	
//We don't always have a movement speed modifier, but include it if we do.
MoveEntitiesQuery.AddRequirement<FMovementSpeedModifier>(EMassFragmentAccess::ReadOnly,EMassFragmentPresence::Optional);
```

Rarely used but still worth a mention `EMassFragmentPresence::Any` filters for entities that must at least ~one~ of the fragments marked with Any. Here is a contrived example:
```c++
FarmAnimalsQuery.AddTagRequirement<F>(EMassFragmentPresence::Any);
FarmAnimalsQuery.AddTagRequirement<FSheepTag>(EMassFragmentPresence::Any);
FarmAnimalsQuery.AddTagRequirement<FGoatTag>(EMassFragmentPresence::Any);
```
<!-- FIXME: less weird Any example? -->
#### 4.5.1 Iterating Queries

To actually use the queries we must call their `ForEachEntityChunk` function with a lambda, the Mass subsystem and execution context. Here is an example from inside the `Execute` function of a processor:
```c++
//Note that this is a lambda! If you want extra data you may need to pass something into the []
MovementEntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [](FMassExecutionContext& Context)
{
  //Get the length of the entities in our current ExecutionContext
  const int32 NumEntities = Context.GetNumEntities();

  //These are what let us read and change entity data from the query in the ForEach
  const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();

  //This one is readonly, so we don't need Mutable
  const TConstArrayView<FMassForceFragment> ForceList = Context.GetFragmentView<FMassForceFragment>();

  //Loop over every entity in the current chunk and do stuff!
  for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
  {
    FTransform& TransformToChange = TransformList[EntityIndex].GetMutableTransform();

    FVector DeltaForce = ForceList[EntityIndex].Value;

    //Multiply the amount to move by delta time from the context.
    DeltaForce = Context.GetDeltaTimeSeconds() * DeltaForce;\

    TransformToChange.AddToTranslation(DeltaForce);
  }
});
```                                                        
#### 4.5.2 Changing entities with Defer()
                                                        
Inside of the ForEachEntity we have access to the current execution context. It is the primary way we get entity data and alter their composition. Here is an example where in which we add a tag to any entity that is the has a color fragment that is color red:

```c++
EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
{

  auto ColorList = Context.GetFragmentView<FSampleColorFragment>();

  for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
  {

    if(ColorList[EntityIndex].Color == FColor::Red)
    {
      //Using the context, defer adding a tag to this entity after done processing!                                                             
      Context.Defer().AddTag<FIsRedTag>();

    }
  }

});
```

<!-- FIXMEE: kind of contrived... better real world example that isn't too crazy? -->

Here are some of the built in basic changes you can defer:

Fragments:
`Context.Defer().AddFragment<FMyTag>();`
`Context.Defer().RemoveFragment<FMyTag>();`
  
Tags:
`Context.Defer().AddTag<FMyTag>();`
`Context.Defer().RemoveTag<FMyTag>();` 
 
Destroying entities:
`Context.Defer().DestroyEntity(MyEntity);`
`Context.Defer().BatchDestroyEntities(MyEntitiesArray)`
  
  <!-- FIXME: You can create your own and use them with `Context.Defer().EmplaceCommand<>()` but that will be for later -->


<a name="mass-traits"></a>
### 4.6 Traits
Traits are C++ defined objects that declare a set of fragments and data to use for authoring new entities in a data-driven way. They usually contain fragments and data that go well together to make defining different kinds of entities in the editor simple. 

To start using traits, simply create a `DataAsset` that inherits from 
`MassEntityConfigAsset` and add new traits to it. Each trait can be expanded to set properties if it has any. There are many built-in traits including an "assorted fragments" trait to add specific fragments without making a new trait. You can also define a parent MassEntityConfigAsset to inherit the fragments from another `DataAsset`.

Traits are often used to add Shared Fragments in the form of settings.

<a name="mass-sf"></a>
### 4.7 Shared Fragments
Shared Fragments (`FMassSharedFragment`) are fragments that multiple entities can point to. This is often used for configuration that won't change for a group of entities at runtime. The archetype only needs to store one copy for many entities that share it. Hashes are used to find existing shared fragments nad to create new ones. 

Adding one to query differs from other fragments:

```c++
PositionToNiagaraFragmentQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
```

<a name="mass-o"></a>
### 4.8 Observers
Observers are processors that derive from (`UMassObserverProcessor`) in order to only operate on entities that have just added or removed one or more specific fragment types they observe. For example, you could create an observer that handles entities that just had an `FColorFragment` added to change their color to something random. They do not run every frame, but for every time a batch of entities is changed in a way that fulfills their observer operations and types. It possible to create any queries you want to use during execution of the process as usual regardless of what fragments are observed. However, have more than one kind of fragment trigger this observer you must overload the `Register` function. 

Note: Currently observers are only called during batched entity actions. This covers processors and spawners but not single entity changes from C++ as demonstrated. 

<a name="mass-pm"></a>
## 5. Mass Plugins and Modules
The Mass framework is divided into many different plugins and modules. Here's my quick overview:

<a name="mass-pm-me"></a>
### 5.1 MassEntity
The core plugin+modules manages all entity creation and storage. You should store a pointer to this subsystem in your code.

<a name="mass-pm-gp"></a>
### 5.2 MassGameplay 
A plugin with many modules with many useful fragments and processors is used throughout the framework. 
- **MassCommon**
Basic fragments like `FTransformFragment`.
- **MassMovement**
Features an important `UMassApplyMovementProcessor` processor that moves entities based on their velocity and force. Also includes a very basic sample.
- **MassRepresentation**
Processors and fragments for rendering entities in the world. They generally use an ISMC to do so.
- **MassSpawner** 
A highly configurable actor type that can spawn specific entities where you want. 
- **MassActors**
A bridge between the general UE5 actor framework and Mass. A type of fragment that turns entities into "Agents" that can exchange data in either direction (or both).
- **MassLOD**
LOD Processors that can manage different kinds of levels of detail, from rendering to ticking at different rates based on fragment settings.
- **MassReplication**
Replication support for Mass! Other modules override `UMassReplicatorBase` to replicate stuff. Entities are given a separate Network ID that gets
- **MassSignals** 
A system that lets entities send named signals to each other.
- **MassSmartObjects** 
Lets entities "claim" SmartObjects to interact with them.

<!-- This section explicitly for AI specific modules-->
<a name="mass-pm-ai"></a>
### 5.3 MassAI
There are more modules that implement AI features that I will document with more detail when I get around to it.

- **Zonegraph**
In-level splines and shapes that use config defined lanes to direct crowd entities around! Think sidewalks, roads etc.
- **StateTree**
A new lightweight AI statemachine that can work in conjunction with Mass Crowds. One of them is used to give movement targets to the cones in the parade in the sample.
