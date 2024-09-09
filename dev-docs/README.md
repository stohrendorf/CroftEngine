# Architecture

## Overview

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

## Build process

When building, the CMake configuration will create wrappers for some enums that allow easy de-/serialization and
conversion. For example, [tracks_tr1.txt](./src/tracks_tr1.txt) will be converted into an enum class, as well as some
utility functions that allow conversion from and to strings, as well as enumerating all enum members.

The build configuration also downloads external dependencies, such as CImg, glm, and others.

## etcpak

This is basically a copy of https://github.com/wolfpld/etcpak, but with some modifications to reduce code complexity,
increase type safety, and easier integration for texture compression and decompression. Its main purpose is to reduce
I/O and performance latencies for texture caching when using texture packs.

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

## qs

This is the "Quantity System." It allows to create a type-safe unit system, where the compiler will fail if you try, for
example, pass a velocity to a function that expects an acceleration. It will automatically change units when you combine
different quantities, for example, it will change to unit to "acceleration" if you divide a velocity by a time unit. It
is _not_ designed to be something that can handle every combination of units or quantities, but it is more than
sufficient for the task.

## hid

The "Human Input Device" module is responsible to handle everything the user inputs for the game. It can translate
joystick movement to axis directions and merge multiple input configurations where several different inputs may lead to
the same action.

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

## dosbox-cdrom

This module contains functionality to access CUE/DAT images necessary to import game data from GOG or Steam
installations.

## loader

Contains everything to read game data files and Glidos texture packs. This module only provides the raw game data and
does not do any processing necessary to render any entity. The conversion from raw data to anything useful to the engine
is done within the [`engine`](#engine) module.

## ui

This is the UI widget module used for the inventory. It is designed to automatically accommodate the contents of the
widgets so that the developer does not need to care about sizing or positioning too much. It also contains some
functionality to break long strings of text into lines.

## soglb

These are the "Structured OpenGL Bindings." Essentially, these are type-safe wrappers around the OpenGL API, including
classes supporting the C++ RAII concept. This module is designed to provide a convenient, type- and context-safe API.

Additionally, it also contains a `Text` class handling modern fonts and rendering them into images. This is an outlier
for the purpose of this module, however.

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