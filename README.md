# [VITA][3] #

## Overview ##

Vita is a scalable, high performance genetic programming / algorithms framework.

It's suitable for classification, symbolic regression, content base image retrieval, data mining and agent control. Main features:

* flexible, fast and clean design
* easy integration with other systems
* simple addition of features and modules
* fast experimentation with detailed run-log
* modern, standard ISO C++14 source code

It's distributed under the [Mozilla Public License v2.0](https://www.mozilla.org/MPL/2.0/) (see the accompanying LICENSE file for more details).

This software was originally developed by [EOS][1] without open source in mind. Later (early 2011) the code has been commented, restructured, documented and released as open source.

Although the core development team is still anchored at EOS, Vita is now open source and we would like it to be run by an international team of AI enthusiasts.

## Build requirements ##

Vita is designed to have fairly minimal requirements to build and use with your projects, but there are some. Currently, we support Linux and Windows. We will also make our best effort to support other platforms (e.g. Mac OS X, Solaris, AIX).
However, since core members of the Vita project have no access to these platforms, Vita may have outstanding issues there. If you notice any problems on your platform, please use the
[issue tracking system](https://bitbucket.org/morinim/vita/issues); patches for fixing them are even more welcome!

### Mandatory ###

* GNU-compatible make
* POSIX-standard shell
* A C++14-standard-compliant compiler (or, at least, partially compatible: e.g. gcc v5.3.x or newer, clang 3.8 or newer)

Under Windows this means a recent version of MinGW / MSYS (our standard Windows test environment is [Nuwen](http://nuwen.net/mingw.html): it's simple and ready to use).

### Optional ###

* [CMake][5]
* [Boost library][2] for some executables and for testing
* [Python v3][4] for additional functionalities

## Getting the source ##

There are two ways of getting Vita's source code: you can download a stable source release in your preferred archive format or directly clone the source from a repository.

Cloning a repository requires a few extra steps and some extra software packages on your system, but lets you track the latest development and make patches much more easily, so we highly encourage it.

### HG Clone ###

Run the following command:
```
hg clone https://bitbucket.org/morinim/vita
```

### Git Clone ###

```
git clone https://github.com/morinim/vita.git
```

Please consider that the [primary repository][3] is Mercurial-based (HG) and hosted on Bitbucket. The repository on Github is a mirror kept for Git users' convenience.

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
  copyright
  src/
    CONTRIBUTORS
    Makefile
    examples/..............Various examples
    kernel/ ...............Vita kernel(core library)
    sr/ ...................Symbolic regression and classification utility
    test/ .................Test-suite
    third_party/ ..........Third party libraries
    utility/ ..............Support libraries / files
  tools/ ..................C++ lint checker and other tools
```

## Setting up the build ##

Kernel library can be built without a header-only [Boost library][2]; other programs may need Boost headers / separately-compiled library binaries (e.g. SR utility needs Program Options Library to compile).

### make ###

```cd vita/src```

* To build all: `make`
* To build the kernel library (libvita.a): `make vita`
* To build the sr tool: `make sr`
* To build tests: `make tests`
* To build example ABC: `make examples/ABC`

The default compiler is g++, to use clang++ type `make CXX=clang++`.

All the output files will, per default, be stored together with the source
code.

With MinGW, if you're [experiencing long delays](http://stackoverflow.com/q/8571657/3235496) after the `make` command, try `make -r` (avoid implicit rules).

### cmake ###

```
cd vita/src
mkdir build
cd build/
cmake ..
```

To suggest a specific compiler you can write:

```
CXX=clang++ cmake ..
```

You're now ready to build using the traditional `make` system (see above).

All the output files will be stored in subdirectories of `build/` (out of source builds).


[1]: http://www.eosdev.it/
[2]: http://www.boost.org/
[3]: https://bitbucket.org/morinim/vita
[4]: http://www.python.org/
[5]: https://cmake.org/