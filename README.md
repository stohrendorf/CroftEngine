EdisonEngine — an open-source Tomb Raider 1-5 engine remake
-----------------------------------------------------------

1. Licensing
------------

EdisonEngine is an open-source engine distributed under LGPLv3 license, which means that ANY part of
the source code must be open-source as well.
    
2. Credits
----------

* TeslaRus: main developer.
* ablepharus: compilation fix-ups and miscellaneous patches.
* Cochrane: renderer rewrites and optimizing, Mac OS X support.
* Gh0stBlade: renderer add-ons, shader port, gameflow implementation, state fix-ups, camera.
* Lwmte: state fix-ups, controls, GUI and audio modules, trigger and entity scripts.
* Nickotte: interface programming, ring inventory implementation, camera fix-ups.
* pmatulka: Linux port and testing.
* Richard_trle: Github migration, Github repo maintenance, website design.
* Saracen: room and static mesh lighting.
* stltomb: general code maintenance, enhancements and bugfixes.
* stohrendorf: CXX-fication, general code refactoring and optimizing.
* T4Larson: general stability patches and bugfixing.
* vobject: nightly builds, maintaining general compiler compatibility.
* vvs-: extensive testing and bug reporting.

Additional contributions from: Ado Croft (extensive testing), E. Popov (TRN caustics shader port),
godmodder (general help), jack9267 (vt loader optimization), meta2tr (testing and bugtracking),
shabtronic (renderer fix-ups), Tonttu (console patch) and xythobuz (additional Mac patches).

Translations by: Joey79100 (French), Nickotte (Italian), Lwmte (Russian), SuiKaze Raider (Spanish).


3. Building
-----------
EdisonEngine uses [vcpkg](https://github.com/Microsoft/vcpkg); refer to its documentation for basic usage,
but the only thing you need is to call cmake with
`-DCMAKE_TOOLCHAIN_FILE=C:/devel/vcpkg/scripts/buildsystems/vcpkg.cmake` (adjust path as necessary).

This is the list of the required libraries to be installed with `vcpkg install` (remember to
set the target triplet as necessary, e.g. `vcpkg install boost:x64-windows`):
* boost
* glfw3
* glew
* ms-gsl
* libpng
* openal-soft
* opengl
* lua
* glm
* zlib
* libsndfile
* freetype
* yaml-cpp
* assimp