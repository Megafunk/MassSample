# Community Mass Sample
### **Note:** This project requires `Git LFS` for it to work properly, `zip` downloads **won't work**.

#### **Authors:**
- Karl Mavko - [@Megafunk](https://github.com/Megafunk)
- Alvaro Jover - [@vorixo](https://github.com/vorixo)

Our **somewhat WIP** understanding of Unreal Engine 5's experimental Entity Component System (ECS) plugin with a small sample project. We are **not** affiliated with Epic Games and this system is actively being changed often so this information might not be totally accurate.

We are totally open to contributions, If something is wrong or you think it could be improved, feel free to [open an issue](https://github.com/Megafunk/MassSample/issues) or submit a [pull request](https://github.com/Megafunk/MassSample/pulls).

Currently built for the Unreal Engine 5 latest version binary from the Epic Games launcher.
This documentation will be updated often!

#### **Requirements:**
- Unreal Engine 5.5 (latest version as of writing) from the [Epic Games launcher](https://www.unrealengine.com/en-US/download)
- `Git` version control:
  - [Windows](https://gitforwindows.org/)
  - [Linux/Unix & macOS](https://git-scm.com/downloads)
- [Git Large File Storage](https://git-lfs.github.com/)

#### **Download instructions (Windows):**
After installing the requirements from above, follow these steps:

1. Right-Click where you wish to hold your project, then press `Git Bash Here`.

2. Within the terminal, clone the project:
	```bash
	git clone https://github.com/Megafunk/MassSample.git
	```

3. Pull LFS:
	```bash
	git lfs pull
	```
4. Once LFS finishes, close the terminal.



<!--- Introduce here table of contents -->
<a name="tocs"></a>
## Table of Contents
> 1. [Mass](#mass)  
> 2. [Entity Component System](#ecs)  
> 3. [Sample Project](#sample)  
> 4. [Mass Concepts](#massconcepts)  
> 4.1 [Entities](#mass-entities)   
> 4.2 [Fragments](#mass-fragments)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.2.1 [Shared Fragments](#mass-fragments-sf)  
> 4.3 [Tags](#mass-tags)  
> 4.4 [Subsystems](#mass-subsystems)  
> 4.5 [The archetype model](#mass-arch-mod)   
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.1 [Tags in the archetype model](#mass-arch-mod-tags)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.2 [Fragments in the archetype model](#mass-arch-mod-fragments)  
> 4.6 [Processors](#mass-processors)  
> 4.7 [Queries](#mass-queries)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.1 [Access requirements](#mass-queries-ar)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.2 [Presence requirements](#mass-queries-pr)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.3 [Iterating Queries](#mass-queries-iq)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.3 [Mutating entities with Defer()](#mass-queries-mq)  
> 4.8 [Traits](#mass-traits)  
> 4.9 [Observers](#mass-o)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.9.1 [Observers limitations](#mass-o-n)                
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.9.2 [Observing multiple Fragment/Tags](#mass-o-mft)       
> 4.10 [Multithreading](#mass-mt)  
> 5. [Common Mass operations](#mass-cm)   
> 5.1 [Spawning entities](#mass-cm-spae)  
> 5.2 [Destroying entities](#mass-cm-dsae)  
> 5.3 [Operating Entities](#mass-cm-opee)  
> 6. [Mass Plugins and Modules](#mass-pm)  
> 6.1 [MassEntity](#mass-pm-me)  
> 6.2 [MassGameplay](#mass-pm-gp)  
> 6.3 [MassAI](#mass-pm-ai)  
> 7. [Other Resources](#mass-or)  


<!--ProposalFUNK: How about a "FAQ" of sorts for debugging etc? like: -->
<!--Why isn't anything being used in my query?-->
<!--When should I use Mass?-->
<!--General debug UI info etc-->
<!-- REVIEWMEVORI: I would split it in two different subsections:
- Common issues
- Mass FAQ

Although I don't know if these should have their own section each one, or if we can group them under the same section (making the subsections).
 -->

<a name="mass"></a>
## 1. Mass
Mass is Unreal's in-house ECS framework! Technically, [Sequencer](https://docs.unrealengine.com/4.26/en-US/AnimatingObjects/Sequencer/Overview/) already used one internally but it wasn't intended for gameplay code. Mass was created by the AI team at Epic Games to facilitate massive crowd simulations, but has grown to include many other features as well. It was featured in the [Matrix Awakens demo](https://www.unrealengine.com/en-US/blog/introducing-the-matrix-awakens-an-unreal-engine-5-experience) Epic released in 2021.

<a name="ecs"></a>
## 2. Entity Component System 
Mass is an archetype-based Entity Componenet System. If you already know what that is you can skip ahead to the next section.

In Mass, some ECS terminology differs from the norm in order to not get confused with existing unreal code:
| ECS | Mass |
| ----------- | ----------- |
| Entity | Entity |
| Component | Fragment | 
| System | Processor | 

Typical Unreal Engine game code is expressed as Actor objects that inherit from parent classes to change their data and functionality based on what they ***are***. 
In an ECS, an entity is only composed of fragments that get manipulated by processors based on which ECS components they ***have***. 

An entity is really just a small unique identifier that points to some fragments. A Processor defines a query that filters only for entities that have specific fragments. For example, a basic "movement" Processor could query for entities that have a transform and velocity component to add the velocity to their current transform position. 

Fragments are stored in memory as tightly packed arrays of other identical fragment arrangements called archetypes. Because of this, the aforementioned movement processor can be incredibly high performance because it does a simple operation on a small amount of data all at once. New functionality can easily be added by creating new fragments and processors.

Internally, Mass is similar to the existing [Unity DOTS](https://docs.unity3d.com/Packages/com.unity.entities@0.17/manual/index.html) and [FLECS](https://github.com/SanderMertens/flecs) archetype-based ECS libraries. There are many more!

<a name="sample"></a>
## 3. Sample Project
Currently, the sample features the following:

- A bare minimum movement processor to show how to set up processors.
- An example of how to use Mass spawners for zonegraph and EQS.
- Mass-simulated crowd of cones that parades around the level following a ZoneGraph shape with lanes.
- Linetraced projectile simulation example.
- Simple 3d hashgrid for entities.
- Very basic Mass blueprint integration.
- Grouped niagara rendering for entities.


<a name="massconcepts"></a>
## 4. Mass Concepts

#### Sections

> 4.1 [Entities](#mass-entities)  
> 4.2 [Fragments](#mass-fragments)  
> 4.3 [Tags](#mass-tags)  
> 4.4 [Subsystems](#mass-subsystems)  
> 4.5 [The archetype model](#mass-arch-mod)   
> 4.6 [Processors](#mass-processors)  
> 4.7 [Queries](#mass-queries)  
> 4.8 [Traits](#mass-traits)  
> 4.9 [Observers](#mass-o)

<a name="mass-entities"></a>
### 4.1 Entities
Small unique identifiers that point to a combination of [fragments](#mass-fragments) and [tags](#mass-tags) in memory. Entities are mainly a simple integer ID. For example, entity 103 might point to a single projectile with transform, velocity, and damage data.

<!-- TODO: Document the different ways in which we can identify an entity in mass and their purpose? FMassEntityHandle, FMassEntity, FMassEntityView?? -->

<a name="mass-fragments"></a>
### 4.2 Fragments
Data-only `UStructs` that entities can own and processors can query on. To create a fragment, inherit from [`FMassFragment`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassFragment/). 

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FLifeTimeFragment : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};
```

With `FMassFragment`s each entity gets its own fragment data, to share data across many entities, we can use a [shared fragment](#mass-fragments-sf). 

<a name="mass-fragments-sf"></a>
#### 4.2.1 Shared Fragments
A Shared Fragment is a type of Fragment that multiple entities can point to. This is often used for configuration common to a group of entities, like LOD or replication settings. To create a shared fragment, inherit from [`FMassSharedFragment`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassSharedFragment/).

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FVisibilityDistanceSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Distance;
};
```

In the example above, all the entities containing the `FVisibilityDistanceSharedFragment` will see the same `Distance` value. If an entity modifies the `Distance` value, the rest of the entities with this fragment will see the change as they share it through the archetype. Shared fragments are generally added from Mass Traits. 

Make sure your shared fragments are Crc hashable or else you may not actually create a new instance when you call `GetOrCreateSharedFragmentByHash`. You can actually pass in your own hash with `GetOrCreateSharedFragmentByHash`, which can help if you prefer to control what makes each one unique.

Thanks to this sharing data requirement, the Mass entity manager only needs to store one Shared Fragment for the entities that use it.

<a name="mass-tags"></a>
### 4.3 Tags
Empty `UScriptStructs` that [processors](#mass-processors) can use to filter entities to process based on their presence/absence. To create a tag, inherit from [`FMassTag`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassTag/).

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};
```
**Note:** Tags should never contain member properties.

<a name="mass-subsystems"></a>
### 4.4 Subsystems
Starting in UE 5.1, Mass enhanced its API to support [`UWorldSubsystems`](https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Subsystems/UWorldSubsystem/) in our [Processors](#mass-processors). This provides a way to create encapsulated functionality to operate Entities. First, inherit from `UWorldSubsystem` and define its basic interface alongside your functions and variables:

```c++
UCLASS()
class MASSCOMMUNITYSAMPLE_API UMyWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void Write(int32 InNumber);
	int32 Read() const;

protected:
	// UWorldSubsystem begin interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UWorldSubsystem end interface
	
private:
	UE_MT_DECLARE_RW_ACCESS_DETECTOR(AccessDetector);
	int Number = 0;
};
```

Following next, we present an implementation example of the provided interface above (see `MassEntityTestTypes.h`):

```c++
void UMyWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Initialize dependent subsystems before calling super
	Collection.InitializeDependency(UMyOtherSubsystemOne::StaticClass());
	Collection.InitializeDependency(UMyOtherSubsystemTwo::StaticClass());
	Super::Initialize(Collection);

	// In here you can hook to delegates!
	// ie: OnFireHandle = FExample::OnFireDelegate.AddUObject(this, &UMyWorldSubsystem::OnFire);
}

void UMyWorldSubsystem::Deinitialize()
{
	// In here you can unhook from delegates
	// ie: FExample::OnFireDelegate.Remove(OnFireHandle);
	Super::Deinitialize();
}

void UMyWorldSubsystem::Write(int32 InNumber)
{
	UE_MT_SCOPED_WRITE_ACCESS(AccessDetector);
	Number = InNumber;
}

int32 UMyWorldSubsystem::Read() const
{
	UE_MT_SCOPED_READ_ACCESS(AccessDetector);
	return Number;
}
```
The code above is multithread-friendly, hence the `UE_MT_X` tokens.

<!-- FIXMEVORI-UE5: Maybe a section exposing the different UE_MT_X tokens? (Get informed about their full scope) -->

Finally, to make this world subsystem compatible with Mass, you must define its subsystem traits, which inform Mass about its [parallel capabilities](#mass-mt). In this case, our subsystem supports parallel reads:

```c++
/**
 * Traits describing how a given piece of code can be used by Mass. 
 * We require author or user of a given subsystem to 
 * define its traits. To do it add the following in an accessible location. 
 */
template<>
struct TMassExternalSubsystemTraits<UMyWorldSubsystem> final
{
	enum
	{
		ThreadSafeRead = true,
		ThreadSafeWrite = false,
	};
};
/**
* this will let Mass know it can access UMyWorldSubsystem on any thread.
*
* This information is being used to calculate processor and query 
* dependencies as well as appropriate distribution of
* calculations across threads.
*/
```
If you want to use a `UWorldSubsystem` that has not had its traits defined before and you cannot modify its header explicitly, you can add the subsystem trait information in a separate header file (see `MassGameplayExternalTraits.h`).

<a name="mass-arch-mod"></a>
### 4.5 The archetype model
As mentioned previously, an entity is a unique combination of fragments and tags. Mass calls each of these combinations archetypes. For example, given three different combinations used by our entities, we would generate three archetypes:

![MassArchetypeDefinition](Images/arche-entity-type.png)

The `FMassArchetypeData` struct represents an archetype in Mass internally. 

<a name="mass-arch-mod-tags"></a>
#### 4.5.1 Tags in the archetype model
Each archetype (`FMassArchetypeData`) holds a bitset (`TScriptStructTypeBitSet<FMassTag>`) that contains the tag presence information, whereas each bit in the bitset represents whether a tag exists in the archetype or not.

![MassArchetypeTags](Images/arche-tags.png)

Following the previous example, *Archetype 0* and *Archetype 2* contain the tags: *TagA*, *TagC* and *TagD*; while *Archetype 1* contains *TagC* and *TagD*. Which makes the combination of *Fragment A* and *Fragment B* to be split in two different archetypes.

<a name="mass-arch-mod-fragments"></a>
#### 4.5.2 Fragments in the archetype model
At the same time, each archetype holds an array of chunks (`FMassArchetypeChunk`) with fragment data.

Each chunk contains a subset of the entities included in our archetype where data is organized in a pseudo-[struct-of-arrays](https://en.wikipedia.org/wiki/AoS_and_SoA#Structure_of_arrays) way:

![MassArchetypeChunks](Images/arche-chunks.png)

The following Figure represents the archetypes from the example above in memory:

![MassArchetypeMemory](Images/arche-mem.png)

By having this pseudo-[struct-of-arrays](https://en.wikipedia.org/wiki/AoS_and_SoA#Structure_of_arrays) data layout divided in multiple chunks, we are allowing a great number of whole-entities to fit in the CPU cache. 

This is thanks to the chunk partitoning, since without it, we wouldn't have as many whole-entities fit in cache, as the following diagram displays:

![MassArchetypeCache](Images/arche-cache-friendly.png)

In the above example, the Chunked Archetype gets whole-entities in cache, while the Linear Archetype gets all the *A Fragments* in cache, but cannot fit each fragment of an entity.

The Linear approach would be fast if we would only access the *A Fragment* when iterating entities, however, this is almost never the case. Usually, when we iterate entities we tend to access multiple fragments, so it is convenient to have them all in cache, which is what the chunk partitioning provides.

The chunk size (`UE::Mass::ChunkSize`) has been conveniently set based on next-gen cache sizes (128 bytes per line and 1024 cache lines). This means that archetypes with more bits of fragment data will contain less entities per chunk.

**Note:** It is relevant to note that a cache miss would be produced every time we want to access a fragment that isn't on cache for a given entity.

<a name="mass-processors"></a>
### 4.6 Processors
Processors combine multiple user-defined [queries](#mass-queries) with functions that compute entities.

Unreal classes deriving from UMassProcessor are automatically registered with Mass and added to the `EMassProcessingPhase::PrePhsysics` processing phase by default. Each `EMassProcessingPhase` relates to an `ETickingGroup`, meaning that, by default, processors tick every frame in their given processing phase.
They can also be created and registered with the `UMassSimulationSubsystem` but the common case is to create a new type. 
Users can configure to which processing phase their processor belongs by modifying the `ProcessingPhase` variable included in `UMassProcessor`: 

| `EMassProcessingPhase` | Related `ETickingGroup` | Description |
| ----------- | ----------- | ----------- |
| `PrePhysics` | `TG_PrePhysics` | Executes before physics simulation starts. |
| `StartPhysics` | `TG_StartPhysics` | Special tick group that starts physics simulation. | 
| `DuringPhysics` | `TG_DuringPhysics` | Executes in parallel with the physics simulation work. | 
| `EndPhysics` | `TG_EndPhysics` |Special tick group that ends physics simulation. | 
| `PostPhysics` | `TG_PostPhysics` |Executes after rigid body and cloth simulation. | 
| `FrameEnd` | `TG_LastDemotable` | Catchall for anything demoted to the end. | 


In their constructor, processors can define rules for their execution order, the processing phase and which type of game clients they execute on:
```c++
UMyProcessor::UMyProcessor()
{
	// This processor is registered with mass by just existing! This is the default behaviour of all processors.
	bAutoRegisterWithProcessingPhases = true;
	// Setting the processing phase explicitly
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	// Using the built-in movement processor group
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	// You can also define other processors that require to run before or after this one
	ExecutionOrder.ExecuteAfter.Add(TEXT("MSMovementProcessor"));
	// This executes only on Clients and Standalone
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	// This processor should not be multithreaded
	bRequiresGameThreadExecution = true;
}
```

On initialization, Mass creates a dependency graph of processors using their execution rules so they execute in order (ie: In the example above we make sure to move our entities with `MSMovementProcessor` before we call `Execute` in `UMyProcessor`).

The `ExecutionFlags` variable indicates whether this processor should be executed on `Standalone`, `Server` or `Client`.

By default [all processors are multithreaded](#mass-mt), however, they can also be configured to run in a single-thread if necessary by setting `bRequiresGameThreadExecution` to `true`.

**Note:** Mass ships with a series of processors that are designed to be inherited and extended with custom logic. ie: The visualization and LOD processors. 

<a name="mass-queries"></a>
### 4.7 Queries
Queries (`FMassEntityQuery`) filter and iterate entities given a series of rules based on Fragment and Tag presence.

Processors can define multiple `FMassEntityQuery`s and should override the `ConfigureQueries` to add rules to the different queries defined in the processor's header:

```c++
void UMyProcessor::ConfigureQueries()
{
	MyQuery.AddTagRequirement<FMoverTag>(EMassFragmentPresence::All);
	MyQuery.AddRequirement<FHitLocationFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	MyQuery.AddSubsystemRequirement<UMassDebuggerSubsystem>(EMassFragmentAccess::ReadWrite);
	MyQuery.RegisterWithProcessor(*this);

	ProcessorRequirements.AddSubsystemRequirement<UMassDebuggerSubsystem>(EMassFragmentAccess::ReadWrite);
}
```

To execute queries on a processor, we must register them by calling `RegisterWithProcessor` passing the processor as a parameter. `FMassEntityQuery` also offers a parameter constructor that calls `RegisterWithProcessor`, which is employed in some processors from various Mass modules (ie: `UDebugVisLocationProcessor`).

`ProcessorRequirements` is a special query part of `UMassProcessor` that holds all the `UWorldSubsystem`s that get accessed in the `Execute` function outside the queries scope. In the example above, `UMassDebuggerSubsystem` gets accessed within `MyQuery`'s scope (`MyQuery.AddSubsystemRequirement`) and in the `Execution` function scope (`ProcessorRequirements.AddSubsystemRequirement`).

Queries are executed by calling the `ForEachEntityChunk` member function with a lambda, passing the related `FMassEntityManager` and `FMassExecutionContext`. 

Processors execute queries within their `Execute` function:

```c++
void UMyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	//Note that this is a lambda! If you want extra data you may need to pass it in the [] operator
	MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		//Loop over every entity in the current chunk and do stuff!
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// ...
		}
	});
}
```
Be aware that the index we employ to iterate entities, in this case `EntityIndex`, doesn't identify uniquely your entities along time, since chunks' disposition may change and an entity that has an index this frame, may be in a different chunk with a different index in the next frame.

**Note:** Queries can also be created and iterated outside processors.

<a name="mass-queries-ar"></a>
#### 4.7.1 Access requirements

Queries can define read/write access requirements for Fragments and Subsystems:

| `EMassFragmentAccess` | Description |
| ----------- | ----------- |
| `None` | No binding required. |
| `ReadOnly` | We want to read the data for the fragment/subsystem. | 
| `ReadWrite` | We want to read and write the data for the fragment/subsystem. | 

`FMassFragment`s use `AddRequirement` to add access and presence requirement to our fragments. While `FMassSharedFragment`s employ `AddSharedRequirement`. Finally, `UWorldSubsystem`s use `AddSubsystemRequirement`. 

Here are some basic examples in which we add access rules in two Fragments from a `FMassEntityQuery MyQuery`:

```c++	
void UMyProcessor::ConfigureQueries()
{
	// Entities must have an FTransformFragment and we are reading and writing it (EMassFragmentAccess::ReadWrite)
	MyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
		
	// Entities must have an FMassForceFragment and we are only reading it (EMassFragmentAccess::ReadOnly)
	MyQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);

	// Entities must have a common FClockSharedFragment that can be read and written
	MyQuery.AddSharedRequirement<FClockSharedFragment>(EMassFragmentAccess::ReadWrite);

	// Entities must have a UMassDebuggerSubsystem that can be read and written
	MyQuery.AddSubsystemRequirement<UMassDebuggerSubsystem>(EMassFragmentAccess::ReadWrite);

	// Registering the query with UMyProcessor
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachEntityChunk`s can use the following functions to access `ReadOnly` or `ReadWrite` data according to the access requirement:

| `EMassFragmentAccess` | Type | Function |Description |
| ----------- | ----------- | ----------- | ----------- |
| `ReadOnly` | Fragment | `GetFragmentView` | Returns a read only `TConstArrayView` containing the data of our `ReadOnly` fragment. |
| `ReadWrite` | Fragment | `GetMutableFragmentView` | Returns a writable `TArrayView` containing de data of our `ReadWrite` fragment. | 
| `ReadOnly` | Shared Fragment | `GetConstSharedFragment` | Returns a constant reference to our read only shared fragment. |
| `ReadWrite` | Shared Fragment | `GetMutableSharedFragment` | Returns a reference of our writable shared fragment. | 
| `ReadOnly` | Subsystem | `GetSubsystemChecked` | Returns a read only constant reference to our world subsystem. |
| `ReadWrite` | Subsystem | `GetMutableSubsystemChecked` | Returns a reference of our writable shared world subsystem. | 

Find below an example:

```c++
MyQuery.ForEachEntityChunk(EntityManager, Context, [this, World = EntityManager.GetWorld()](FMassExecutionContext& Context)
{
	UMassDebuggerSubsystem& Debugger = Context.GetMutableSubsystemChecked<UMassDebuggerSubsystem>(World);

	const auto TransformList = Context.GetFragmentView<FTransformFragment>();
	const auto ForceList = Context.GetMutableFragmentView<FMassForceFragment>();

	for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
	{
		FTransform& TransformToChange = TransformList[EntityIndex].GetMutableTransform();
		const FVector DeltaForce = Context.GetDeltaTimeSeconds() * ForceList[EntityIndex].Value;
		TransformToChange.AddToTranslation(DeltaForce);
		Debugger.AddShape(EMassEntityDebugShape::Box, TransformToChange.GetLocation(), 10.f);
	}
});
```

**Note:** Tags do not have access requirements since they don't contain data.

<a name="mass-queries-pr"></a>
#### 4.7.2 Presence requirements
Queries can define presence requirements for Fragments and Tags:

| `EMassFragmentPresence` | Description                                                                       |
| ----------- |-----------------------------------------------------------------------------------|
| All | All of the required fragments/tags must be present. Default presence requirement. |
| Any | At least one of the fragments/tags marked any must be present.                    | 
| None | None of the required fragments/tags can be present.                               | 
| Optional | If fragment/tag is present we'll use it, but it does not need to be present.      | 

##### 4.7.2.1 Presence requirements in Tags
To add presence rules to Tags, use `AddTagRequirement`.   
```c++
void UMyProcessor::ConfigureQueries()
{
	// Entities are considered for iteration without the need of containing the specified Tag
	MyQuery.AddTagRequirement<FOptionalTag>(EMassFragmentPresence::Optional);
	// Entities must at least have the FHorseTag or the FSheepTag
	MyQuery.AddTagRequirement<FHorseTag>(EMassFragmentPresence::Any);
	MyQuery.AddTagRequirement<FSheepTag>(EMassFragmentPresence::Any);
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachChunk`s can use `DoesArchetypeHaveTag` to determine if the current archetype contains the the Tag:

```c++
MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	if(Context.DoesArchetypeHaveTag<FOptionalTag>())
	{
		// I do have the FOptionalTag tag!!
	}

	// Same with Tags marked with Any
	if(Context.DoesArchetypeHaveTag<FHorseTag>())
	{
		// I do have the FHorseTag tag!!
	}
	if(Context.DoesArchetypeHaveTag<FSheepTag>())
	{
		// I do have the FSheepTag tag!!
	}
});
```

##### 4.7.2.2 Presence requirements in Fragments
Fragments and shared fragments can define presence rules in an additional `EMassFragmentPresence` parameter through `AddRequirement` and `AddSharedRequirement`, respectively.

```c++
void UMyProcessor::ConfigureQueries()
{
	// Entities are considered for iteration without the need of containing the specified Fragment
	MyQuery.AddRequirement<FMyOptionalFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	// Entities must at least have the FHorseFragment or the FSheepFragment
	MyQuery.AddRequirement<FHorseFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	MyQuery.AddRequirement<FSheepFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachChunk`s can use the length of the `Optional`/`Any` fragment's `TArrayView` to determine if the current chunk contains the Fragment before accessing it:

```c++
MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	const auto OptionalFragmentList = Context.GetMutableFragmentView<FMyOptionalFragment>();
	const auto HorseFragmentList = Context.GetMutableFragmentView<FHorseFragment>();	
	const auto SheepFragmentList = Context.GetMutableFragmentView<FSheepFragment>();
	for (int32 i = 0; i < Context.GetNumEntities(); ++i)
	{
		// An optional fragment array is present in our current chunk if the OptionalFragmentList isn't empty
		if(OptionalFragmentList.Num() > 0)
		{
			// Now that we know it is safe to do so, we can compute
			OptionalFragmentList[i].DoOptionalStuff();
		}

		// Same with fragments marked with Any
		if(HorseFragmentList.Num() > 0)
		{
			HorseFragmentList[i].DoHorseStuff();
		}
		if(SheepFragmentList.Num() > 0)
		{
			SheepFragmentList[i].DoSheepStuff();
		}		
	}
});
```
<!-- REVIEWMEVORI: Maybe move to common Mass operations!! Spawning/Destroying subsections, although I think that wouldn't hurt having this here, and then referencing it back in the common mass operation section -->
<a name="mass-queries-mq"></a>
#### 4.7.3 Mutating entities with `Defer()`
                                                        
Within the `ForEachEntityChunk` we have access to the current execution context. `FMassExecutionContext` enables us to get entity data and mutate their composition. The following code adds the tag `FDead` to any entity that has a health fragment with its `Health` variable less or equal to 0, at the same time, as we define in `ConfigureQueries`, after the `FDead` tag is added, the entity won't be considered for iteration (`EMassFragmentPresence::None`):

```c++
void UDeathProcessor::ConfigureQueries()
{
	// All the entities processed in this query must have the FHealthFragment fragment
	DeclareDeathQuery.AddRequirement<FHealthFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::All);
	// Entities processed by this queries shouldn't have the FDead tag, as this query adds the FDead tag
	DeclareDeathQuery.AddTagRequirement<FDead>(EMassFragmentPresence::None);
	DeclareDeathQuery.RegisterWithProcessor(*this);
}

void UDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	DeclareDeathQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
	{
		auto HealthList = Context.GetFragmentView<FHealthFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			if(HealthList[EntityIndex].Health <= 0.f)
			{
				// Adding a tag to this entity when the deferred commands get flushed
				FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);
				Context.Defer().AddTag<FDead>(EntityHandle);
			}
		}
	});
}
```

In order to Defer Entity mutations we require to obtain the handle (`FMassEntityHandle`) of the Entities we wish to modify. `FMassExecutionContext` holds an array with all the Entity handles. We can access it through two different methods:

| Plurality | Code |
| ----------- | ----------- |
| Singular | `FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);` |
| Plural | `auto EntityHandleArray = Context.GetEntities();` | 

The following Subsections will employ the keywords `EntityHandle` and `EntityHandleArray` when handling singular or plural operations, respectively.


##### 4.7.3.1 Basic mutation operations
The following Listings define the native mutations that you can defer:

Deferring is commonly done from a processor's `FMassExecutionContext` with `.Defer()` but these can be done outside processing with a `EntityManager->Defer()` call. 

Fragments:
```c++
Context.Defer().AddFragment<FMyFragment>(EntityHandle);
Context.Defer().RemoveFragment<FMyFragment>(EntityHandle);
```

Tags:
```c++
Context.Defer().AddTag<FMyTag>(EntityHandle);
Context.Defer().RemoveTag<FMyTag>(EntityHandle);
Context.Defer().SwapTags<FOldTag, FNewTag>(EntityHandle);
```
 
Destroying entities:
```c++
Context.Defer().DestroyEntity(EntityHandle);
Context.Defer().DestroyEntities(EntityHandleArray);
```
These are all convenient wrappers for the internal template based deferred commands.

##### 4.7.3.2 Advanced mutation operations
There is a set of `FCommandBufferEntryBase` commands that can be used to defer some more useful entity mutations. The following subsections provide an overview. 

###### 4.7.3.2.1 `FMassCommandAddFragmentInstanceList`
Defers adding new fragment data to an existing entity. 

In the example below we mutate the `FHitResultFragment` with HitResult data, and a `FSampleColorFragment` fragment with a new color and add (or set if already present) them to an existing entity.

```c++
FHitResultFragment HitResultFragment;
FSampleColorFragment ColorFragment = FSampleColorFragment(Color);

// Sets fragment data on an existing entity
EntityManager->Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, ColorFragment, HitResultFragment);

// It can add single fragment instances as well, and should set data on existing fragments safely
EntityManager->Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, SomeOtherFragment);
```

<a name="mass-queries-FBuildEntityFromFragmentInstances"></a>
###### 4.7.3.2.2 `FMassCommandBuildEntity`
Defers Creating an Entity and adds a list of fragments with data to it.

```c++
FTransformFragment MyTransformFragment;
MyTransformFragment.SetTransform(FTransform::Identity);

// We build a new entity and add data to it in one command!
EntityManager->Defer().PushCommand<FMassCommandBuildEntity>(ReserverdEntity, MyTransformFragment, SomeOtherFragment);
));
```

###### 4.7.3.2.3 `FMassCommandBuildEntityWithSharedFragments` 
Similar to `FMassCommandBuildEntity` but it takes a `FMassArchetypeSharedFragmentValues` struct to set shared fragment values on the entity as well. This requires some extra work to find or create the shared fragment.
```c++
FMassArchetypeSharedFragmentValues SharedFragmentValues;
// This is what traits use to create their shared fragment info as well
FConstSharedStruct& SharedFragment = EntityManager->GetOrCreateConstSharedFragment(MySharedFragment);
SharedFragmentValues.AddConstSharedFragment(SharedFragment);

// MoveTemp is required here...
EntityManager->Defer().PushCommand<FMassCommandBuildEntityWithSharedFragments>(EntityHandle, MoveTemp(SharedFragmentValues), TransformFragment, AnotherFragmentEtc);
```

<!-- FIXMEVORI: For consistency, lets add as a title the name of the command, however in this one I'm not sure which ones we should include -->
##### 4.7.3.2.4 `FMassDeferredSetCommand`
Defers the execution of the `TFunction` lambda passed in as a parameter. It is useful for performing Mass-related operations that none of the other commands cover. This is a smart way to handle Actor mutations, as [those usually need to happen on the main thread](https://vkguide.dev/docs/extra-chapter/multithreading/#ways-of-using-multithreading-in-game-engines).

```c++
EntityManager->Defer().PushCommand<FMassDeferredSetCommand>(
   [&](FMassEntityManager& Manager)
  {
      	// This runs when the deferred commands are flushed
      	MyActor.DoGameThreadWork();
      	// Regular Mass manager calls can happen in here as well. For example:
  	EntityManager.BuildEntity(ReservedEntity, InstanceStructs, EntityTemplate.GetSharedFragmentValues());
  });
```

**Note:** The `TFunction` lambda does have a FMassEntityManager& as a function parameter you should include in every lambda using this command.

<!-- FIXMEVORI: What is this? maybe we need a code example, since the example above doesn't cover it -->
`FMassDeferredCreateCommand`,`FMassDeferredSetCommand` and the other similarly named types are each templated to set specific `EMassCommandOperationType`

These are designed to organize deferred commands into different operation types. For example: we want to create entities before we change fragments on them!
Here they are and what they do in order when commands are flushed:

| Operation         |                                                |
|-------------------|------------------------------------------------|
| Create            | Creating new entities.                         |
| Add               | Adding fragments/tags                          |
| Remove            | Removing fragments/tags                        |
| ChangeComposition | Adding and removing tags/fragments.            |
| Set               | Changing fragment data (also adding fragments) |
| None              | Default value, always executed last.           |


<!-- REVIEWMEFUNK: I think this section is a bit overkill and might mislead people to thinking they need to make a new template to do anything. They could probably figure out how to do this on their own by just reading the source. -->
<!-- Do you think FMassDeferredSetCommand can cover them all? Isn't it worth to mention how to create new commands? Remember that this doc is to show how to use mass, so before having the command from above it was a bit tricky -->

[//]: # ()
[//]: # (##### 4.7.3.2.7 Custom commands)
[//]: # (It is possible to create custom mutations by implementing your own commands derived from `FCommandBufferEntryBase`.)
[//]: # ()
[//]: # (```c++)
[//]: # (Context.Defer&#40;&#41;.EmplaceCommand<FMyCustomComand>&#40;...&#41;)
[//]: # (```)
[//]: # ()
[//]: # (The command needs to have a constructor and to override `FCommandBufferEntryBase::Execute&#40;&#41;` but in order to correctly trigger observers two extra steps are required:)
[//]: # ()
[//]: # (1. Setting the `Type` definition to either `ECommandBufferOperationType::Remove` or `ECommandBufferOperationType::Add` in the header.)
[//]: # (```c++)
[//]: # (enum)
[//]: # ({)
[//]: # (	Type = ECommandBufferOperationType::Add)
[//]: # (};)
[//]: # (```)
[//]: # (2. Implementing &#40;not overriding&#41; `AppendAffectedEntitiesPerType` and calling functions on the passed in `FMassCommandsObservedTypes` as needed. Here we are adding a changed `Tag` and changed `Fragment`. `TargetEntity` is a member of the parent struct.)
[//]: # (```c++)
[//]: # (void AppendAffectedEntitiesPerType&#40;FMassCommandsObservedTypes& ObservedTypes&#41;)
[//]: # ({)
[//]: # (	ObservedTypes.TagAdded&#40;TagType, TargetEntity&#41;;	)
[//]: # (	ObservedTypes.FragmentAdded&#40;FragmentType, TargetEntity&#41;;)
[//]: # (})
[//]: # (```)
[//]: # ()

<a name="mass-traits"></a>
### 4.8 Traits
Traits are C++ defined objects that declare a set of Fragments, Tags and data for authoring new entities in a data-driven way. 

To start using traits, create a `DataAsset` that inherits from 
`UMassEntityConfigAsset` and add new traits to it. Each trait can be expanded to set properties if it has any. 

In addition, it is possible to inherit Fragments from another `UMassEntityConfigAsset` by setting it in the `Parent` field.

![MassEntityConfigAsset](Images/massentityconfigasset.jpg)

Between the many built-in traits offered by Mass, we can find the `Assorted Fragments` trait, which holds an array of `FInstancedStruct` that enables adding Fragments to this trait from the editor without the need of creating a new C++ Trait. 

![AssortedFragments](Images/assortedfragments.jpg)


<!-- FIXME: Please elaborate -->
<!-- REVIEWMEFUNK kind of hard to talk about it too much here with the other section existing -->
Traits are often used to add Shared Fragments in the form of settings. For example, our visualization traits save memory by sharing which mesh they are displaying, parameters etc. Configs with the same settings will share the same Shared Fragment.


#### 4.8.1 Creating a trait
Traits are created by inheriting `UMassEntityTraitBase` and overriding `BuildTemplate`. Here is a very basic example:

```c++
UCLASS(meta = (DisplayName = "Debug Printing"))
class MASSCOMMUNITYSAMPLE_API UMSDebugTagTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override
	{
		// Adding a tag
		BuildContext.AddTag<FMassSampleDebuggableTag>();
		
		// Adding a fragment
		BuildContext.AddFragment<FTransformFragment>();

		// _GetRef lets us mutate the fragment
		BuildContext.AddFragment_GetRef<FSampleColorFragment>().Color = UserSetColor;
	};

	// Editable in the editor property list for this asset
	UPROPERTY(EditAnywhere)
	FColor UserSetColor;
};
```
**Note:** We recommend looking at the many existing traits in this sample and the mass modules for a better overview. For the most part, they are fairly simple UObjects that occasionally have extra code to make sure the fragments are all valid and set correctly. 


 <!--REVIEWMEFUNK moved it up to this section "-->
##### Shared Fragments

 Here is a partial `BuildTemplate` example for adding a shared struct, which can do some extra work to see if a shared fragment identical to the new one already exists:
```c++
	//Create the actual fragment struct and set up the data for it however you like 
	FMySharedSettings MyFragment;
	MyFragment.MyValue = UserSetValue;

	//Get a hash of a FConstStructView of said fragment and store it
	uint32 MySharedFragmentHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(MyFragment));
	
	//Search the Mass Entity subsystem for an identical struct with the hash. If there are none, make a new one with the set fragment.
	FSharedStruct MySharedFragment = 
		EntityManager.GetOrCreateSharedFragment<FMySharedSettings>(MySharedFragmentHash, MyFragment);

	//Finally, add the shared fragment to the BuildContext!
	BuildContext.AddSharedFragment(MySharedFragment);
```


#### 4.8.2 Validating traits
Traits can override `ValidateTemplate` to provide custom validation code for the trait. Native traits use this function to log  errors and/or change the `BuildContext` if required. This function is called after `BuildTemplate` and is invoked for all of the traits of the current template.

In the following snippet, we check if a field of the trait is `nullptr` and log an error:
```c++
void UMSNiagaraRepresentationTrait::ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	// If our shared niagara system is null, show an error!
	if (!SharedNiagaraSystem)
	{
		UE_VLOG(&World, LogMass, Error, TEXT("SharedNiagaraSystem is null!"));
		return;
	}
}
```

<a name="mass-o"></a>
### 4.9 Observers
The `UMassObserverProcessor` is a type of processor that operates on entities that have just performed a `EMassObservedOperation` over the Fragment/Tag type observed:

| `EMassObservedOperation` | Description |
| ----------- | ----------- |
| Add | The observed Fragment/Tag was added to an entity. |
| Remove | The observed Fragment/Tag was removed from an entity. | 

Observers do not run every frame, but every time a batch of entities is changed in a way that fulfills the observer requirements.

For example, this observer changes the color to the entities that just had an `FColorFragment` added:

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

void UMSObserverOnAdd::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [&,this](FMassExecutionContext& Context)
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
<!-- FIXMEFUNK: What happened with this section? :( -->
<!-- REVIEWMEVORI: Lots of source changes, since it's still not really clear when observers fire I felt the need to get extra specific here... 
It's getting to the point where the only things that don't trigger them would be easier to list out. I really think Epic should mark the API calls that don't do this with _Internal or something -->
<a name="mass-o-n"></a>
#### 4.9.1 Entity Manager Observer calls
 - In order for observers to fire, something must call alert the observer manager to the change in composition.
- [Deferred commands](#mass-queries-mq) that change entities will trigger observers.

If your code relies on observers firing immediately you should make sure the Mass function calls you are making actually alert observers to changes. 
In earlier versions there were some functions that skipped calling observers but as of 5.5 nearly all ways of changing entity composition are covered.

[This commit](https://github.com/EpicGames/UnrealEngine/commit/2b883dec5f6c821648f2d6005ac06e704099dbd9
) on ue5-main has rectified this issue for most mass subsystem-level calls. 

If you need to, asking the observer manager to trigger changes should only require calling `OnCompositionChanged()` with the delta of newly added or removed components.
<!-- FIXMEFUNK: This is kind of a wacky example. I assume most people who need this might 
```c++
EntityManager.GetObserverManager().OnCompositionChanged(
          FMassArchetypeEntityCollection(MyEntityArchetypeHandle, Entity,
          FMassArchetypeEntityCollection::NoDuplicates)
		, EntityTemplate.GetCompositionDescriptor()
		, EMassObservedOperation::Add))
```
<!-- FIXMEVORI: I'll review this the next day -->
<a name="mass-o-mft"></a>
#### 4.9.2 Observing multiple Fragment/Tags
Observers can also be used to observe multiple operations and/or types. For that, override the `Register` function in `UMassObserverProcessor`: 

```c++
// header file
UPROPERTY()
UScriptStruct* MyObserverType = nullptr;

EMassObservedOperation MyOperation = EMassObservedOperation::MAX;


// cpp file 
UMyMassObserverProcessor::UMyMassObserverProcessor()
{
	ObservedType = FSampleColorFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
	ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
	MyObserverType = FSampleMaterialFragment::StaticStruct();
	MyOperation = EMassObservedOperation::Add;
}

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


<!--FIXMEFUNK - Very WIP. I will share some images of Insights in here soon...-->
<a name="mass-mt"></a>
### 4.10 Multithreading

Out of the box Mass can spread out work to threads in two different ways:
<!--FIXMEFUNK - we really need to figure out which ini this goes in...-->
- Per-Processor threading based on the processor dependency graph by setting the console variable `mass.FullyParallel 1`

- Per-query parallelism spreads the job of one query over multiple threads using a `ParallelFor`. This is available by using `Query.ParallelForEachEntityChunk` in place of `Query.ForEachEntityChunk`.
```c++
MyQuery.ParallelForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	//Loop over every entity in the current chunk and do stuff!
	for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
	{
		// ...
	}
}, FMassEntityQuery::ForceParallelExecution);
```
Notes:
- ParallelForEachEntityChunk will create a dedicated command buffer for each job by default.
- the Cvar `mass.AllowQueryParallelFor` must be enabled.



<a name="mass-cm"></a>
## 5. Common Mass operations
This section is designed to serve as a quick reference for how to perform common operations with Mass. As usual, we are open to ideas on how to organize this stuff!!

As a rule of thumb, most entity mutations (adding/removing components, spawning or removing entities) are generally done by deferring them from inside of processors. 


<!-- You can create your own `FMassExecutionContext` whenever you need one as well! We have one on the `UMSSubsystem` as an example. -->



<!--FIXMEFUNK: When does changing values require deferrment if ever? need more concurrency info for that-->

<a name="mass-cm-spae"></a>
## 5.1 Spawning entities

In this Section we are going to review different methods to spawn entities. First, we review the `Mass Spawner`, which is useful to spawn entities with predefined data. Then, we'll move to more complex spawning methods that enable us to have fine grained control over the spawning.

### 5.1.1 Spawn entities with data predefined in the Editor - Mass Spawner

Mass Spawners (`AMassSpawner`) are useful to spawn entities with static data in the world (predefined CDO and spawning transform).

Mass Spawners require two things to spawn entities:
- An array of entity types: Define which entities to spawn through a [`UMassEntityConfigAsset`](#mass-traits). 
- An array of Spawn Data Generators (`FMassSpawnDataGenerator`): Define where to spawn entities (their starting transform).

In the details panel of a `AMassSpawner` we can find the following:
![MassSpawnerSettings](Images/massspawneractor.jpg)

In the above image, the `MEC_DebugVisualize` Entity Config is used to spawn 25 entities on `BeginPlay` (`bAutoSpawnOnBeginPlay` is set to `true`).

The spawning location of these entities is generated by the `EQS SpawnPoints Generator`, which is a built-in generator that uses the [Environmental Query System](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ArtificialIntelligence/EQS/EQSOverview/) to find locations in the world to spawn. In this example, we are creating a circle of locations around the spawner actor: 

![EQSCircle](Images/eqscircle.jpg)

The result in game on BeginPlay:

![SpawnerCircleResult](Images/spawnercircleresult.jpg)

Mass Spawners are placed in the level and can be queried in runtime to trigger spawns by calling `DoSpawning()` from C++ or Blueprints:

![aa](Images/massspawner-lvl1minions.jpg)

Mass Spawners provide a minimal API to do spawn related operations, following next we provide some of the user-friendly accessible functions from both, blueprints and C++:
- `DoSpawning()`: Performs the spawning of all the agent types of this spawner.
- `DoDespawning()`: Despawns all mass agents spawned by this spawner.
- `ScaleSpawningCount(float Scale)`: Scales the spawning counts. Scale is the number to multiply the all counts of each agent types.
- `GetCount()`: Returns the unscaled count of entities to spawn.
- `GetSpawningCountScale()`: Returns the number to multiply the all counts of each agent types.

**Note:** The Matrix demo uses extensively the Mass Spawner system.


### 5.1.2 Spawn entities with runtime data
In this section we explore more flexible spawn mechanism, in which we are able to spawn entities on demand with runtime data (ie: a passed in location).

These spawning methods can be benefitial when we require to mutate entities on spawn, or when when the spawning data cannot be predefined (ie: the initial transform data for a projectile spawning from a weapon).

<!-- FIXMEVORI: Karl we need more movement in this section, I kind of repurposed it to make more sense! -->

#### 5.1.2.1 Batch Spawning
In C++, you can just call `BatchCreateEntities()` on an instance of a `FMassEntityManager` by passing in a specific archetype with the number you want. This is actually how `AMassSpawner` spawns stuff internally! It calls `BatchSetEntityFragmentsValues()` afterwards to set the initial data on the returned `FEntityCreationContext`.

<!--FIXMEKARL another struct to document (FEntityCreationContext) Weeeeee! It actually might be useful for our mutation merging idea. -->

#### 5.1.2.2 Single Entity Spawning
Spawning a new Entity only requires asking the Mass Entity Subsystem for a new entity. Here are some common ways to create new entities with data.

#### Entity with Fragment data
<!--FIXMEFUNK: Aaaagh!! -->
<!-- FIXMEVORI: Nopers, as I told you don't worry about replicating code and documentation, but let's expose here the best of the bestests practice -->

[Check out this example with `FBuildEntityFromFragmentInstance` from the commands section:](#mass-queries-FBuildEntityFromFragmentInstances)

We currently recommend not calling `UMassEntitySubsystem::BuildEntity` directly unless you are sure don't need observers to trigger for the entity. The shared fragments go in there as well as the third function argument!

#### Entity with Fragment data and Tags

Currently, my best guess is to use `FMassCommandBuildEntity` and then defer however many `Context.Defer().AddTag<FTagType>(EntityReservedEarlier);` you need.

<!--REVIEWMEFUNK: Added stuff in observers-->
#### A note on observers

It is very important to remember that Observers are only triggered explicitely in certain functions out of the box. [Check out the list here.](#mass-o-n) 

<!-- NEW! -->
<a name="mass-cm-dsae"></a>
## 5.2 Destroying entities
- Deferred

The preferred way of destroying entities is to defer it (especially when processing, to stay safe.)
```c++ 
EntityManager->Defer().DestroyEntities(Entities);
EntityManager->Defer().DestroyEntity(Entity);
```
- Directly

`BatchDestroyEntityChunks` is preferred as it calls the observer manager for you. This is only truly safe to call outside of processing on the main thread, like other direct composition changes.
`UMassSpawnerSubsystem::DestroyEntities` calls this as well.

```c++
EntityManager->BatchDestroyEntityChunks(Collection) 

```


<!-- #### Deferred -->

<!-- #### Direct Call -->

<a name="mass-cm-opee"></a>
## 5.3 Operating Entities

In this Section we are going to explore the most relevant tools Mass offers to operate Entities. This covers all the get and set operations and structures to work with them (fragment, archetype, tags...).

**Note:** In cases where we need to operate with Entities outside the current processing context (e.g. avoidance between Entity crowds) it is possible to call all of the regular Mass Subsystem functions or deferred actions on them. This is not ideal for cache coherency but it is nearly unavoidable in gameplay code. 

## 5.2.1 `FMassEntityView`

`FMassEntityView` is a struct that eases all kinds of Entity operations. One can be constructed with a `FMassEntityHandle` and a `FMassEntityManager`. On construction, the `FMassEntityView` caches the Entity's archetype data, which will later reduce repeated work needed to retrieve information about the Entity.

Following next, we expose some of the relevant functions of `FMassEntityView`:

<!--TODO: List of relevant functions interesting for the user:-->
<!-- REVIEWMEFUNK slighty better example... -->
In the following contrived processor example, we check if `NearbyEntity` is an enemy, if it is, we damage it:
```c++
FMassEntityView EntityView(Manager, NearbyEntity.Entity);

//Check if we have a tag
if (EntityView.HasTag<FEnemyTag>())
{
	if(auto DamageOnHitFragment = EntityView.GetFragmentDataPtr<FDamageOnHit>())
	{
	    // Now we defer something to do to the other entity!
 	    FDamageFragment DamageFragment;
 	    DamageFragment.Damage = DamageOnHitFragment.Damage * AttackPower;
        Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(EntityView.GetEntity, DamageFragment);
	}
}
```


<a name="mass-pm"></a>
## 6. Mass Plugins and Modules
This Section overviews the three main Mass plugins and their different modules:

> 6.1 [`MassEntity`](#mass-pm-me)  
> 6.2 [`MassGameplay`](#mass-pm-gp)  
> 6.3 [`MassAI`](#mass-pm-ai)  

<a name="mass-pm-me"></a>
### 6.1 [`MassEntity`](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/)
`MassEntity` is the main plugin that manages everything regarding Entity creation and storage.


<a name="mass-pm-gp"></a>
### 6.2 `MassGameplay `
The `MassGameplay` plugin compiles a number of useful Fragments and Processors that are used in different parts of the Mass framework. It is divided into the following modules:

> 6.2.1 [`MassCommon`](#mass-pm-gp-mc)  
> 6.2.2 [`MassMovement`](#mass-pm-gp-mm)  
> 6.2.3 [`MassRepresentation`](#mass-pm-gp-mr)  
> 6.2.4 [`MassSpawner`](#mass-pm-gp-ms)  
> 6.2.5 [`MassActors`](#mass-pm-gp-ma)  
> 6.2.6 [`MassLOD`](#mass-pm-gp-ml)  
> 6.2.7 [`MassReplication`](#mass-pm-gp-mre)  
> 6.2.8 [`MassSignals`](#mass-pm-gp-msi)  
> 6.2.9 [`MassSmartObjects`](#mass-pm-gp-mso)  

<!-- FIXME: Since there are some modules more interesting than others we will format them in a subsection manner, so we can extend the interesting one easier. -->
<a name="mass-pm-gp-mc"></a>
#### 6.2.1 `MassCommon`
Basic fragments like `FTransformFragment`.

<a name="mass-pm-gp-mm"></a>
#### 6.2.2 `MassMovement`
Features an important `UMassApplyMovementProcessor` processor that moves entities based on their velocity and force.

<a name="mass-pm-gp-mr"></a>
#### 6.2.3 `MassRepresentation`
Processors and fragments for rendering entities in the world. They generally use an ISMC to do so, but can also swap entities out with full Unreal actors at user specified distances.

<a name="mass-pm-gp-ms"></a>
#### 6.2.4 `MassSpawner`
A highly configurable actor type that can spawn specific entities where you want. There are two ways of choosing locations built in, one that uses an Environmental Query System asset and one that uses a ZoneGraph tag-based query. The Mass Spawner actor appears to be intended for things that spawn all at once initially like NPCs,trees etc, rather than dynamically spawned things like projectiles, for example.

<a name="mass-pm-gp-ma"></a>
#### 6.2.5 `MassActors`
A bridge between the general UE5 actor framework and Mass. A type of fragment that turns entities into "Agents" that can exchange data in either direction (or both ways).

<a name="mass-pm-gp-ml"></a>
#### 6.2.6 `MassLOD`
LOD Processors that can manage different kinds of levels of detail, from rendering to ticking at different rates based on fragment settings. They are used in visualization and replication currently as well.

<a name="mass-pm-gp-mre"></a>
#### 6.2.7 `MassReplication`
Replication support for Mass! Other modules override `UMassReplicatorBase` to replicate stuff. Entities are given a separate Network ID that gets passed over the network, rather than the EntityHandle. An example showing this is planned for much later.

<a name="mass-pm-gp-msi"></a>
#### 6.2.8 `MassSignals`
A system that lets entities send named signals to each other.

<a name="mass-pm-gp-mso"></a>
#### 6.2.9 [`MassSmartObjects`](https://docs.unrealengine.com/5.0/en-US/smart-objects-in-unreal-engine/)
Lets entities "claim" SmartObjects to interact with them.

<!-- This section explicitly for AI specific modules-->
<a name="mass-pm-ai"></a>
### 6.3 MassAI
`MassAI` is a plugin that provides AI features for Mass within a series of modules:

> 6.3.1 [`ZoneGraph`](#mass-pm-ai-zg)  
> 6.3.2 [`StateTree`](#mass-pm-ai-st)  
> 6.3.3 ...

This section, like the rest of the document, is still work in progress.

<!-- FIXME: Ideally, this section should be like the previous one. -->
<!-- FIXME: To what extent do we want to cover the AI side of mass. -->
<!-- FIXMEFUNK: I think we should cover a brief overview at the minimum. most of Mass is attached to the AI stuff so we kind of have to at least mention all of it. The Zonegraph cones are a good short example. We should suggest to check out the CitySample at least. -->

<a name="mass-pm-ai-zg"></a>
#### 6.3.1 `ZoneGraph`
<!-- FIXME: Add screenshots and examples. -->
In-level splines and shapes that use config defined lanes to direct zonegraph pathing things around! Think sidewalks, roads etc. This is the main way Mass Crowd members get around.

<a name="mass-pm-ai-st"></a>
#### 6.3.2 [`StateTree`](https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/)
<!-- FIXME: Add screenshots and examples. -->
A new lightweight generic statemachine that can work in conjunction with Mass. One of them is used to give movement targets to the cones in the parade in the sample.




<a name="mass-or"></a>
## 7. Other Resources

### 7.1 Mass
This section compiles very useful Mass resources to complement this documentation.
#### **Epic Games Official resources:**
  - [[Documentation] MassEntity](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/): Overview of Unreal Engine's MassEntity system.
  - [[Documentation] Mass Avoidance](https://docs.unrealengine.com/5.0/en-US/mass-avoidance-in-unreal-engine/): Mass Avoidance is a force-based avoidance system integrated with MassEntity.
  - [[Documentation] Smart Objects](https://docs.unrealengine.com/5.0/en-US/smart-objects-in-unreal-engine/): Smart Objects represent a set of activities in the level that can be used through a reservation system.
  - [[Documentation] StateTree](https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/): Overview of the Mass AI StateTree system.
  - [[Video] State of Unreal : Large Numbers of Entities with Mass](https://youtu.be/f9q8A-9DvPo): Mario Palermo (Global Unreal Engine 5 Lead Evangelist) showcases Mass in detail in a 30-minute video.

<!-- Huge credit to this blog for teaching us how to use spawners! -->
#### **[@quabqi](https://www.zhihu.com/people/quabqi)'s blog posts (Chinese):**
  - [ECS of UE5: MASS framework (1)](https://zhuanlan.zhihu.com/p/441773595): Mass memory hierarchy, entity and archetype introduction.
  - [ECS of UE5: MASS framework (2)](https://zhuanlan.zhihu.com/p/446937133): Mass basic execution.
  - [ECS of UE5: MASS framework (3)](https://zhuanlan.zhihu.com/p/477803528): A deep dive in `MassGameplay`.
  - [MassAI crowd drawing of UE5 CitySample](https://zhuanlan.zhihu.com/p/496165391): How are the pedestrians of the UE5 CitySample handled?
 
### 7.2 General Entity Component Systems (ECS)

  - [Sander's Entity Component System FAQ](https://github.com/SanderMertens/ecs-faq): This FAQ is for anyone interested in ECS & modern, high performance game development.
  - [Data-Oriented Design by Richard Fabian](https://www.dataorienteddesign.com/dodbook/): A book detailing a style/paradigm of programming called "Data-Oriented Design". Entity Component System libraries like Mass make data oriented design easy!
  - [Evolve Your Hierarchy by Mick West](https://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/): An article demonstrating how to use composition over inheritance to represent game entities.



