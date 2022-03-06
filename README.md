# ![EdisonEngine logo](./share/logo_32.png) EdisonEngine

[![EdisonEngine-CI](https://github.com/stohrendorf/EdisonEngine/actions/workflows/repo-sync.yml/badge.svg)](https://github.com/stohrendorf/EdisonEngine/actions/workflows/repo-sync.yml)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine?ref=badge_shield)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/stohrendorf/EdisonEngine.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/stohrendorf/EdisonEngine/alerts/)
[![Language grade: Python](https://img.shields.io/lgtm/grade/python/g/stohrendorf/EdisonEngine.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/stohrendorf/EdisonEngine/context:python)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/stohrendorf/EdisonEngine.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/stohrendorf/EdisonEngine/context:cpp)

An open-source Tomb Raider 1 engine remake.

![EdisonEngine logo](./share/splash.png)

> *Showcase Trailer* (Raina Audron)
>
> [![Edison Engine Showcase Trailer](https://img.youtube.com/vi/IAA6ILvQ4Uw/0.jpg)](https://www.youtube.com/watch?v=IAA6ILvQ4Uw)
>
> *The Lost Valley Playthrough* (Raina Audron)
>
> [![The Lost Valley Playthrough in Edison Engine](https://img.youtube.com/vi/o8FEo1QU1QM/0.jpg)](https://www.youtube.com/watch?v=o8FEo1QU1QM)
>
> *EdisonEngine - Tomb Raider, Enhanced! w/ developer commentary \[Beta 10\] \[PC\] \[Stream Archive\]* (Dfactor Longplays)
>
> [![EdisonEngine - Tomb Raider, Enhanced! w/ developer commentary \[Beta 10\] \[PC\] \[Stream Archive\]](https://img.youtube.com/vi/ayMPVAKTMWI/0.jpg)](https://www.youtube.com/watch?v=ayMPVAKTMWI)

## Licensing

EdisonEngine is an open-source engine distributed under LGPLv3 license, which means that ANY part of the source code
must be open-source as well.

[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2Fstohrendorf%2FEdisonEngine?ref=badge_large)

## Usage

> *A user manual has been kindly provided by Raina Audron [here](https://drive.google.com/file/d/1iRz4Svdi_dhmlDNFYeZqyP-wfApWhDoN/view)*.

> ***Please note that as of Beta 15 a setup assistant is included that should start if anything is missing.***

1. Get the most recent release from [here](https://github.com/stohrendorf/EdisonEngine/releases). It is recommended to
   use the installer in Windows. For openSUSE users, the engine is available for most distribution versions in the
   "Games" repository.
2. You will possibly encounter bugs or strange oddities. Please don't think too much, report them immediately in
   the [issue tracker](https://github.com/stohrendorf/EdisonEngine/issues) here. I can't fix things I'm not aware of. If
   possible, attach screenshots (which are by default bound to F12) and instructions how to toggle that bug. If you
   can't reproduce the bug, at least try to be as specific as you can when describing the bug. Any information I can
   gather is in fact helpful to nail that bug down.
3. You need soundtrack files, grab them from [here](https://opentomb.earvillage.net/).
4. On Windows, navigate to `%LOCALAPPDATA%` and create a directory `edisonengine` there; on Linux, navigate
   to `~/.local/share` and create a directory `edisonengine` there.
5. Ensure your the files in the newly created directory look something like this:
   ```
   edisonengine
   └ data
     └ tr1
       ├ AUDIO
       │ ├ 002.ogg
       │ ├ 003.ogg
       │ └ ...
       ├ DATA
       │ ├ CUT1.PHD
       │ ├ CUT2.PHD
       │ └ ...
       └ FMV
         ├ CAFE.RPL
         ├ CANYON.RPL
         └ ...
   ```
6. You should now be able to run EdisonEngine. If something bad happens as mentioned above, or something doesn't work as
   expected, make a copy of `edisonengine.log` located in the data directory, and attach the file to the issue. You may
   also get help by [joining Discord](https://discord.gg/ndBqb5BmkH), I'll be there in the #edisonengine channel and
   chat with you as soon as I can, there are also other members already involved enough to give you some guidance.
7. The default keybindings are WASD for movement Q and E for stepping left and right, Space for jump, Shift for walking,
   X for rolling, Ctrl for Action, 1 for drawing pistols, 2 for shotguns, 3 for uzis and 4 for magnums. You can consume
   small medi packs by pressing 5, and large ones by pressing 6. Quicksaves and loading them can be done using F5 and
   F6. You can take screenshots by pressing F12. The menu can be opened using Esc, and videos can be skipped using Esc.

## Credits

The following people deserve some noteworthy credit.

* Raina Audron for creating the first YouTube videos and writing the user manual.
* [Dirk Stoecker](https://build.opensuse.org/users/dstoecker) for being the package maintainer of the openSUSE packages.
* [Liinx86](https://www.twitch.tv/liinx86) for being the world's first streamer streaming EdisonEngine on twitch; also
  gave the idea for the Ghost feature.
* [Tomb_of_Ash](https://www.twitch.tv/tomb_of_ash) for being the second streamer.
* [Dfactor Longplays](https://www.youtube.com/channel/UCwLhYb4QDAzQfzbhAHGKjgQ) for showing that the Corner Bug works.
* All testers reporting bugs.
* Every contributor.
* All players.

The following people did extensive work on the [OpenTomb engine](http://opentomb.github.io/), which was the starting
point for EdisonEngine. Although EdisonEngine is a complete re-write with negligible remnants of OpenTomb's code base
and pretty much different goals, you can see the commit history of OpenTomb in this repository. Because of that - and
because I want to be grateful - I'd like to give credit to the OpenTomb contributors.

* [TeslaRus](https://github.com/TeslaRus): main developer.
* [Cochrane](https://github.com/Cochrane): renderer rewrites and optimizing, Mac OS X support.
* [Gh0stBlade](https://github.com/Gh0stBlade): renderer add-ons, shader port, gameflow implementation, state control
  fix-ups, camera and AI programming.
* [Lwmte](https://github.com/Lwmte): state and scripting fix-ups, controls, GUI and audio modules, trigger and entity
  system rewrites.
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

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port), godmodder (general
help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking), shabtronic (renderer fix-ups), Tonttu (
console patch) and xythobuz (additional Mac patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).

## Building

Do a `git submodule update --init --recursive`.

EdisonEngine uses [vcpkg](https://github.com/Microsoft/vcpkg); refer to its documentation for basic usage, but the only
thing you need is to call cmake with
`-DCMAKE_TOOLCHAIN_FILE=C:/devel/vcpkg/scripts/buildsystems/vcpkg.cmake` (adjust path as necessary).

This is the list of the required libraries to be installed with `vcpkg install` (remember to set the target triplet as
necessary, e.g. `vcpkg install boost:x64-windows`):

* boost
* glfw3
* libpng
* openal-soft
* opengl
* freetype
* ffmpeg
* utfcpp
* gettext
* libarchive

...or use this, and adjust the triplet:
> `vcpkg install --triplet x64-windows boost glfw3 libpng openal-soft opengl freetype ffmpeg utfcpp gettext libarchive`

Additionally, an installation of [Qt 5](https://www.qt.io) is necessary; as manually downloading and installing this
package is usually faster than letting it build through vcpkg, it is not included in the above list. If you don't want
to register for a Qt online account, you have to build it yourself, though, by installing the `qt5` vcpkg package.

## Generating Glad OpenGL bindings

**Warning!** The [Glad](https://glad.dav1d.de/) bindings have been manually patched to always try to load
the `GL_ARB_bindless_texture` extension, regardless of whether `GL_EXTENSIONS` reports it or not. This is to allow
debugging with [RenderDoc](https://github.com/baldurk/renderdoc/).

See [src/soglb/glad-patches.md](./src/soglb/glad-patches.md) for the applied patches.

### Generator Settings

Use the following settings to generate the bindings:

* OpenGL version 4.5, core profile
* No loader generation
* Do not omit KHR
* Local files
* Extensions:
    * GL_ARB_bindless_texture
    * GL_ARB_texture_filter_anisotropic
    * GL_ATI_meminfo
    * GL_EXT_texture_filter_anisotropic
    * GL_NVX_gpu_memory_info
