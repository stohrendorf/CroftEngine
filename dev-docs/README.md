# Architecture

## A bit of history and design principles

CE has its roots in [OpenTomb](https://github.com/TeslaRus/OpenTomb), as I was once looking for a modern alternative to
the original engine without DosBox, allowing me to play Tomb Raider on a modern system while taking advantage of modern
hardware. This fork started around 2015, and has grown since, so let's discuss the design principles.

In general, the engine is built to make use of modern technology that's generally available for the past 5-ish years for
gamers, while still maintaining the original feel and aesthetics. The engine is also designed to be accessible and
customizable, so that everyone can adjust it for them to enjoy it the most, without being rocket science. Unfortunately,
this also means that it's not "the original" - especially since the code is written to be safe, which in turn means
there were some unavoidable changes in some algorithms, especially AI path finding and camera code (and both still have
bugs I wasn't able to resolve after years).

I have watched hours and hours of streams and YouTube videos to understand what other people find enjoyable, and what
could be expanded on, which led to some well-received features like edge outlines. I wanted this to feel like a true
remaster, not just like "let's ship the hi-res textures and models we used during development." I know some people
disagree on this, but that's fine. The goal is to make this a fun, enjoyable engine, which is easily adjustable to make
it enjoyable *for you* (side note, butt bubbles were implemented after having a fun stream over a beer).

In conclusion...

* make it type-safe, modern C++
* make it fun (even while your own definition of fun may differ from others)
* don't make it realistic (it's a game, and fun must be the priority, not a simulation of life)
* don't be afraid of crazy ideas like butt bubbles (you can still remove it later if it proves to be too crazy)
* just fun!

## Build process

When building, the CMake configuration will create wrappers for some enums that allow easy de-/serialization and
conversion. For example, [tracks_tr1.txt](../src/tracks_tr1.txt) will be converted into an enum class, as well as some
utility functions that allow conversion from and to strings, as well as enumerating all enum members.

The build configuration also downloads external dependencies, such as CImg, glm, and others.

The engine has several submodules, mostly separated within the `src` folder:

* The `archive` module is responsible for accessing archives. It is used to
  import 3rd party levels, as well as to package up the local ghost data.
* The `audio` module is responsible for anything audio-related. It handles
  audio streams, positional audio, and static audio not related to a position.
* The `core` module contains anything used throughout the whole engine. It contains
  very generic stuff like vectors and i18n that is used globally across many
  other modules.
* The `dosbox-cdrom` module contains the code for accessing disk images, used to
  import the main game data.
* The `engine` module contains the heart, and is the most complex one. It connects
  nearly all the other modules and makes the game work.
* The [`etcpak`](#etcpak) module contains the texture compression code for caching texture
  pack data.
* The `ffmpeg` module contains everything to decode audio and video data.
* The `gameflow` module is a small module to handle gameflow metadata.
* The `hid` module is responsible to handle user input.
* The `launcher` module contains anything the user gets to see first, i.e. the
  main window.
* The `loader` submodule contains everything to slurp in the data files.
* The `menu` module contains anything regarding the in-game menu, i.e. the
  inventory. It is implemented as a state machine.
* The `network` module contains all the stuff needed to play online with others.
  It contains authorization and state synchronization.
* The [`qs`](#qs) module contains everything regarding the quantity system, which is used
  to protect against using the wrong values on compile-time. It brought up several
  issues in the past after implementing this. Devils be in there.
* The [`render`](#render) submodule is an abstraction of the OpenGL wrapper `soglb` to make
  it easier to draw anything on the screen.
* The `serialization` module is for serializing and deserializing data from and to
  YAML. It is designed to be a plug-in thing, so that only the implementations need
  to include the headers necessary to (de-)serialize anything. Devils be in there,
  too.
* The `shared` submodule contains some generally helpful stuff. It's not within
  `core`, as it's containing stuff more specific to certain use-cases.
* [`soglb`](#soglb) contains the `Structured OpenGL Bindings` (so it's not my initials here).
  This is a type-safe wrapper around the OpenGL API.
* The `testutil` module contains some stuff to better integrate into CI pipelines.
  It's not relevant to anything in general, and you shouldn't worry about it.
* The [`ui`](#ui) submodule is the UI framework, containing stuff like text boxes, lists,
  message boxes, etc., anything that defines the UI.
* The `util` submodule contains some stuff even less common than the `shared` module,
  even while it's used across some other modules
* The `video` module contains all the stuff to display the FMVs.

## audio

The "audio" module handles anything regarding audio effects. It controls:

* the listener position,
* the listener's audio effects (like muffling underwater),
* allocating voices to sound effects depending on the position since there's a limited amount of concurrently playing
  sounds,
* and multiple channels of audio streams.

Regarding audio streams, the engine allocates channels to audio streams, where each channel can only play a single
stream. This allows switching ambient streams without overlapping, because they're using the same channel, while still
playing interception audio like cinematic music at the same time in a different channel.

## core

Contains i18n support, generally used types like angles, vectors, type-safe IDs, intervals and so on, but it also
contains the magic values like the sector size, climb limits, or the frame rate in [`magic.h`](../src/core/magic.h).
Heavily relies on the [`qs`](#qs) module.

## dosbox-cdrom

This module contains functionality to access CUE/DAT images necessary to import game data from GOG or Steam
installations.

## engine

This is the heart of CE, and it contains several submodules itself. The `Engine` class is a container for anything
needed game-flow-wise to run a level.

The engine is generally built to allow loading multiple levels (or "worlds") at the same time, or even multiple
gameflows. While that led to a cleaner architecture, it is not used at all, but could be used to preload a level once
the prior level ends during the stats screen.

First, the engine initializes the script engine for the chosen game-flow. Then, the engine sets up a `Presenter`, which
basically is a container around everything a user can sense - audio and video currently, but may also include gamepad
rumble effects in the future once it's implemented in GLFW (blocked
by [issue #57](https://github.com/glfw/glfw/issues/57) and [pull 1687](https://github.com/glfw/glfw/pull/1678)). The
presenter also handles the [`hid`](#hid) and OpenGL window using [`soglb`](#soglb).

Once the `Presenter` is initialized, the user's chosen render settings are applied, their input config is applied, and (
optionally) the configured Glidos texture pack is loaded. Loading the Glidos texture pack does not involve loading the
textures yet, it is just loading the general layout information, as the materialized textures are cached using
[`etcpak`](#etcpak).

Once the engine is initialized, the control returns to the main file.

The engine also maintains a cache for the world geometry, so that re-loading the same level is sped up significantly.
The cache is reset on level change.

### ai

This submodule contains general AI stuff like behaviour handling and pathfinding.

The pathfinding code is one of the most convoluted areas in this engine, as the original code wasn't in the best shape.
Additionally, the original code had several major issues with out-of-bounds accesses, which led to necessary changes
that made the code even worse and behave differently. There are still bugs in the pathfinding code after all these
years - if you can find and fix them, you're a god.

### floordata

Contains everything for triggers and floor data logic. The "floor data" are sector-based instructions. For example, when
you're walking over a bridge, there is an instruction to activate that bridge, otherwise you would fall through. Another
example are boundary rooms - when reaching the edge of a room, an instruction will be triggered that changes the
entity's parent room.

It also contains a function to get a floordata's secret mask, which is used to count the secrets in a level without
relying on external information (this leads to a single discrepancy with the original TR1 levels).

### ghosting

This submodule is responsible to read and write ghost data, as well as providing a scene node for the ghost
(see the [`render`](#render) module).

### lara

Lara is basically a state machine. Every animation Lara plays has a set state. This submodule provides the states'
behaviour. For example, the "Jump Back" state handler sets the desired state to "Free Fall" if the vertical velocity
exceeds a certain threshold. When Lara's animation is played, the animation frames can provide some state transition
information, allowing to switch to a different animation, given a certain frame range. This will implicitly switch to a
different animation state handler.

### objects

Contains everything that defines the game entities. The enemies' behaviours are defined here, as well as objects'
behaviours. The `update()` function is called each frame for every active object. There are usually custom serialization
overrides for entities that have specific state variables (see also [`serialization`](#serialization)), and also custom
`collide()` overrides to handle collisions with Lara (like the hand of Midas or switches). Some entities like bridges
are also able to change the effective ceiling or floor height when they're activated through [`floordata`](#floordata).

### script

Contains the whole script engine. This loads the game-flow scripts and provides reflection for interacting with the
scripts.

Game-flow scripts provide the following data:

* entity descriptors like smartness, behaviour stereotype, health, etc.
* soundtrack mappings from internal identifiers to files and some audio configurations, like fade-in time or the
  slot/channel the audio files should be allocated to (see [`audio`](#audio))
* the level sequence
* meta-information about the game like authors and the title

### world

Provides the massaged data necessary to run everything, including some optimizations for easier data access and
converted game data (see [`loader`](#loader)).

Currently, [`worldgeometry`](../src/engine/world/worldgeometry.h) is responsible for converting the raw game data into
something useful. It holds animation data, meshes, massaged [`floordata`](#floordata), rooms, audio data, etc.

This module is also responsible to re-map textures into the large texture atlases used by the engine, as well as
applying Glidos texture packs if configured. Materialized textures are cached using [`etcpak`](#etcpak).

```mermaid
classDiagram
    RoomGeometry --o Mesh
    RoomGeometry --o TextureAnimator
    RoomGeometry ..> AnimatedUV
    WorldGeometry ..> StaticMesh
    WorldGeometry ..> Mesh
    WorldGeometry ..> SkeletalModelType
    WorldGeometry --o Sprite
    WorldGeometry --o SpriteSequence
    WorldGeometry --o AtlasTile
    WorldGeometry --o Animation
    WorldGeometry ..> Texture2DArray
    WorldGeometry --o RoomGeometry
    RoomGeometry ..> Mesh
    RoomGeometry ..> TextureAnimator
    RoomGeometry ..> AnimatedUV
    RoomGeometry ..> Mesh
    World ..> Engine
    World --* AudioEngine
    World --* CameraController
    World --> Voice
    World --* ObjectManager
    World --o PositionalEmitter
    World --o PickupWidget
    World ..> Player
    World --* FloorData
    World --o Box
    World --o Room
    World --o StaticSoundEffect
    World --o CinematicFrame
    World --o CameraSink
    Box ..> "0..*" Box
    SkeletalModelType ..> AnimFrame
    SkeletalModelType ..> Animation
    Sector ..> Box
    Sector ..> Room
    Room ..> Portal
    Portal ..> Room
    Portal ..> Mesh
    Room --o Light
    Room --o Portal
    Room --o Sector
    Room --o RoomStaticMesh
    Room ..> Room
    Room --* InstancedParticleCollection
    Room --* RoomGeometry
    PositionalEmitter --|> Emitter
    World --* WorldGeometry
    WorldGeometry --o StaticMesh
    WorldGeometry --o Mesh
    WorldGeometry --o SkeletalModelType
    WorldGeometry --o Sprite
    WorldGeometry --o SpriteSequence
    WorldGeometry --o AtlasTile
    WorldGeometry --o Animation
    WorldGeometry --o Texture2DArray
    WorldGeometry --* "0..*" RoomGeometry
```

#### Re-texturing

This happens in [`texturing.cpp`](../src/engine/world/texturing.cpp). The texturing class also maintains a texture size
cache file which heavily speeds up loading cached textures because only the layout process needs to be done, and loading
a cached texture does not involve loading individual, costly texture files.

The individual textures of the original texture atlases are determined by iterating over every primitive and get their
bounding boxes. This step also removes spurious "inner boxes" (boxes that are contained within other boxes).

After the layout is done (which only needs the bounding boxes of the original textures, and the sizes of the new
textures), the new layout is applied to the appropriate UV coordinates so they point to the changed texture coordinates.
If no cached texture exists yet, the texture atlases are materialized, which also includes expanding the edges to avoid
color and alpha bleeding when scaling down for mip-maps.

If no Glidos texture pack is configured, this process only re-arranges the original texture atlases into larger ones.

The final texture atlases are finally stored in a `gl::Texture2DArray<gl::PremultipliedSRGBA8>` resource.

#### Level data conversion

Level data is converted into engine digestible data in `World::initFromLevel`. Before conversion, a few simple
pre-flight checks are done to identify problematic data - this is useful to identify broken or problematic custom levels
which are usually produced by Tomb Editor. Fixing the level data in these levels programmatically is basically
impossible without risking to break working levels.

After the pre-flight checks are done, the conversion starts. Conversion usually includes replacing indices with direct
pointers or separating individual flags or values from bit fields (partially done by the raw loader already). Also, the
scene tree is set up here (see [`render`](#render)).

## etcpak

This is basically a copy of https://github.com/wolfpld/etcpak, but with some modifications to reduce code complexity,
increase type safety, and easier integration for texture compression and decompression. Its main purpose is to reduce
I/O and performance latencies for texture caching when using texture packs.

## hid

The "Human Input Device" module is responsible to handle everything the user inputs for the game. It can translate
joystick movement to axis directions and merge multiple input configurations where several different inputs may lead to
the same action.

## loader

Contains everything to read game data files and Glidos texture packs. This module only provides the raw game data and
does not do any processing necessary to render any entity. The conversion from raw data to anything useful to the engine
is done within the [`engine`](#engine) module.

## menu

The menu module contains the inventory code. It resembles the original code to some extent as that it is still a state
machine. However, here it is built around different class instances for different states. It makes heavy use of the
[`ui`](#ui) module.

## network

Contains code to connect, authenticate, and communicate
with [haunted-coop](https://github.com/stohrendorf/haunted-coop).

## qs

This is the "Quantity System." It allows to create a type-safe unit system, where the compiler will fail if you try, for
example, pass a velocity to a function that expects an acceleration. It will automatically change units when you combine
different quantities, for example, it will change to unit to "acceleration" if you divide a velocity by a time unit. It
is _not_ designed to be something that can handle every combination of units or quantities, but it is more than
sufficient for the task.

## render

This contains everything to abstract the OpenGL API. It contains, for example, functionality to bind a shader parameter
to a callback function, which allows setting the parameter dynamically when meshes are drawn. It also contains the whole
render pipeline, with all the framebuffers needed for all the effects. It is an even more abstract wrapper
around [`soglb`](#soglb).

### Rendering

Scenes are built as trees of nodes. Each node has an optional "Renderable," which provides the visual of this node. Each
node can be hidden; this property is recursive. The nodes' transforms are relative to their parents and are calculated
on-demand. Each node contains a "render state" which contains only the necessary changes to the OpenGL state, overriding
only the specified settings of their parent's node. Each "Renderable" may also provide a render state.

Rendering the scene tree is done using a visitor pattern. The visitor maintains a stack of render states that are
applied to the OpenGL state when rendering a visual.

Each rendering step is done within a framebuffer. These framebuffers are usually from render passes, for example bloom
or HBAO.

When rendering, the engine performs an initial "depth prefill" pass, pre-rendering the rooms without its entities into
the depth buffer, to avoid rendering visuals with expensive fragment shaders.

## serialization

Probably even more complicated than the [`qs`](#qs) module. This module provides a de-/serialization interface for YAML
files. It is designed as a plugin architecture, so that header files don't need to be polluted with specific
serializers, like `std::map`s or `std::filesystem::path`. It also helps with compile time.

In general, a serialization of something involves a `dispatch` call, which looks for freestanding de-/serialization and
factory methods suitable for the job, or methods contained within classes with the same names. Within these methods, the
serializer is then recursively called back-and-forth.

## soglb

These are the "Structured OpenGL Bindings." Essentially, these are type-safe wrappers around the OpenGL API, including
classes supporting the C++ RAII concept. This module is designed to provide a convenient, type- and context-safe API.

Additionally, it also contains a `Text` class handling modern fonts and rendering them into images. This is an outlier
for the purpose of this module, however.

## ui

This is the UI widget module used for the inventory. It is designed to automatically accommodate the contents of the
widgets so that the developer does not need to care about sizing or positioning too much. It also contains some
functionality to break long strings of text into lines.

When drawing the UI, the draw commands (lines, rects, etc.) are collected in the `Ui` class so all commands can be
rendered using a single draw call.

## video

Contains the code responsible to play videos and make them look less ugly after upscaling.

## Code flow overview

Generally, it works like this (simplified, read docs above for missing details):

```mermaid
flowchart TD
    main((main)) --> launcher
    launcher --> quit(((quit)))
    bootstrap["run bootstrap sequence<br>(Eidos logo FMV etc.)"]
    menu["main menu"]
    menu --> launcher
    menu --> nextlevel
    launcher --> load_gameflow["load gameflow script"] --> bootstrap

    subgraph gameflow loop
        nextlevel["select level from save<br>or first level sequence item"]
        nextlevel --> load["load level data<br>and texture pack"] --> loadsave["load savegame<br>(optional)"]
        loadsave --> level["run level"] --> nextlevel
        level --> inventory --> level
        level --> menu
        inventory --> menu
    end

    bootstrap --> nextlevel
```

## Audio engine

The audio engine manages all in-game sounds, including audio streams, positional audio, entity-bound positional audio,
and non-positional audio (e.g. when Lara is talking). It also hosts the sound engine from the [`audio`](#audio) module,
which is the abstraction of the underlying OpenAL API.

When loading a level, all level audio (except streams) is stored here. It handles "audio effects," which are descriptors
that contain multiple sounds, sound playback chance, pitch range for random pitch, etc. The game engine only requests
audio effects or streams from the audio engine.

The audio engine also handles which sounds are played at the same time, since there's only a limited amount of audible
"voices." Depending on the distance between the listener and the sound, the audio engine may stop too distant sounds and
re-allocate the freed voice to another sound that's closer.

```mermaid
classDiagram
    AudioEngine --* SoundEngine
    AudioEngine --o VoiceGroup
    AudioEngine --o BufferHandle
    AudioEngine ..> SoundEffectProperties
    SoundEngine --* Device
    SoundEngine ..> Voice
    SoundEngine --o SlotStream
    VoiceGroup ..> Voice
    SlotStream ..> StreamVoice
    StreamVoice ..> Voice
    StreamVoice ..> StreamingSourceHandle
    StreamVoice --* AbstractStreamSource
    StreamVoice --o BufferHandle
    BufferVoice --|> Voice
    BufferVoice --* BufferHandle
    Voice --* SourceHandle
    Device --o Voice
    StreamingSourceHandle --o BufferHandle
    AbstractStreamSource <|-- BasicFfmpegStreamSource
    BasicFfmpegStreamSource --* AudioStreamDecoder
    BasicFfmpegStreamSource <|-- FfmpegMemoryStreamSource
    BasicFfmpegStreamSource <|-- FfmpegStreamSource
    BasicFfmpegStreamSource <|-- FfmpegSubStreamStreamSource
```