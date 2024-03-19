# Пример Community Mass
### **Внимание:** Этот проект требует `Git LFS` для правильной работы, скачивание `zip`-архива **не работает**.

#### **Авторы:**
- Karl Mavko - [@Megafunk](https://github.com/Megafunk)
- Alvaro Jover - [@vorixo](https://github.com/vorixo)

Наше **очень долго находящеея в разработке** понимание эксперементальной системы сущностей движка Unreal Engine 5 - плагин Entity Component System (ECS) с небольшим примером. Мы **не относимся** к Epic Games и в эту систему часто вносятся изменения, поэтому информация может быть не совсем точной.

Мы полностью открыты для вклада, если что-то не так или вы считаете, что это можно улучшить, не стесняйтесь [открыть issue](https://github.com/Megafunk/MassSample/issues) или отправить [pull request](https://github.com/Megafunk/MassSample/pulls).

В настоящее время создан для Unreal Engine 5 последней версии из лаунчера Epic Games.
Эта документация будет часто обновляться!

# ⚠ Исправление ошибки 5.2 ⚠

В 5.2 есть ошибка в установке флагов выполнения (Execution Flags) для мира и mass-обработчиков, которая может быть устранена либо в конфигурации Mass, либо изменениями в движке. 
[тут!](https://dev.epicgames.com/community/learning/tutorials/JXMl/unreal-engine-your-first-60-minutes-with-mass#**massprocessorbugin5.2)

#### **Требования:**
- Unreal Engine 5.3 (последняя на момент написания) из [лаунчера Epic Games](https://www.unrealengine.com/en-US/download)
- Система контроля версий `Git`:
  - [Windows](https://gitforwindows.org/)
  - [Linux/Unix & macOS](https://git-scm.com/downloads)
- [Git Large File Storage](https://git-lfs.github.com/)

#### **Инструкция по скачиванию (Windows):**
После установки вышеуказанных требований выполните следующие действия:

1. Щелкните правой кнопкой мыши в том месте, где вы хотите разместить свой проект, и нажмите `Git Bash Here`.

2. В терминале клонируйте проект:
	```bash
	git clone https://github.com/Megafunk/MassSample.git
	```

3. Вытянуть LFS:
	```bash
	git lfs pull
	```
4. После завершения работы LFS закройте терминал.



<!--- Introduce here table of contents -->
<a name="tocs"></a>
## Оглавление
> 1. [Mass](#mass)  
> 2. [Сущность Компонент Система](#ecs)  
> 3. [Проект примера](#sample) 
> 4. [Концепции Mass](#massconcepts)  
> 4.1 [Сущности](#mass-entities)   
> 4.2 [Фрагменты](#mass-fragments)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.2.1 [Общие фрагменты](#mass-fragments-sf)  
> 4.3 [Тэги](#mass-tags)  
> 4.4 [Подсистемы](#mass-subsystems)  
> 4.5 [Модель Архитипов](#mass-arch-mod)   
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.1 [Тэги в модели архитипов](#mass-arch-mod-tags)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.5.2 [Фрагменты в модели архитипов](#mass-arch-mod-fragments)  
> 4.6 [Обработчики](#mass-processors)  
> 4.7 [Запросы](#mass-queries)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.1 [Требования доступа](#mass-queries-ar)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.2 [Требования наличия](#mass-queries-pr)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.3 [Итерационные запросы](#mass-queries-iq)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.7.3 [Изменение сущностей с помощью Defer()](#mass-queries-mq)  
> 4.8 [Признаки](#mass-traits)  
> 4.9 [Наблюдатели](#mass-o)  
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.9.1 [Ограничения наблюдателей](#mass-o-n)                
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;4.9.2 [Наблюдение за несколькими фрагментами/тегами](#mass-o-mft)       
> 4.10 [Многопоточность](#mass-mt)  
> 5. [Общие Mass-операции](#mass-cm)   
> 5.1 [Порождение сущностей](#mass-cm-spae)  
> 5.2 [Уничтожение сущностей](#mass-cm-dsae)  
> 5.3 [Оперирование сущностями](#mass-cm-opee)  
> 6. [Mass плагины и модули](#mass-pm)  
> 6.1 [MassEntity](#mass-pm-me)  
> 6.2 [MassGameplay](#mass-pm-gp)  
> 6.3 [MassAI](#mass-pm-ai)  
> 7. [Другие ресурсы](#mass-or)  


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
Mass - это собственный ECS-фреймворк Unreal Engine! Технически, [Sequencer](https://docs.unrealengine.com/4.26/en-US/AnimatingObjects/Sequencer/Overview/) уже использовал один внутренний модуль, но он не был предназначен для игрового кода. Mass был создан командой искусственного интеллекта Epic Games для облегчения симуляции массовых скоплений людей, но впоследствии стал включать в себя и множество других функций. Он был задействован в демоверсии [Matrix Awakens](https://www.unrealengine.com/en-US/blog/introducing-the-matrix-awakens-an-unreal-engine-5-experience), которую Epic выпустила в 2021 году.

<a name="ecs"></a>
## 2. Сущность Компонент Система - ECS
Mass - это основанная на архетипах система объединения сущностей. Если вы уже знаете, что это такое, то можете переходить к следующему разделу.

В Mass некоторая терминология ECS отличается от общепринятой, чтобы не запутаться в существующем коде Unreal,  приводим обозначения:
| ECS | Mass |
| ----------- | ----------- |
| Entity | Entity |
| Component | Fragment | 
| System | Processor | 

Типичный игровой код Unreal Engine выражается в виде Actor объектов, которые наследуются от родительских классов и изменяют свои данные и функциональность в зависимости от того, чем они ***являются***. 
В ECS сущность состоит только из фрагментов, которыми манипулируют обработчики в зависимости от того, какими компонентами ECS они ***обладают***. 

На самом деле сущность (Entity) - это просто небольшой уникальный идентификатор, который указывает на некоторые фрагменты. Обработчик определяет запрос, который фильтрует только те сущности, которые имеют определенные фрагменты. Например, базовый обработчик "Движение" может запросить сущности, имеющие компонент трансформации и скорости, чтобы добавить скорость к их текущей позиции трансформации. 

Фрагменты хранятся в памяти в виде плотно упакованных массивов других идентичных фрагментов, называемых архетипами. Благодаря этому вышеупомянутый обработчик движений может быть невероятно высокопроизводительным, поскольку он выполняет простую операцию над небольшим количеством данных за один раз. Новая функциональность может быть легко добавлена путем создания новых фрагментов и обработчиков.

Внутренне Mass похож на существующие библиотеки ECS на основе архетипов [Unity DOTS](https://docs.unity3d.com/Packages/com.unity.entities@0.17/manual/index.html) и [FLECS](https://github.com/SanderMertens/flecs). Существует множество других!


<a name="sample"></a>
## 3. Проект примера
В настоящее время пример содержит следующее:

- Минимальный обработчик движений, чтобы показать, как настраивать обработчики.
- Пример использования порождающих Mass-объектов для ZoneGraph и EQS.
- Массовая симуляция толпы конусов, которая движется по уровню, следуя форме ZoneGraph с линиями.
- Пример симуляции снаряда с линейной трассировкой.
- Простая 3d хэш-сетка для сущностей.
- Очень базовая интеграция Mass blueprint.
- Групповой Niagara-рендеринг для сущностей.


<a name="massconcepts"></a>
## 4. Концепции Mass

#### Разделы

> 4.1 [Сущности](#mass-entities)  
> 4.2 [Фрагменты](#mass-fragments)  
> 4.3 [Тэги](#mass-tags)  
> 4.4 [Подсистемы](#mass-subsystems)  
> 4.5 [Модель архитипов](#mass-arch-mod)   
> 4.6 [Обработчики](#mass-processors)  
> 4.7 [Запросы](#mass-queries)  
> 4.8 [Признаки](#mass-traits)  
> 4.9 [Наблюдатели](#mass-o)

<a name="mass-entities"></a>
### 4.1 Сущности
Небольшие уникальные идентификаторы, указывающие на комбинацию [фрагментов](#mass-fragments) и [тегов](#mass-tags) в памяти. Сущности в основном представляют собой простой целочисленный идентификатор. Например, сущность 103 может указывать на один снаряд с данными о трансформации, скорости и уроне.

<!-- TODO: Document the different ways in which we can identify an entity in mass and their purpose? FMassEntityHandle, FMassEntity, FMassEntityView?? -->

<a name="mass-fragments"></a>
### 4.2 Фрагменты
Структуры-данные типа `UStructs`, которыми могут владеть сущности и к которым могут обращаться обработчики. Чтобы создать фрагмент, наследуйте от [`FMassFragment`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassFragment/). 

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FLifeTimeFragment : public FMassFragment
{
	GENERATED_BODY()
	float Time;
};
```

С `FMassFragment` каждая сущность получает свои собственные данные фрагмента, чтобы разделить данные между несколькими сущностями, мы можем использовать [общие фрагменты] (#mass-fragments-sf). 

<a name="mass-fragments-sf"></a>
#### 4.2.1 Общие фрагменты
Общий фрагмент - это тип фрагмента, на который могут указывать несколько сущностей. Он часто используется для конфигурации, общей для группы сущностей, например для настроек LOD или репликации. Чтобы создать общий фрагмент, наследуйте от [`FMassSharedFragment`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassSharedFragment/).

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FVisibilityDistanceSharedFragment : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Distance;
};
```

В приведенном выше примере все сущности, содержащие фрагмент `FVisibilityDistanceSharedFragment`, будут видеть одно и то же значение `Distance`. Если сущность изменит значение `Distance`, остальные сущности с этим фрагментом увидят изменения, поскольку они разделяют его через архетип. Общие фрагменты обычно добавляются из Mass-признаков. 

Убедитесь, что ваши общие фрагменты CRC-хэшируются, иначе вы можете не создать новый экземпляр при вызове `GetOrCreateSharedFragmentByHash`. Вы можете передать свой собственный хэш в `GetOrCreateSharedFragmentByHash`, что может помочь, если вы хотите контролировать, что делает каждый фрагмент уникальным.

Благодаря этому требованию к общим данным, менеджеру Mass-сущностей  нужно хранить только один общий фрагмент для сущностей, которые его используют.

<a name="mass-tags"></a>
### 4.3 Теги
Пустые `UScriptStructs`, которые могут использоваться [обработчиками](#mass-processors) для фильтрации сущностей для обработки на основе их наличия/отсутствия. 
Чтобы создать тег, наследуйте от [`FMassTag`](https://docs.unrealengine.com/5.0/en-US/API/Plugins/MassEntity/FMassTag/).

```c++
USTRUCT()
struct MASSCOMMUNITYSAMPLE_API FProjectileTag : public FMassTag
{
	GENERATED_BODY()
};
```
**Примечание:** Теги никогда не должны содержать свойства членов.

<a name="mass-subsystems"></a>
### 4.4 Подсистемы
Начиная с UE 5.1, Mass расширил свой API для поддержки [`UWorldSubsystems`](https://docs.unrealengine.com/4.27/en-US/API/Runtime/Engine/Subsystems/UWorldSubsystem/) в наших [обработчиках](#mass-processors). Это дает возможность создавать встроенную функциональность для управления сущностями. Сначала наследуйте от `UWorldSubsystem` и определите ее базовый интерфейс вместе с вашими функциями и переменными:

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

Далее мы приводим пример реализации предоставленного выше интерфейса (см. `MassEntityTestTypes.h`):

```c++
void UMyWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Инициализируйте зависимые подсистемы перед вызовом родителя
	Collection.InitializeDependency(UMyOtherSubsystemOne::StaticClass());
	Collection.InitializeDependency(UMyOtherSubsystemTwo::StaticClass());
	Super::Initialize(Collection);

	// Здесь вы можете подключиться к делегатам!
	// ie: OnFireHandle = FExample::OnFireDelegate.AddUObject(this, &UMyWorldSubsystem::OnFire);
}

void UMyWorldSubsystem::Deinitialize()
{
	// Здесь вы можете отключиться от делегатов
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
Приведенный выше код ориентирован на многопоточность, отсюда и токены `UE_MT_X`.

<!-- FIXMEVORI-UE5: Maybe a section exposing the different UE_MT_X tokens? (Get informed about their full scope) -->

Наконец, чтобы сделать эту подсистему мира совместимой с Mass, вы должны определить ее признаки подсистемы, которые сообщают Mass о ее [параллельных возможностях](#mass-mt). В данном случае наша подсистема поддерживает параллельное чтение:

```c++
/**
 * Признаки, описывающие, как данный фрагмент кода может быть использован Mass. 
 * Мы требуем, чтобы автор или пользователь данной подсистемы 
 * определил ее признаки. Для этого добавьте в доступное место следующее. 
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
* это позволит Mass узнать, что он может получить доступ к UMyWorldSubsystem в любом потоке.
*
* Эта информация используется для расчета зависимостей процессоров и запросов, а также для 
* соответствующего распределения вычислений по потокам.
*/
```
Если вы хотите использовать `UWorldSubsystem`, у которой ранее не были определены признаки, и вы не можете явно изменить ее заголовок, вы можете добавить информацию о признаках подсистемы в отдельный заголовочный файл (см. `MassGameplayExternalTraits.h`).

<a name="mass-arch-mod"></a>
### 4.5 Модель архетипа
Как уже говорилось, сущность - это уникальная комбинация фрагментов и тегов. Mass называет каждую из этих комбинаций архетипами. Например, при наличии трех различных комбинаций, используемых нашими сущностями, мы создадим три архетипа:

![MassArchetypeDefinition](Images/arche-entity-type.png)

Структура `FMassArchetypeData` представляет архетип в Mass внутренне. 

<a name="mass-arch-mod-tags"></a>
#### 4.5.1 Теги в модели архетипа
Каждый архетип (`FMassArchetypeData`) содержит набор бит (`TScriptStructTypeBitSet<FMassTag>`), который содержит информацию о наличии тегов, при этом каждый бит в наборе бит представляет, существует ли тег в архетипе или нет.

![MassArchetypeTags](Images/arche-tags.png)

В соответствии с предыдущим примером, *Archetype 0* и *Archetype 2* содержат теги: *TagA*, *TagC* и *TagD*; в то время как *Archetype 1* содержит *TagC* и *TagD*. Таким образом, комбинация *Фрагмент A* и *Фрагмент B* должна быть разделена на два разных архетипа.

<a name="mass-arch-mod-fragments"></a>
#### 4.5.2 Фрагменты в модели архетипа
В то же время, каждый архетип содержит массив блоков (`FMassArchetypeChunk`) с данными о фрагментах.

Каждый блок содержит подмножество сущностей, включенных в наш архетип, где данные организованы в псевдо-[структуры массивов](https://en.wikipedia.org/wiki/AoS_and_SoA#Structure_of_arrays):

![MassArchetypeChunks](Images/arche-chunks.png)

На следующем рисунке представлены архетипы из приведенного выше примера в памяти:

![MassArchetypeMemory](Images/arche-mem.png)

Благодаря тому, что эта псевдо-[структура массивов](https://en.wikipedia.org/wiki/AoS_and_SoA#Structure_of_arrays) схема данных разделена на множество блоков, мы можем поместить в кэш процессора большое количество целых объектов. 

Это происходит благодаря разделению блоков, поскольку без него в кэш не поместилось бы так много целых объектов, как показано на следующей диаграмме:

![MassArchetypeCache](Images/arche-cache-friendly.png)

В приведенном выше примере архетип Chunked получает в кэш целые сущности, а Linear архетип получает в кэш все *A фрагменты*, но не может вместить каждый фрагмент сущности.

Линейный подход был бы быстрым, если бы мы обращались к *A-фрагменту* только при итерации сущностей, однако это почти никогда не происходит. Обычно при итерации сущностей мы обращаемся к нескольким фрагментам, поэтому удобно иметь их все в кэше, что и обеспечивает разбиение на блоки (chunk partitioning).

Размер блока (`UE::Mass::ChunkSize`) был удобно установлен на основе размеров кэша следующего поколения (128 байт на строку и 1024 строки кэша). Это означает, что архетипы с большим количеством битов данных о фрагментах будут содержать меньше сущностей в одном блоке.

**Примечание:** Следует отметить, что промах по кэшу будет возникать каждый раз, когда мы захотим получить доступ к фрагменту, которого нет в кэше для данной сущности.

<a name="mass-processors"></a>
### 4.6 Обработчики
Обработчики объединяют множество пользовательских [запросов](#mass-queries) с функциями, которые вычисляют сущности.

Классы Unreal, производные от UMassProcessor, автоматически регистрируются в Mass и по умолчанию добавляются в фазу обработки `EMassProcessingPhase::PrePhsysics`. Каждая `EMassProcessingPhase` связана с `ETickingGroup`, что означает, что по умолчанию обработчики отмечают каждый кадр в данной фазе обработки.
Они также могут быть созданы и зарегистрированы в `UMassSimulationSubsystem`, но чаще всего создается новый тип. 
Пользователи могут настроить, к какой фазе обработки относится их обработчик, изменив переменную `ProcessingPhase`, включенную в `UMassProcessor`: 

| `EMassProcessingPhase` | Related `ETickingGroup` | Description |
| ----------- | ----------- | ----------- |
| `PrePhysics` | `TG_PrePhysics` | Выполняется перед началом моделирования физики. |
| `StartPhysics` | `TG_StartPhysics` | Специальная группа тиков, запускающая симуляцию физики. | 
| `DuringPhysics` | `TG_DuringPhysics` | Выполняется параллельно с работой симуляции физики. | 
| `EndPhysics` | `TG_EndPhysics` | Специальная группа тиков, завершающая симуляцию физики. | 
| `PostPhysics` | `TG_PostPhysics` | Выполняется после моделирования жесткого тела и ткани. | 
| `FrameEnd` | `TG_LastDemotable` | Уловитель для всего, что перемещается в конец. | 


В своем конструкторе обработчики могут определять правила для порядка выполнения, фазы обработки и типа игровых клиентов, на которых они выполняются:
```c++
UMyProcessor::UMyProcessor()
{
	// Этот обработчик зарегистрирован массово, просто существуя! Это поведение по умолчанию для всех обработчиков.
	bAutoRegisterWithProcessingPhases = true;
	// Явное задание фазы обработки
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	// Использование встроенной группы обработчиков движения
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	// Вы также можете определить другие обработчики, которые должны быть запущены до или после этого обработчика
	ExecutionOrder.ExecuteAfter.Add(TEXT("MSMovementProcessor"));
	// Это выполняется только на клиентах и в автономном режиме
	ExecutionFlags = (int32)(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	// Этот обработчик не должен быть многопоточным
	bRequiresGameThreadExecution = true;
}
```

При инициализации Mass создает граф зависимостей обработчиков, используя их правила выполнения, чтобы они выполнялись по порядку (т.е. в примере выше мы убеждаемся, что перемещаем сущности с помощью `MSMovementProcessor` перед вызовом `Execute` в `UMyProcessor`).

Переменная `ExecutionFlags` указывает, на каком уровне должен выполняться данный обработчик: на `Standalone`, `Server` или `Client`.

По умолчанию [все обработчики являются многопоточными](#mass-mt), однако при необходимости их можно настроить на однопоточный запуск, установив `bRequiresGameThreadExecution` в `true`.

**Примечание:** Mass поставляется с рядом обработчиков, которые предназначены для наследования и расширения с помощью пользовательской логики. Например: обработчики рендеринга и LOD. 

<a name="mass-queries"></a>
### 4.7 Запросы
Запросы (`FMassEntityQuery`) фильтруют и итерируют сущности, задавая ряд правил, основанных на наличии фрагментов и тегов.

Обработчики могут определять несколько `FMassEntityQuery` и должны переопределять `ConfigureQueries` для добавления правил к различным запросам, определенным в заголовке обработчика:

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

Чтобы выполнять запросы на обработчике, мы должны зарегистрировать их, вызвав `RegisterWithProcessor`, передав обработчик в качестве параметра. `FMassEntityQuery` также предлагает конструктор параметров, вызывающий `RegisterWithProcessor`, который используется в некоторых обработчиках из различных модулей Mass (например, `UDebugVisLocationProcessor`).

`ProcessorRequirements` - это специальная часть запроса `UMassProcessor`, которая содержит все `UWorldSubsystem`, к которым обращаются в функции `Execute` за пределами области действия запросов. В примере выше `UMassDebuggerSubsystem` получает доступ в области видимости `MyQuery` (`MyQuery.AddSubsystemRequirement`) и в области видимости функции `Execution` (`ProcessorRequirements.AddSubsystemRequirement`).

Запросы выполняются путем вызова функции-члена `ForEachEntityChunk` с лямбдой, передавая связанные с ней `FMassEntityManager` и `FMassExecutionContext`. 

Обработчики выполняют запросы в своей функции `Execute`:

```c++
void UMyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Обратите внимание, что это лямбда! Если вам нужны дополнительные данные, их можно передать в операторе []
	MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
	{
		// Перебираем все сущности в текущем блоке и делаем что-нибудь!
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// ...
		}
	});
}
```
Имейте в виду, что индекс, который мы используем для итерации сущностей, в данном случае `EntityIndex`, не идентифицирует однозначно ваши сущности во времени, поскольку расположение чанков может меняться, и сущность, имеющая индекс в этом кадре, может оказаться в другом чанке с другим индексом в следующем кадре.

**Примечание:** Запросы также могут создаваться и итерироваться вне обработчиков.

<a name="mass-queries-ar"></a>
#### 4.7.1 Требования к доступу

Запросы могут определять требования доступа для чтения/записи для фрагментов и подсистем:

| `EMassFragmentAccess` | Description |
| ----------- | ----------- |
| `None` | Привязка не требуется. |
| `ReadOnly` | Мы хотим прочитать данные для фрагмента/подсистемы. | 
| `ReadWrite` | Мы хотим читать и записывать данные для фрагмента/подсистемы. | 

В `FMassFragment` используется `AddRequirement` для добавления требований доступа и наличия к нашим фрагментам. В то время как `FMassSharedFragment` использует `AddSharedRequirement`. Наконец, `UWorldSubsystem` использует `AddSubsystemRequirement`. 

Вот несколько основных примеров, в которых мы добавляем правила доступа в два фрагмента из `FMassEntityQuery MyQuery`:

```c++	
void UMyProcessor::ConfigureQueries()
{
	// Сущности должны иметь фрагмент FTransformFragment, и мы читаем и записываем его (EMassFragmentAccess::ReadWrite)
	MyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
		
	// Сущности должны иметь FMassForceFragment, а мы только читаем его (EMassFragmentAccess::ReadOnly)
	MyQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);

	// Сущности должны иметь общий фрагмент FClockSharedFragment, который можно читать и записывать
	MyQuery.AddSharedRequirement<FClockSharedFragment>(EMassFragmentAccess::ReadWrite);

	// Сущности должны иметь подсистему UMassDebuggerSubsystem, которая может быть прочитана и записана
	MyQuery.AddSubsystemRequirement<UMassDebuggerSubsystem>(EMassFragmentAccess::ReadWrite);

	// Регистрация запроса в UMyProcessor
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachEntityChunk` может использовать следующие функции для доступа к данным `ReadOnly` или `ReadWrite` в соответствии с требованиями к доступу:

| `EMassFragmentAccess` | Type | Function |Description |
| ----------- | ----------- | ----------- | ----------- |
| `ReadOnly` | Fragment | `GetFragmentView` | Возвращает только для чтения `TConstArrayView`, содержащий данные нашего `ReadOnly` фрагмента.|
| `ReadWrite` | Fragment | `GetMutableFragmentView` | Возвращает доступный для записи `TArrayView`, содержащий данные нашего фрагмента `ReadWrite`.| 
| `ReadOnly` | Shared Fragment | `GetConstSharedFragment` | Возвращает константную ссылку на наш общий фрагмент, предназначенный только для чтения.|
| `ReadWrite` | Shared Fragment | `GetMutableSharedFragment` | Возвращает ссылку на наш общий фрагмент с возможностью записи.| 
| `ReadOnly` | Subsystem | `GetSubsystemChecked` | Возвращает константную ссылку на нашу мировую подсистему только для чтения.|
| `ReadWrite` | Subsystem | `GetMutableSubsystemChecked` | Возвращает ссылку на нашу записываемую общую подсистему мира. |

Ниже приведен пример:

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

**Примечание:** К тегам не предъявляются требования доступа, поскольку они не содержат данных.

<a name="mass-queries-pr"></a>
#### 4.7.2 Требования присутствия
Запросы могут определять требования присутствия для фрагментов и меток:

| `EMassFragmentPresence` | Описание |
| ----------- |-----------------------------------------------------------------------------------|
| All | Все требуемые фрагменты/теги должны присутствовать. Требование присутствия по умолчанию. |
| Any | Хотя бы один из фрагментов/тегов, помеченных как any, должен присутствовать.                    | 
| None | Ни один из требуемых фрагментов/тегов не может присутствовать.                               | 
| Optional | Если фрагмент / тег присутствует, мы будем его использовать, но он не обязательно должен присутствовать.      | 

##### 4.7.2.1 Требования присутствия в тегах
Чтобы добавить правила присутствия в теги, используйте `AddTagRequirement`.   
```c++
void UMyProcessor::ConfigureQueries()
{
	// Сущности рассматриваются для итерации без необходимости содержать указанный тег
	MyQuery.AddTagRequirement<FOptionalTag>(EMassFragmentPresence::Optional);
	// Сущности должны как минимум иметь тег FHorseTag или FSheepTag
	MyQuery.AddTagRequirement<FHorseTag>(EMassFragmentPresence::Any);
	MyQuery.AddTagRequirement<FSheepTag>(EMassFragmentPresence::Any);
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachChunk` может использовать `DoesArchetypeHaveTag`, чтобы определить, содержит ли текущий архетип тег:

```c++
MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	if(Context.DoesArchetypeHaveTag<FOptionalTag>())
	{
		// I do have the FOptionalTag tag!!
	}

	// То же самое с тегами, отмеченными символом Any
	if(Context.DoesArchetypeHaveTag<FHorseTag>())
	{
		// У меня есть тег FHorseTag!!!
	}
	if(Context.DoesArchetypeHaveTag<FSheepTag>())
	{
		// У меня есть тег FSheepTag!!!
	}
});
```

##### 4.7.2.2 Требования наличия во фрагментах
Фрагменты и общие фрагменты могут определять правила наличия в дополнительном параметре `EMassFragmentPresence` через `AddRequirement` и `AddSharedRequirement`, соответственно.

```c++
void UMyProcessor::ConfigureQueries()
{
	// Сущности считаются для итерации без необходимости содержать указанный фрагмент
	MyQuery.AddRequirement<FMyOptionalFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	// Сущности должны как минимум иметь фрагмент FHorseFragment или FSheepFragment
	MyQuery.AddRequirement<FHorseFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	MyQuery.AddRequirement<FSheepFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Any);
	MyQuery.RegisterWithProcessor(*this);
}
```

`ForEachChunk` может использовать длину `TArrayView` фрагмента `Optional`/`Any`, чтобы определить, содержит ли текущий блок данный фрагмент, прежде чем обращаться к нему:

```c++
MyQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
{
	const auto OptionalFragmentList = Context.GetMutableFragmentView<FMyOptionalFragment>();
	const auto HorseFragmentList = Context.GetMutableFragmentView<FHorseFragment>();	
	const auto SheepFragmentList = Context.GetMutableFragmentView<FSheepFragment>();
	for (int32 i = 0; i < Context.GetNumEntities(); ++i)
	{
		// Массив необязательных фрагментов присутствует в нашем текущем блоке, если OptionalFragmentList не пуст
		if(OptionalFragmentList.Num() > 0)
		{
			// Теперь, когда мы знаем, что это безопасно, мы можем вычислить
			OptionalFragmentList[i].DoOptionalStuff();
		}

		// То же самое с фрагментами, помеченными Any
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
#### 4.7.3 Изменение сущностей с помощью `Defer()`.
                                                        
Внутри `ForEachEntityChunk` у нас есть доступ к текущему контексту выполнения. `FMassExecutionContext` позволяет нам получать данные о сущностях и изменять их состав. Следующий код добавляет тег `FDead` к любой сущности, у которой есть фрагмент здоровья с переменной `Health` меньше или равной 0, в то же время, как мы определяем в `ConfigureQueries`, после добавления тега `FDead` сущность не будет рассматриваться для итерации (`EMassFragmentPresence::None`):

```c++
void UDeathProcessor::ConfigureQueries()
{
	// Все сущности, обрабатываемые в этом запросе, должны иметь фрагмент FHealthFragment
	DeclareDeathQuery.AddRequirement<FHealthFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::All);
	// Сущности, обрабатываемые этим запросом, не должны иметь тег FDead, так как этот запрос добавляет тег FDead
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
				// Добавление метки к этой сущности, когда отложенные команды будут удалены
				FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);
				Context.Defer().AddTag<FDead>(EntityHandle);
			}
		}
	});
}
```

Для того чтобы отложить изменения сущностей, нам необходимо получить хэндл (`FMassEntityHandle`) сущностей, которые мы хотим модифицировать. В `FMassExecutionContext` хранится массив со всеми хэндлами сущностей. Мы можем получить к нему доступ с помощью двух различных методов:

| Множество | Код |
| ----------- | ----------- |
| Единичное число | `FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);` |
Множественное число | `auto EntityHandleArray = Context.GetEntities();` | | 

В следующих подразделах будут использоваться ключевые слова `EntityHandle` и `EntityHandleArray` при обработке операций с единственным или множественным числом, соответственно.


##### 4.7.3.1 Основные операции изменения
Следующие листинги определяют базовые мутации, которые вы можете отложить:

Отсрочка обычно выполняется из `FMassExecutionContext` обработчика с помощью `.Defer()`, но их можно выполнить и вне обработки с помощью вызова `EntityManager->Defer()`. 

Фрагменты:
```c++
Context.Defer().AddFragment<FMyFragment>(EntityHandle);
Context.Defer().RemoveFragment<FMyFragment>(EntityHandle);
```

Теги:
```c++
Context.Defer().AddTag<FMyTag>(EntityHandle);
Context.Defer().RemoveTag<FMyTag>(EntityHandle);
Context.Defer().SwapTags<FOldTag, FNewTag>(EntityHandle);
```
 
Уничтожение сущностей:
```c++
Context.Defer().DestroyEntity(EntityHandle);
Context.Defer().DestroyEntities(EntityHandleArray);
```
Все это удобные обертки для внутренних отложенных команд, основанных на шаблонах.

##### 4.7.3.2 Расширенные операции изменения сущностей
Существует набор команд `FCommandBufferEntryBase`, которые могут быть использованы для отсрочки некоторых более полезных изменений сущностей. В следующих подразделах представлен обзор. 

###### 4.7.3.2.1 `FMassCommandAddFragmentInstanceList`.
Откладывает добавление новых данных фрагмента к существующей сущности. 

В примере ниже мы меняем фрагмент `FHitResultFragment` с данными HitResult и фрагмент `FSampleColorFragment` с новым цветом и добавляем (или устанавливаем, если уже есть) их к существующей сущности.

```c++
FHitResultFragment HitResultFragment;
FSampleColorFragment ColorFragment = FSampleColorFragment(Color);

// Устанавливает данные фрагмента на существующую сущность
EntityManager->Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, ColorFragment, HitResultFragment);

// Она может добавлять и отдельные экземпляры фрагментов, а также безопасно устанавливать данные на существующих фрагментах
EntityManager->Defer().PushCommand<FMassCommandAddFragmentInstances>(Entity, SomeOtherFragment);
```

<a name="mass-queries-FBuildEntityFromFragmentInstances"></a>
###### 4.7.3.2.2 `FMassCommandBuildEntity`
Откладывает создание сущности и добавляет к ней список фрагментов с данными.

```c++
FTransformFragment MyTransformFragment;
MyTransformFragment.SetTransform(FTransform::Identity);

// Мы создаем новую сущность и добавляем в нее данные одной командой!
EntityManager->Defer().PushCommand<FMassCommandBuildEntity>(ReserverdEntity, MyTransformFragment, SomeOtherFragment);
));
```

###### 4.7.3.2.3 `FMassCommandBuildEntityWithSharedFragments` 
Аналогичен `FMassCommandBuildEntity`, но принимает структуру `FMassArchetypeSharedFragmentValues`, чтобы установить значения общих фрагментов и для сущности. Это требует некоторой дополнительной работы по поиску или созданию общего фрагмента.
```c++
FMassArchetypeSharedFragmentValues SharedFragmentValues;
// Это то, что признаки используют для создания общей информации о фрагментах.
FConstSharedStruct& SharedFragment = EntityManager->GetOrCreateConstSharedFragment(MySharedFragment);
SharedFragmentValues.AddConstSharedFragment(SharedFragment);

// Здесь требуется MoveTemp...
EntityManager->Defer().PushCommand<FMassCommandBuildEntityWithSharedFragments>(EntityHandle, MoveTemp(SharedFragmentValues), TransformFragment, AnotherFragmentEtc);
```

<!-- FIXMEVORI: For consistency, lets add as a title the name of the command, however in this one I'm not sure which ones we should include -->
##### 4.7.3.2.4 `FMassDeferredSetCommand`
Откладывает выполнение лямбды `TFunction`, переданной в качестве параметра. Она полезна для выполнения операций, связанных с Mass, которые не охватываются другими командами. Это разумный способ обработки изменений Актора, так как [они обычно должны происходить в главном потоке](https://vkguide.dev/docs/extra-chapter/multithreading/#ways-of-using-multithreading-in-game-engines).

```c++
EntityManager->Defer().PushCommand<FMassDeferredSetCommand>(
   [&](FMassEntityManager& Manager)
  {
      	// Это запускается, когда отложенные команды смываются
      	MyActor.DoGameThreadWork();
      	// Здесь также могут происходить обычные вызовы мессенджера. Например:
  	EntityManager.BuildEntity(ReservedEntity, InstanceStructs, EntityTemplate.GetSharedFragmentValues());
  });
```

**Примечание:** Лямбда `TFunction` имеет FMassEntityManager& в качестве параметра функции, который необходимо включать в каждую лямбду, использующую эту команду.

<!-- FIXMEVORI: What is this? maybe we need a code example, since the example above doesn't cover it -->
`FMassDeferredCreateCommand`, `FMassDeferredSetCommand` и другие аналогичные типы имеют шаблоны для установки определенного `EMassCommandOperationType`.

Они предназначены для организации отложенных команд в различные типы операций. Например: мы хотим создавать сущности до того, как изменим на них фрагменты!
Вот они, и что они делают по порядку, когда команды смываются:

| Операция | |
|-------------------|------------------------------------------------|
| Создать | Создание новых сущностей.                         |
| Добавить | Добавление фрагментов/тегов |
| Удалить | Удаление фрагментов/тегов |
| ChangeComposition | Добавление и удаление тегов/фрагментов.            |
| Set | Изменение данных фрагмента (также добавление фрагментов) |
| None | Значение по умолчанию, всегда выполняется последним.           |


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
### 4.8 Признаки
Признаки - это объекты, определенные в C++, которые объявляют набор фрагментов, тегов и данных для создания новых сущностей на основе данных. 

Чтобы начать использовать признаки, создайте `DataAsset`, который наследуется от 
`UMassEntityConfigAsset` и добавьте к нему новые признаки. Каждый признак может быть расширен для установки свойств, если они у него есть. 

Кроме того, можно наследовать фрагменты от другого `UMassEntityConfigAsset`, задав его в поле `Parent`.

![MassEntityConfigAsset](Images/massentityconfigasset.jpg)

Среди множества встроенных признаков, предлагаемых Mass, мы можем найти признак `Assorted Fragments`, который содержит массив `FInstancedStruct`, что позволяет добавлять фрагменты в этот признак из редактора без необходимости создания нового признака в C++. 

![AssortedFragments](Images/assortedfragments.jpg)


<!-- FIXME: Please elaborate -->
<!-- REVIEWMEFUNK kind of hard to talk about it too much here with the other section existing -->
Признаки часто используются для добавления общих фрагментов в виде настроек. Например, наши признаки визуализации экономят память, совместно используя меш, который они отображают, параметры и т. д. Конфиги с одинаковыми настройками будут использовать один и тот же общий фрагмент.


#### 4.8.1 Создание признака
Признаки создаются путем наследования `UMassEntityTraitBase` и переопределения `BuildTemplate`. Вот очень простой пример:

```c++
UCLASS(meta = (DisplayName = "Debug Printing"))
class MASSCOMMUNITYSAMPLE_API UMSDebugTagTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override
	{
		// Добавление тега
		BuildContext.AddTag<FMassSampleDebuggableTag>();
		
		// Добавление фрагмента
		BuildContext.AddFragment<FTransformFragment>();

		// _GetRef позволяет нам мутировать фрагмент
		BuildContext.AddFragment_GetRef<FSampleColorFragment>().Color = UserSetColor;
	};

	// Редактируется в списке свойств редактора для данного актива
	UPROPERTY(EditAnywhere)
	FColor UserSetColor;
};
```
**Примечание:** Мы рекомендуем посмотреть на множество существующих признаков в этом примере и модулях mass для лучшего понимания. По большей части это довольно простые UObjects, которые иногда содержат дополнительный код, чтобы убедиться, что все фрагменты действительны и установлены правильно. 


 <!--REVIEWMEFUNK moved it up to this section "-->
##### Общие фрагменты

 Вот пример частичного `BuildTemplate` для добавления общей структуры, которая может выполнять некоторую дополнительную работу, чтобы проверить, существует ли уже общий фрагмент, идентичный новому:
```c++
	// Создаем фактическую структуру фрагмента и настраиваем данные для нее по своему усмотрению 
	FMySharedSettings MyFragment;
	MyFragment.MyValue = UserSetValue;

	// Получаем хэш FConstStructView указанного фрагмента и сохраняем его
	uint32 MySharedFragmentHash = UE::StructUtils::GetStructCrc32(FConstStructView::Make(MyFragment));
	
	// Поиск в подсистеме Mass Entity идентичной структуры с хэшем. Если таковых нет, создайте новую с заданным фрагментом.
	FSharedStruct MySharedFragment = 
		EntityManager.GetOrCreateSharedFragment<FMySharedSettings>(MySharedFragmentHash, MyFragment);

	// И наконец, добавляем общий фрагмент в BuildContext!
	BuildContext.AddSharedFragment(MySharedFragment);
```


#### 4.8.2 Проверка признаков
Признаки могут переопределять `ValidateTemplate` для предоставления пользовательского кода проверки для признака. Нативные признаки используют эту функцию для регистрации ошибок и/или изменения `BuildContext`, если это необходимо. Эта функция вызывается после `BuildTemplate` и вызывается для всех признаков текущего шаблона.

В следующем фрагменте мы проверяем, является ли поле признака `nullptr`, и регистрируем ошибку:
```c++
void UMSNiagaraRepresentationTrait::ValidateTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	// Если наша общая система niagara равна null, покажите ошибку!
	if (!SharedNiagaraSystem)
	{
		UE_VLOG(&World, LogMass, Error, TEXT("SharedNiagaraSystem is null!"));
		return;
	}
}
```

<a name="mass-o"></a>
### 4.9 Наблюдатели
Обработчик `UMassObserverProcessor` - это тип обработчика, который работает с сущностями, только что выполнившими операцию `EMassObservedOperation` над наблюдаемым типом фрагмента/тега:

| `EMassObservedOperation` | Description |
| ----------- | ----------- |
| Add | Наблюдаемый фрагмент/метка был добавлен к сущности. |
| Remove | Наблюдаемый фрагмент/метка был удален из сущности. | 

Наблюдатели запускаются не каждый кадр, а каждый раз, когда пакет сущностей изменяется таким образом, что удовлетворяет требованиям наблюдателя.

Например, этот наблюдатель изменяет цвет сущностей, к которым только что был добавлен `FColorFragment`:

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
			// Когда добавляется цвет, сделайте его случайным!
			Colors[EntityIndex].Color = FColor::MakeRandomColor();
		}
	});
}
```
<!-- FIXMEFUNK: What happened with this section? :( -->
<!-- REVIEWMEVORI: Lots of source changes, since it's still not really clear when observers fire I felt the need to get extra specific here... 
It's getting to the point where the only things that don't trigger them would be easier to list out. I really think Epic should mark the API calls that don't do this with _Internal or something -->
<a name="mass-o-n"></a>
#### 4.9.1 Вызовы наблюдателей менеджера сущностей
На момент написания статьи наблюдатели вызываются менеджером Mass только непосредственно во время определенных действий с сущностями. В основном это связано с некоторыми специфическими функциями, изменяющими отдельные сущности, такими как `addfragmenttoentity`
<!-- FIXMEVORI: Maybe this isn't the case because we are not following the recommended practices!! Should ensure not skipping the appropriate exec path-->
<!--  REVIEWMEFUNK starting to feel pointless but I still think it's a good thing to know-->
- Изменения сущности в менеджере сущностей:
  - `FMassEntityManager::BatchBuildEntities`
  - `FMassEntityManager::BatchCreateEntities`
  - `FMassEntityManager::BatchDestroyEntityChunks` 
  - `FMassEntityManager::AddCompositionToEntity_GetDelta`
  - `FMassEntityManager::RemoveCompositionFromEntity`
  - `FMassEntityManager::BatchChangeTagsForEntities`
  - `FMassEntityManager::BatchChangeFragmentCompositionForEntities`
  - `FMassEntityManager::BatchAddFragmentInstancesForEntities`
- Все [отложенные команды](#mass-queries-mq), которые изменяют сущность, должны вызывать одну из вышеперечисленных. 

Это охватывает обработчики и спавнеры, но не изменения одиночных сущностей из C++.

К счастью, [недавний коммит](https://github.com/EpicGames/UnrealEngine/commit/2b883dec5f6c821648f2d6005ac06e704099dbd9
) на ue5-main исправил эту проблему.

Если вам нужно, попросите менеджер наблюдателей проверить изменения, для этого достаточно вызвать `OnCompositionChanged()` с дельтой новых добавленных или удаленных компонентов.
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
#### 4.9.2 Наблюдение за несколькими фрагментами/тегами
Наблюдатели также могут быть использованы для наблюдения за несколькими операциями и/или типами. Для этого переопределите функцию `Register` в `UMassObserverProcessor`: 

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
Как отмечалось выше, можно повторно использовать одну и ту же операцию `EMassObservedOperation` для нескольких типов наблюдений, и наоборот.


<!--FIXMEFUNK - Very WIP. I will share some images of Insights in here soon...-->
<a name="mass-mt"></a>
### 4.10 Многопоточность

Из коробки Mass может распределять работу по потокам двумя разными способами:
<!--FIXMEFUNK - we really need to figure out which ini this goes in...-->
- Потоковая обработка (по потоку на обработчик) на основе графа зависимостей обработчиков путем установки консольной переменной `mass.FullyParallel 1`.

- Параллелизация запросов для вызовов, которые распределяют работу одного запроса по нескольким потокам, с помощью аргумента команды `ParallelMassQueries=1` для данного процесса Unreal. В настоящее время этот аргумент не используется ни в модулях Mass, ни в примерах, и при отсрочке команд от него несколько раз за кадр он, похоже, не работает.


<a name="mass-cm"></a>
## 5. Общие операции с Mass
Этот раздел предназначен для быстрого ознакомления с тем, как выполнять общие операции с Mass. Как обычно, мы открыты для идей по организации этого материала!!!

Как правило, большинство изменений сущностей (добавление/удаление компонентов, порождение или удаление сущностей) обычно выполняются путем отложения их внутри обработчиков. 


<!-- You can create your own `FMassExecutionContext` whenever you need one as well! We have one on the `UMSSubsystem` as an example. -->



<!--FIXMEFUNK: When does changing values require deferrment if ever? need more concurrency info for that-->

<a name="mass-cm-spae"></a>
## 5.1 Порождение сущностей

В этом разделе мы рассмотрим различные методы порождения сущностей. Сначала мы рассмотрим `Mass Spawner`, который полезен для порождения сущностей с заранее определенными данными. Затем мы перейдем к более сложным методам порождения, которые позволяют нам получить тонкий контроль над порождением.

### 5.1.1 Порождение сущностей с данными, предопределенными в редакторе - Mass Spawner

Массовые породители (`AMassSpawner`) полезны для порождения сущностей со статическими данными в мире (предопределенные CDO и трансформация порождения).

Массовые породители требуют двух вещей для порождения сущностей:
- Массив типов сущностей: Определение типов сущностей для спавна через [`UMassEntityConfigAsset`](#mass-traits). 
- Массив генераторов данных спавна (`FMassSpawnDataGenerator`): Определяют место порождения сущностей (их начальное преобразование).

В панели деталей `AMassSpawner` мы можем найти следующее:
![MassSpawnerSettings](Images/massspawneractor.jpg)

На изображении выше конфигурация сущностей `MEC_DebugVisualize` используется для порождения 25 сущностей при `BeginPlay` (`bAutoSpawnOnBeginPlay` установлено в `true`).

Место порождения этих сущностей генерируется `EQS SpawnPoints Generator`, который является встроенным генератором, использующим [Environmental Query System](https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ArtificialIntelligence/EQS/EQSOverview/) для поиска мест в мире для порождения. В этом примере мы создаем круг вокруг порождающего агента: 

![EQSCircle](Images/eqscircle.jpg)

Результат в игре на BeginPlay:

![SpawnerCircleResult](Images/spawnercircleresult.jpg)

Массовые породители размещаются на уровне и могут быть запрошены во время выполнения, чтобы вызвать порождение, вызвав `DoSpawning()` из C++ или Blueprints:

![aa](Images/massspawner-lvl1minions.jpg)

Массовые спавнеры предоставляют минимальный API для выполнения операций, связанных с порождением, и далее мы приводим некоторые из удобных функций, доступных как из блюпринтов, так и из C++:
- `DoSpawning()`: Выполняет порождение всех типов агентов данного породителя.
- `DoDespawning()`: Отключает всех массовых агентов, порожденных этим породителем.
- `ScaleSpawningCount(float Scale)`: Масштабирует количество порождений. Scale - это число, на которое нужно умножить все количество агентов каждого типа.
- `GetCount()`: Возвращает немасштабированное количество порождений.
- `GetSpawningCountScale()`: Возвращает число, на которое нужно умножить все количество агентов каждого типа.

**Примечание:** В демо-версии Matrix широко используется система массового породителя.


### 5.1.2 Порождение сущностей с помощью данных времени выполнения
В этом разделе мы рассмотрим более гибкий механизм порождения сущностей, в котором мы можем порождать сущности по требованию, используя данные времени выполнения (т.е. переданное местоположение).

Эти методы порождения сущностей могут быть полезны, когда нам нужно изменить сущность при порождении, или когда данные порождения не могут быть предопределены (например, данные начальной трансформации для снаряда, порождаемого оружием).

<!-- FIXMEVORI: Karl we need more movement in this section, I kind of repurposed it to make more sense! -->

#### 5.1.2.1 Пакетное порождение
В C++ вы можете просто вызвать `BatchCreateEntities()` на экземпляре `FMassEntityManager`, передав определенный архетип с нужным вам количеством. На самом деле, именно так `AMassSpawner` порождает вещи внутри себя! После этого он вызывает `BatchSetEntityFragmentsValues()`, чтобы установить начальные данные для возвращенного `FEntityCreationContext`.

<!--FIXMEKARL another struct to document (FEntityCreationContext) Weeeeee! It actually might be useful for our mutation merging idea. -->

#### 5.1.2.2 Порождение одной сущности
Для порождения новой сущности достаточно запросить новую сущность у подсистемы массовых сущностей.Вот несколько распространенных способов создания новых сущностей с помощью данных.

#### Сущность с данными фрагмента
<!--FIXMEFUNK: Aaaagh!! -->
<!-- FIXMEVORI: Nopers, as I told you don't worry about replicating code and documentation, but let's expose here the best of the bestests practice -->

[Посмотрите этот пример с `FBuildEntityFromFragmentInstance` из раздела команд:](#mass-queries-FBuildEntityFromFragmentInstances)

В настоящее время мы не рекомендуем вызывать `UMassEntitySubsystem::BuildEntity` напрямую, если вы не уверены, что вам не нужны наблюдатели для срабатывания сущности.

Общие фрагменты попадают туда же, как и третий аргумент функции!
#### Сущность с данными фрагментов и тегами

На данный момент я считаю, что лучше всего использовать `FMassCommandBuildEntity`, а затем отложить столько `Context.Defer().AddTag<FTagType>(EntityReservedEarlier);`, сколько вам нужно.

<!--REVIEWMEFUNK: Added stuff in observers-->
#### Замечание о наблюдателях

Очень важно помнить, что наблюдатели могут быть вызваны явно только в определенных функциях из коробки.

[Ознакомьтесь со списком здесь] (#mass-o-n). 

<!-- NEW! -->
<a name="mass-cm-dsae"></a>
## 5.2 Уничтожение сущностей
- Отложенное

Предпочтительным способом уничтожения сущностей является отложенное уничтожение (особенно при обработке, чтобы оставаться в безопасности).
```c++ 
EntityManager->Defer().DestroyEntities(Entities);
EntityManager->Defer().DestroyEntity(Entity);
```
- Непосредственно

Предпочтительнее использовать `BatchDestroyEntityChunks`, так как он вызывает менеджер наблюдателей за вас. Это действительно безопасно вызывать только вне обработки в главном потоке, как и другие прямые изменения композиции.
`UMassSpawnerSubsystem::DestroyEntities` также вызывает эту функцию.

```c++
EntityManager->BatchDestroyEntityChunks(Collection)
```


<!-- #### Deferred -->

<!-- #### Direct Call -->

<a name="mass-cm-opee"></a>
## 5.3 Управление сущностями

В этом разделе мы рассмотрим наиболее важные инструменты, которые предлагает Mass для управления сущностями. Здесь рассматриваются все операции get и set, а также структуры для работы с ними (фрагмент, архетип, теги...).

**Примечание:** В случаях, когда нам нужно работать с сущностями вне текущего контекста обработки (например, избегать скопления сущностей), можно вызывать все обычные функции подсистемы Mass или отложенные действия над ними. Это не идеально для когерентности кэша, но практически неизбежно в игровом коде. 

## 5.2.1 `FMassEntityView`

`FMassEntityView` - это структура, которая облегчает все виды управления сущностями. Она может быть построена с помощью `FMassEntityHandle` и `FMassEntityManager`. При создании `FMassEntityView` кэширует данные архетипа сущности, что в дальнейшем сократит повторную работу по получению информации о сущности.

Далее мы раскрываем некоторые из соответствующих функций `FMassEntityView`:

<!--TODO: List of relevant functions interesting for the user:-->
<!-- REVIEWMEFUNK slighty better example... -->
В следующем искусственном примере обработчика мы проверяем, является ли `NearbyEntity` врагом, и если да, то повреждаем его:
```c++
FMassEntityView EntityView(Manager, NearbyEntity.Entity);

// Проверьте, есть ли у нас тег
if (EntityView.HasTag<FEnemyTag>())
{
	if(auto DamageOnHitFragment = EntityView.GetFragmentDataPtr<FDamageOnHit>())
	{
	    // Теперь мы откладываем выполнение задания для другой сущности!
 	    FDamageFragment DamageFragment;
 	    DamageFragment.Damage = DamageOnHitFragment.Damage * AttackPower;
        Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(EntityView.GetEntity, DamageFragment);
	}
}
```


<a name="mass-pm"></a>
## 6. Плагины и модули Mass
В этом разделе рассматриваются три основных плагина Mass и их различные модули:

> 6.1 [`MassEntity`](#mass-pm-me)  
> 6.2 [`MassGameplay`](#mass-pm-gp)  
> 6.3 [`MassAI`](#mass-pm-ai)  

<a name="mass-pm-me"></a>
### 6.1 [`MassEntity`](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/)
`MassEntity` является основным плагином, который управляет всем, что касается создания и хранения сущностей.


<a name="mass-pm-gp"></a>
### 6.2 `MassGameplay `
The `MassGameplay' плагин компилирует ряд полезных Фрагментов и Обработчиков, которые используются в различных частях фреймворка Mass. Он разделен на следующие модули:

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
Базовые фрагменты типа `FTransformFragment**.

<a name="mass-pm-gp-mm"></a>
#### 6.2.2 `MassMovement`
Features an important `UMassApplyMovementProcessor` processor that moves entities based on their velocity and force.

<a name="mass-pm-gp-mr"></a>
#### 6.2.3 `MassRepresentation`
Processors and fragments for rendering entities in the world. They generally use an ISMC to do so, but can also swap entities out with full Unreal actors at user specified distances.

<a name="mass-pm-gp-ms"></a>
#### 6.2.4 `MassSpawner`
Очень настраиваемый тип агента, который может порождать сущности там, где вы хотите. Есть два способа выбора мест, один из которых использует актив Environmental Query System, а другой - запрос на основе тегов ZoneGraph. Похоже, что актер Mass Spawner предназначен для вещей, которые спаунятся сразу, например NPC, деревья и т. д., а не для динамически спаунящихся вещей, например, снарядов.

<a name="mass-pm-gp-ma"></a>
#### 6.2.5 `MassActors`
Мост между общим фреймворком акторов UE5 и Mass. Тип фрагмента, превращающего сущности в "агентов", которые могут обмениваться данными в любом направлении (или в обоих).

<a name="mass-pm-gp-ml"></a>
#### 6.2.6 `MassLOD`
Обработчики LOD, которые могут управлять различными уровнями детализации, от рендеринга до тикинга с различной скоростью на основе настроек фрагмента. В настоящее время они также используются в визуализации и репликации.

<a name="mass-pm-gp-mre"></a>
#### 6.2.7 `MassReplication`
Поддержка репликации для Mass! Другие модули переопределяют `UMassReplicatorBase**, чтобы реплицировать материал. Сущностям присваивается отдельный идентификатор сети, который передается по сети, а не хэндл сущности. Пример, демонстрирующий это, планируется сделать гораздо позже.

<a name="mass-pm-gp-msi"></a>
#### 6.2.8 `MassSignals`
Система, позволяющая сущностям посылать друг другу именованные сигналы.

<a name="mass-pm-gp-mso"></a>
#### 6.2.9 [`MassSmartObjects`](https://docs.unrealengine.com/5.0/en-US/smart-objects-in-unreal-engine/)
Позволяет сущностям "требовать" смарт-объекты для взаимодействия с ними.

<!-- This section explicitly for AI specific modules-->
<a name="mass-pm-ai"></a>
### 6.3 MassAI
`MassAI` - это плагин, который предоставляет возможности ИИ для Mass в рамках серии модулей:

> 6.3.1 [`ZoneGraph`](#mass-pm-ai-zg)  
> 6.3.2 [`StateTree`](#mass-pm-ai-st)  
> 6.3.3 ...

Работа над этим разделом, как и над остальными частями документа, еще не завершена.

<!-- FIXME: Ideally, this section should be like the previous one. -->
<!-- FIXME: To what extent do we want to cover the AI side of mass. -->
<!-- FIXMEFUNK: I think we should cover a brief overview at the minimum. most of Mass is attached to the AI stuff so we kind of have to at least mention all of it. The Zonegraph cones are a good short example. We should suggest to check out the CitySample at least. -->

<a name="mass-pm-ai-zg"></a>
#### 6.3.1 `ZoneGraph`
<!-- FIXME: Add screenshots and examples. -->
Сплайны и фигуры на уровне, использующие заданные в конфигурации дорожки для прокладывания путей в zonegraph! Подумайте о тротуарах, дорогах и т.д. Это основной способ передвижения участников Mass Crowd.

<a name="mass-pm-ai-st"></a>
#### 6.3.2 [`StateTree`](https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/)
<!-- FIXME: Add screenshots and examples. -->
Новая легкая универсальная машина состояний, которая может работать в связке с Mass. Один из них используется для задания целей движения конусам на параде в примере.

<a name="mass-or"></a>
## 7. Другие ресурсы

### 7.1 Mass
В этом разделе собраны очень полезные ресурсы по Mass, дополняющие эту документацию.
#### **Официальные ресурсы Epic Games:**.
  - [[Documentation] MassEntity](https://docs.unrealengine.com/5.0/en-US/overview-of-mass-entity-in-unreal-engine/): Обзор системы MassEntity в Unreal Engine.
  - [[Documentation] Mass Avoidance](https://docs.unrealengine.com/5.0/en-US/mass-avoidance-in-unreal-engine/): Mass Avoidance - это система уклонения на основе силы, интегрированная с MassEntity.
  - [[Documentation] Smart Objects](https://docs.unrealengine.com/5.0/en-US/smart-objects-in-unreal-engine/): Смарт-объекты представляют собой набор действий на уровне, который можно использовать через систему резервирования.
  - [[Documentation] StateTree](https://docs.unrealengine.com/5.0/en-US/overview-of-state-tree-in-unreal-engine/): Обзор системы Mass AI StateTree.
  - [[Видео] State of Unreal: Large Numbers of Entities with Mass](https://youtu.be/f9q8A-9DvPo): Марио Палермо (ведущий глобальный евангелист Unreal Engine 5) подробно рассказывает о Mass в 30-минутном видео.

<!-- Huge credit to this blog for teaching us how to use spawners! -->
#### **[@quabqi](https://www.zhihu.com/people/quabqi)'s записи в блоге (на китайском языке):**
  - [ECS of UE5: MASS framework (1)](https://zhuanlan.zhihu.com/p/441773595): Иерархия памяти Mass, представление сущностей и архетипов.
  - [ECS of UE5: MASS framework (2)](https://zhuanlan.zhihu.com/p/446937133): Массовое базовое исполнение.
  - [ECS of UE5: MASS framework (3)](https://zhuanlan.zhihu.com/p/477803528): Глубокое погружение в `MassGameplay`.
  - [MassAI crowd drawing of UE5 CitySample](https://zhuanlan.zhihu.com/p/496165391): Как обрабатываются пешеходы в примере UE5 CitySample?
 
### 7.2 Общие ECS

  - [Sander's Entity Component System FAQ](https://github.com/SanderMertens/ecs-faq): Этот FAQ предназначен для всех, кто интересуется ECS и современной высокопроизводительной разработкой игр.
  - [Data-Oriented Design by Richard Fabian](https://www.dataorienteddesign.com/dodbook/): Книга, описывающая стиль/парадигму программирования под названием "Дизайн, ориентированный на данные". Библиотеки Entity Component System, такие как Mass, делают проектирование, ориентированное на данные, простым!
  - [Evolve Your Hierarchy by Mick West](https://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/): Статья, демонстрирующая, как использовать композицию вместо наследования для представления игровых сущностей.



