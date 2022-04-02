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
> 4. [Mass Concepts](#massconcepts)  
> 4.1 [Entities](#mass-entities)   
> 4.2 [Fragments](#mass-fragments)  
> 4.3 [Tags](#mass-tags)  
> 4.4 [Processors](#mass-processors)  
> 4.5 [Queries](#mass-queries)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.1 [Access requirements](#mass-queries-ar)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.2 [Presence requirements](#mass-queries-pr)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.3 [Iterating Queries](#mass-queries-iq)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.4 [Mutating entities with Defer()](#mass-queries-mq)  
> 4.6 [Traits](#mass-traits)  
> 4.7 [Shared Fragments](#mass-sf)  
> 4.8 [Observers](#mass-o)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.8.1 [Observing multiple Fragment/Tags](#mass-o-mft)
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
<!-- FIXME: Revise short definition. Shouldn't define entity by employing the term entity. -->
Unique identifiers for individual entities.

<a name="mass-fragments"></a>
### 4.2 Fragments
Data-only `UScriptStructs` that entities can own and processors can query on. Stored in chunked archetype arrays for quick processing.
<!-- FIXME: Elaborate on why chunked archetype arrays are faster (data locality)? Add diagram showcasing mem layout? (Use figma) -->

<a name="mass-tags"></a>
### 4.3 Tags
<!-- REV: Tags aren't fragments!! Please extend explanation. What filtering? -->
Empty `UScriptStructs` employed for filtering. 
Just bits on an archetype internally. <!-- FIXME: vvv Please carify this phrase vvv -->

<a name="mass-processors"></a>
### 4.4 Processors
The main way fragments are operated on in Mass. Combine one more user-defined queries with functions that operate on the data inside them. 
<!-- FIXME: See comment below. -->
~~They can also include rules that define in which order they are called in.~~ Automatically registered with Mass by default. 

<!-- FIXME: You don't define rules for execution order in the processor. Processors are sorted in groups, and in the processor you can configure to which group the processor belongs to, but the ordering rules are defined at a group and processor level in the ProcessingPhase. See: UMassEntitySettings -->
In their constructor they can define ~~rules for their execution order~~ and which types of game client they execute on:
```c++
//Using the built-in movement processor group
ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
  
//This executes only on clients and not the dedicated server
ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
```

<!-- FIXME: Showcase why order is important. Dependencies!! -->
On initialization, Mass creates a graph of processors using their execution rules so they execute in order.

Remember: you create the queries yourself in the header!


<a name="mass-queries"></a>
### 4.5 Queries
Processors use one or more `FMassEntityQuery` to select the entities to iterate on. 

<!-- FIXME: Please rephrase -->
They are collection of Fragments and Tags combined with rules to filter for usually defined in `ConfigureQueries`. 

<a name="mass-queries-ar"></a>
#### 4.5.1 Access requirements

Queries can define read/write access requirements for Fragments:

| `EMassFragmentAccess` | Description |
| ----------- | ----------- |
| None | No binding required. |
| ReadOnly | We want to read the data for the fragment. | 
| ReadWrite | We want to read and write the data for the fragment. | 

Here are some basic examples in which we add access rules in two Fragments from a `FMassEntityQuery MoveEntitiesQuery`:

```c++	
//Entities must have an FTransformFragment and we are reading and changing it (EMassFragmentAccess::ReadWrite)
MoveEntitiesQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	
//Entities must have an FMassForceFragment and we are only reading it (EMassFragmentAccess::ReadOnly)
MoveEntitiesQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
```

Note that Tags **do not have** access requirements, since they don't contain data.

<a name="mass-queries-pr"></a>
#### 4.5.2 Presence requirements
Queries can define presence requirements for Fragments and Tags:

| `EMassFragmentPresence` | Description |
| ----------- | ----------- |
| All | All of the required fragments must be present. Default presence requirement. |
| Any | At least one of the fragments marked any must be present. | 
| None | None of the required fragments can be present. | 
| Optional | If fragment is present we'll use it, but it does not need to be present. | 

Here are some basic examples in which we add presence rules in two Tags from a `FMassEntityQuery MoveEntitiesQuery`:
```c++
// All entities must have a FMoverTag
MoveEntitiesQuery.AddTagRequirement<FMoverTag>(EMassFragmentPresence::All);
// None of the Entities may have a FStopTag
MoveEntitiesQuery.AddTagRequirement<FStopTag>(EMassFragmentPresence::None);
```
Fragments can be filtered by presence with an additional `EMassFragmentPresence` parameter.
```c++	
// Don't include entities with a HitLocation fragment
MoveEntitiesQuery.AddRequirement<FHitLocationFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::None);
```
<!-- REVIEWME: Please Funk review this text below: -->
`EMassFragmentPresence::Optional` can be used to get an Entity to be considered for iteration without the need of containing the specified Tag or Fragment. If the Tag or Fragment exists, it will be processed.
```c++	
// We don't always have a movement speed modifier, but include it if we do
MoveEntitiesQuery.AddRequirement<FMovementSpeedModifier>(EMassFragmentAccess::ReadOnly,EMassFragmentPresence::Optional);
```

Rarely used but still worth a mention `EMassFragmentPresence::Any` filters for entities that must at least one of the fragments marked with Any. Here is a contrived example:
```c++
FarmAnimalsQuery.AddTagRequirement<FHorseTag>(EMassFragmentPresence::Any);
FarmAnimalsQuery.AddTagRequirement<FSheepTag>(EMassFragmentPresence::Any);
FarmAnimalsQuery.AddTagRequirement<FGoatTag>(EMassFragmentPresence::Any);
```

<a name="mass-queries-iq"></a>
#### 4.5.3 Iterating Queries
Queries are executed by calling `ForEachEntityChunk` member function with a lambda, passing the related `UMassEntitySubsystem` and `FMassExecutionContext`. The following example code lies inside the `Execute` function of a processor:
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
<a name="mass-queries-mq"></a>
#### 4.5.4 Mutating entities with Defer()
                                                        
Within the `ForEachEntityChunk` we have access to the current execution context. `FMassExecutionContext` enables us to get entity data and mutate their composition. The following code adds the tag `FIsRedTag` to any entity that has a color fragment with its `Color` property set to `Red`:

<!-- REV: Isn't this executed constantly? Wouldn't be adding the tag all the time? Can't we do this just once? -->

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

##### 4.5.4.1 Native mutation operations
The following Listings define the native mutations that you can defer:

Fragments:
```c++
Context.Defer().AddFragment<FMyTag>();
Context.Defer().RemoveFragment<FMyTag>();
```

Tags:
```c++
Context.Defer().AddTag<FMyTag>();
Context.Defer().RemoveTag<FMyTag>();
```
 
Destroying entities:
```c++
Context.Defer().DestroyEntity(MyEntity);
Context.Defer().BatchDestroyEntities(MyEntitiesArray);
```

##### 4.5.4.2 Custom mutation operations

It is also possible to create custom mutations by implementing your own commands and passing them through `Context.Defer().EmplaceCommand<FMyCustomComand>(...)`.

<!-- FIXME: Please complete! (later) -->

<a name="mass-traits"></a>
### 4.6 Traits
Traits are C++ defined objects that declare a set of Fragments, Tags and data for authoring new entities in a data-driven way. 

To start using traits, create a `DataAsset` that inherits from 
`MassEntityConfigAsset` and add new traits to it. Each trait can be expanded to set properties if it has any. 

![MassEntityConfigAsset](Images/massentityconfigasset.jpg)

Between the many built-in traits offered by Mass, we can find the `Assorted Fragments` trait, which holds an array of `FInstancedStruct` that enables adding fragments to this trait from the editor without the need of creating a new C++ Trait. 

![AssortedFragments](Images/assortedfragments.jpg)

<!-- FIXME: This is how ue works, I think it's not necessary -->
~~You can also define a parent MassEntityConfigAsset to inherit the fragments from another `DataAsset`.~~

<!-- FIXME: Please elaborate -->
Traits are often used to add Shared Fragments in the form of settings.


<!-- FIXME: New section, please fill with hello world example -->
#### 4.6.1 Creating a trait
You can create C++ traits!

<a name="mass-sf"></a>
### 4.7 Shared Fragments
Shared Fragments (`FMassSharedFragment`) are fragments that multiple entities can point to. This is often used for configuration that won't change for a group of entities at runtime. 

<!-- FIXME: Which archetype? Which hashes? This is a bit confusing! -->
The archetype only needs to store one copy for many entities that share it. Hashes are used to find existing shared fragments nad to create new ones. 

<!-- FIXME: Quack? x'D. Please rephrase, can't understand this -->
Adding one to query differs from other fragments:

```c++
PositionToNiagaraFragmentQuery.AddSharedRequirement<FSharedNiagaraSystemFragment>(EMassFragmentAccess::ReadWrite);
```

<a name="mass-o"></a>
### 4.8 Observers
The `UMassObserverProcessor` is a type of processor that operates on entities that have just performed a `EMassObservedOperation` over the Fragment/Tag type observed:

| `EMassObservedOperation` | Description |
| ----------- | ----------- |
| Add | The observed Fragment/Tag was added to an entity. |
| Remove | The observed Fragment/Tag was removed from an entity. | 

Observers do not run every frame, but every time a batch of entities is changed in a way that fulfills the observer requirements.

For example, you could create an observer that handles entities that just had an `FColorFragment` added to change their color:

```c++
UMSObserverOnAdd::UMSObserverOnAdd()
{
	ObservedType = FSampleColorFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UMSObserverOnAdd::ConfigureQueries()
{
	EntityQuery.AddRequirement<FSampleColorFragment>(EMassFragmentAccess::ReadWrite);
}

void UMSObserverOnAdd::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&,this](FMassExecutionContext& Context)
	{
		auto Colors = Context.GetMutableFragmentView<FSampleColorFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// When a color is added, make it random!
			Colors[EntityIndex].Color = FColor::MakeRandomColor();
		}
	});
}
```

It is also possible to create [queries](#mass-queries) to use during the execution process regardless the observed Fragment/Tag.

**Note:** _Currently_ observers are only called during batched entity actions. This covers processors and spawners but not single entity changes from C++. 

<a name="mass-o-mft"></a>
#### 4.8.1 Observing multiple Fragment/Tags
Observers can also be used to observe multiple operations and/or types. For that, override the `Register` function in `UMassObserverProcessor`: 

```c++
void UMyMassObserverProcessor::Register()
{
	check(ObservedType);
	check(MyObservedType);

	UMassObserverRegistry::GetMutable().RegisterObserver(*ObservedType, Operation, GetClass());
	UMassObserverRegistry::GetMutable().RegisterObserver(*ObservedType, MyOperation, GetClass());
	UMassObserverRegistry::GetMutable().RegisterObserver(*MyObservedType, MyOperation, GetClass());
	UMassObserverRegistry::GetMutable().RegisterObserver(*MyObservedType, Operation, GetClass());
	UMassObserverRegistry::GetMutable().RegisterObserver(*MyObservedType, EMassObservedOperation::Add, GetClass());
}
```
As noted above, it is possible to reuse the same `EMassObservedOperation` operation for multiple observed types, and vice-versa.

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
