# [VITA](https://bitbucket.org/morinim/vita) #

## Overview ##

Vita is a scalable, high performance genetic programming / genetic algorithms environment.

It's suitable for classification, symbolic regression, content base image retrieval, data mining and agent control. Main features:

* flexible, fast and clean design
* easy integration with other systems
* simple addition of features and modules
* fast experimentation with detailed run-log
* modern, standard ISO C++11/C++14 source code

It's distributed under the [Mozilla Public License v2.0](https://www.mozilla.org/MPL/2.0/) (see the accompanying LICENSE file for more details).

This software was originally developed by [EOS][1] without open source in mind. Later (early 2011) the code has been commented, restructured, documented and released as open source.

Although the core development team is still anchored at EOS (the project is maintained by Manlio Morini), Vita is now open source and we would like it to be run by an international team of AI enthusiasts.

## Requirements ##

Vita is designed to have fairly minimal requirements to build and use with your projects, but there are some. Currently, we support Linux and Windows. We will also make our best effort to support other platforms (e.g. Mac OS X, Solaris, AIX).
However, since core members of the Vita project have no access to these platforms, Vita may have outstanding issues there. If you notice any problems on your platform, please use the
[issue tracking system](https://bitbucket.org/morinim/vita/issues). Patches for fixing them are even more welcome!

### Linux requirements ###

* GNU-compatible make
* POSIX-standard shell
* [Boost library][2]
* A C++11-standard-compliant compiler (or, at least, partially compatible: e.g. gcc v4.8.x or newer, clang 3.3 or newer)
* Python v3 (for some functions)

### Windows requirements ###

* MinGW/MSYS with a C++11-standard-compliant compiler (or, at least, partially compatible: e.g. gcc v4.8.x or newer, clang 3.3 or newer)
* [Boost library][2]
* Python v3 (for some functions)

## Getting the source ##

There are two primary ways of getting Vita's source code: you can download a stable source release in your preferred archive format or directly check out the source from the Mercurial (HG) repository.

The HG checkout requires a few extra steps and some extra software packages on your system, but lets you track the latest development and make patches much more easily, so we highly encourage it.

### HG Checkout ###

Run the following Mercurial command:
```
hg clone https://morinim@bitbucket.org/morinim/vita
```

### Source Package ###

Vita is released in versioned source packages which can be downloaded from the project's [download page](https://bitbucket.org/morinim/vita/downloads). Several different archive formats are provided, but the only difference is the tools used to manipulate them and the size of the resulting file. Download whichever you are most comfortable with.

Once the package is downloaded, expand it using whichever tools you prefer for that type. This will result in a new directory with the name "vita-X.Y.Z" which contains all of the source code. Here are some examples on Linux:
```
$ tar -xvJf vita-X.Y.Z.txz
$ tar -xvjf vita-X.Y.Z.tar.bz2
$ unzip vita-X.Y.Z.zip
```

## The Vita distribution ##

This is a sketch of the resulting directory structure:
```
vita/
  LICENSE
  README.md
  src/
    AUTHORS
    copyright
    Makefile
    examples/..............Various examples
    kernel/ ...............Vita kernel(core library)
    sr/ ...................Symbolic regression and classification utility
    test/ .................Vita-wide test-suite
    tools/ ................Utilities (C++ lint checker...)
```

## Setting up the build ##

If they are not already installed, you need the following libraries:

* [BOOST][2]

Kernel library can be built with a header-only Boost library; other programs may need Boost separately-compiled library binaries (e.g. SR utility needs Program Options Library to compile). To build Boost library under MinGW see below.

### GCC / MinGW ###

```cd vita```

* To build all: `make`
* To build the kernel library (libvita.a): `make kernel`
* To build the sr tool: `make sr`
* To build tests: `make tests`
* To build example ABC: `make examples/ABC`

If you see errors, try to tweak the contents of Makefile to make them go away.

The default compiler is g++, to use clang++ type `make CXX=clang++`.

All output files will, per default, be stored together with the source code.

With MinGW, if you're [experiencing long delays](http://stackoverflow.com/q/8571657/3235496) after the `make` command, try `make -r` (avoid implicit rules).

### Additional instructions for building Boost on MinGW / MSYS ###

Our standard Windows test environment is [Nuwen](http://nuwen.net/mingw.html): it's simple and ready to use.

If you choose other distributions, you'd need to build Boost:

1. Set up a MinGW/MSYS environment.
2. Get the latest Boost.
3. Extract Boost archive under `vita/src/` directory (you can expand Boost in a different path but you'll have to add the `stage\lib` directory to the library search path and `boost/` to the include search path).
4. Open the Windows command prompt (`cmd.exe` *not* MSYS) and cd to your boost directory.
5. Execute
    `bootstrap.bat mingw`
    and
    `b2 link=static runtime-link=static threading=multi toolset=gcc variant=release --build-type=minimal --layout=system`
6. Go and make yourself *several* cups of sencha green tea.
7. The stage subdir now contains libs.

[1]: http://www.eosdev.it/
[2]: http://www.boost.org/