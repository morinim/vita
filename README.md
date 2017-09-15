# [VITA][3] #

## Overview ##

Vita is a scalable, high performance genetic programming / algorithms framework.

It's suitable for classification, symbolic regression, content base image retrieval, data mining and agent control. Main features:

* flexible, fast and clean design
* easy integration with other systems
* simple addition of features and modules
* fast experimentation with detailed run-log
* modern, standard ISO C++14 source code

It's distributed under the [Mozilla Public License v2.0][7] (see the accompanying [LICENSE][8] file for more details).

This software was originally developed by [EOS][1] without open source in mind. Later (early 2011) the code has been commented, restructured, documented and released as open source.

Although the core development team is still anchored at EOS, Vita is now open source and we would like it to be run by an international team of AI enthusiasts.

## Build requirements ##

Vita is designed to have fairly minimal requirements to build and use with your projects, but there are some. Currently, we support Linux and Windows. We will also make our best effort to support other platforms (e.g. Mac OS X, Solaris, AIX).
However, since core members of the Vita project have no access to these platforms, Vita may have outstanding issues there. If you notice any problems on your platform, please use the
[issue tracking system][9]; patches for fixing them are even more welcome!

### Mandatory ###

* GNU-compatible make
* POSIX-standard shell
* A C++14-standard-compliant compiler (or, at least, partially compatible: e.g. gcc v5.3.x or newer, clang 3.8 or newer)

For details about building Vita under Windows take a look at the [specific walkthrough][6].

### Optional ###

* [CMake][5]
* [Boost library][2] for some executables and for testing
* [Python v3][4] for additional functionalities

## Getting the source ##

There are two ways of getting Vita's source code: you can [download](https://github.com/morinim/vita/archive/master.zip) a stable source release in your preferred archive format or directly clone the source from a repository.

Cloning a repository requires a few extra steps and some extra software packages on your system, but lets you track the latest development and make patches much more easily, so we highly encourage it.

Run the following command:

```
git clone https://github.com/morinim/vita.git
```

## The Vita distribution ##

This is a sketch of the resulting directory structure:
```
vita/
  CHANGELOG.md
  CONTRIBUTING.md
  LICENSE
  README.md
  copyright
  contributors/
  doc/
  misc/
  src/
    CMakeLists.txt
    Makefile
    build/ ................Built executables
    examples/ .............Various examples
    kernel/ ...............Vita kernel(core library)
    sr/ ...................Symbolic regression and classification utility
    test/ .................Test-suite
    third_party/ ..........Third party libraries
    utility/ ..............Support libraries / files
  tools/ ..................C++ lint checker and other tools
```

## Setting up the build ##

Kernel library can be built without the [Boost library][2]; some programs / examples may need Boost headers / separately-compiled library binaries (e.g. the `sr` utility needs *Boost Program Options Library* to compile).

### make ###

```cd vita/src```

* To build (almost) all: `make`
* To build the kernel library (`libvita.a`): `make vita`
* To build the sr tool: `make sr`
* To build / perform tests: `make tests`
* To build example ABC: `make examples/ABC`

The default compiler is g++, to use clang++ type `make CXX=clang++`.

All the output executables will, per default, be stored inside the `build/`
directory.

### cmake ###

```shell
cd vita/src
mkdir -p build
cd build/
cmake ..
```

To suggest a specific compiler you can write:

```shell
CXX=clang++ cmake ..
```

You're now ready to build using the traditional `make` system (see above).

All the output files will be stored in subdirectories of `build/` (out of source builds).

The real story is that Windows may need various expedients about which you can read in the [Windows walkthrough][6].


[1]: http://www.eosdev.it/
[2]: http://www.boost.org/
[3]: https://github.com/morinim/vita
[4]: http://www.python.org/
[5]: https://cmake.org/
[6]: https://github.com/morinim/vita/wiki/win_build
[7]: https://www.mozilla.org/MPL/2.0/
[8]: https://github.com/morinim/vita/blob/master/LICENSE
[9]: https://github.com/morinim/vita/issues