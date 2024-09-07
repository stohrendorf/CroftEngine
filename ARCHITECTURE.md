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
* The `etcpak` module contains the texture compression code for caching texture
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
* The `qs` module contains everything regarding the quantity system, which is used
  to protect against using the wrong values on compile-time. It brought up several
  issues in the past after implementing this. Devils be in there.
* The `render` submodule is an abstraction of the OpenGL wrapper `soglb` to make
  it easier to draw anything on the screen.
* The `serialization` module is for serializing and deserializing data from and to
  YAML. It is designed to be a plug-in thing, so that only the implementations need
  to include the headers necessary to (de-)serialize anything. Devils be in there,
  too.
* The `shared` submodule contains some generally helpful stuff. It's not within
  `core`, as it's containing stuff more specific to certain use-cases.
* `soglb` contains the `Structured OpenGL Bindings` (so it's not my initials here).
  This is a type-safe wrapper around the OpenGL API.
* The `testutil` module contains some stuff to better integrate into CI pipelines.
  It's not relevant to anything in general, and you shouldn't worry about it.
* The `ui` submodule is the UI framework, containing stuff like text boxes, lists,
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
render pipeline, with all the framebuffers needed for all the effects.

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

Contains everything to read game data files and Glidos texture packs.

## ui

This is the UI widget module used for the inventory. It is designed to automatically accommodate the contents of the
widgets so that the developer does not need to care about sizing or positioning too much. It also contains some
functionality to break long strings of text into lines.

## soglb

These are the "Structured OpenGL Bindings." Essentially, these are type-safe wrappers around the OpenGL API, including
classes supporting the C++ RAII concept.

Additionally, it also contains a `Text` class handling modern fonts and rendering them into images. This is an outlier
for the purpose of this module, however.
