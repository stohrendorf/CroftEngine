# EdisonEngine
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine?ref=badge_shield)


An open-source Tomb Raider 1-5 engine remake.

## Licensing

EdisonEngine is an open-source engine distributed under LGPLv3 license, which means that ANY part of
the source code must be open-source as well.
    

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine?ref=badge_large)

## Credits

The following people did extensive work on the [OpenTomb engine](http://opentomb.github.io/), which was
the starting point for EdisonEngine.  Although EdisonEngine is a complete re-write with negligible
remnants of OpenTomb's code base and pretty much different goals, you can see the commit history of
OpenTomb in this repository.  Because of that - and because I want to be grateful - I'd like to give credit
to the OpenTomb contributors.


* [TeslaRus](https://github.com/TeslaRus): main developer.
* [Cochrane](https://github.com/Cochrane): renderer rewrites and optimizing, Mac OS X support.
* [Gh0stBlade](https://github.com/Gh0stBlade): renderer add-ons, shader port, gameflow implementation,
  state control fix-ups, camera and AI programming.
* [Lwmte](https://github.com/Lwmte): state and scripting fix-ups, controls, GUI and audio modules,
  trigger and entity system rewrites.
* Nickotte: interface programming, ring inventory implementation, camera fix-ups.
* [pmatulka](https://github.com/pmatulka): Linux port and testing.
* [richardba](https://github.com/richardba): Github migration, Github repo maintenance, website design.
* [Saracen](https://github.com/Saracen): room and static mesh lighting.
* [T4Larson](https://github.com/T4Larson): general stability patches and bugfixing.
* [vobject](https://github.com/vobject): nightly builds, maintaining general compiler compatibility.
* [vvs-](https://github.com/vvs-): testing, feedback, bug report.
* [xproger](https://github.com/xproger): documentation updates.
* [Banderi](https://github.com/Banderi): documentation, bugfixing.
* [gabrielmtzcarrillo](https://github.com/gabrielmtzcarrillo): entity shader work.
* [filfreire](https://github.com/filfreire): documentation.

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port),
godmodder (general help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and xythobuz (additional Mac patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).


## Building

EdisonEngine uses [vcpkg](https://github.com/Microsoft/vcpkg); refer to its documentation for basic usage,
but the only thing you need is to call cmake with
`-DCMAKE_TOOLCHAIN_FILE=C:/devel/vcpkg/scripts/buildsystems/vcpkg.cmake` (adjust path as necessary).

This is the list of the required libraries to be installed with `vcpkg install` (remember to
set the target triplet as necessary, e.g. `vcpkg install boost:x64-windows`):
* boost
* glfw3
* libpng
* openal-soft
* opengl
* lua
* glm
* zlib
* libsndfile
* freetype
* yaml-cpp
* sol2
* cimg
* ffmpeg
* glew

...or use this, and adjust the triplet:
> `vcpkg install --triplet x64-windows boost glfw3 libpng openal-soft opengl lua glm zlib libsndfile freetype yaml-cpp sol2 cimg ffmpeg glew`