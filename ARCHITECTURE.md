# Overview

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
