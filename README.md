# [VITA][homepage] #
[![Vita](https://github.com/morinim/vita/wiki/img/logo.png)][homepage]
[![Version](https://img.shields.io/github/tag/morinim/vita.svg)][news]

[![Travis](https://travis-ci.org/morinim/vita.svg?branch=master)][travis]
![C++14](https://img.shields.io/badge/c%2B%2B-14-blue.svg)
[![CII](https://bestpractices.coreinfrastructure.org/projects/1012/badge)][cii]
[![License](https://img.shields.io/badge/license-MPLv2-blue.svg)][mpl2]
[![Twitter](https://img.shields.io/twitter/url/https/github.com/morinim/vita.svg?style=social)][twitter]

## Overview ##

Vita is a scalable, high performance genetic programming / algorithms framework.

It's suitable for classification, symbolic regression, content base image retrieval, data mining and agent control. Main features:

* flexible, fast and clean design
* easy integration with other systems
* simple addition of features and modules
* fast experimentation with detailed run-log
* modern, standard ISO C++14 source code
* [more][features]

This software was originally developed by [EOS][eos] without open source in mind. Later (early 2011) the code has been commented, restructured, documented and released as open source.

Although the core development team is still anchored at EOS, Vita is now open source and we would like it to be run by an international team of AI enthusiasts.

## Build requirements ##

Vita is designed to have fairly minimal requirements to build and use with your projects, but there are some. Currently, we support Linux and Windows. We will also make our best effort to support other platforms (e.g. Mac OS X, Solaris, AIX).
However, since core members of the Vita project have no access to these platforms, Vita may have outstanding issues there. If you notice any problems on your platform, please use the
[issue tracking system][issue]; patches for fixing them are even more welcome!

### Mandatory ###

* A C++14-standard-compliant compiler (or, at least, partially compatible: e.g. gcc v5.3.x or newer, clang 3.8 or newer)
* [CMake][cmake]

### Optional ###

* [Python v3][python] for additional functionalities

## Getting the source ##

There are two ways of getting Vita's source code: you can [download][download] a stable source release in your preferred archive format or directly clone the source from a repository.

Cloning a repository requires a few extra steps and some extra software packages on your system, but lets you track the latest development and make patches much more easily, so we highly encourage it.

Run the following command:

```
git clone https://github.com/morinim/vita.git
```

## The Vita distribution ##

This is a sketch of the resulting directory structure:
```
vita/
  NEWS.md..................Don't let your friends dump git logs into NEWS
  CODE_OF_CONDUCT.md.......Standards for how to engage in this community
  CONTRIBUTING.md
  LICENSE
  README.md
  copyright
  contributors/
  doc/
  misc/
  src/
    CMakeLists.txt
    build/ ................Built executables
    examples/ .............Various examples
      forex/ ..............Forex example
      sr/ .................Symbolic regression and classification utility
    kernel/ ...............Vita kernel (core library)
    test/ .................Test-suite
    third_party/ ..........Third party libraries
    utility/ ..............Support libraries / files
  tools/ ..................C++ lint checker and other tools
```

## Setting up the build ##

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

You're now ready to build using the traditional `make` system:

* everything: `make`
* kernel library (`libvita.a`): `make vita`
* `sr` tool: `make sr`
* tests: `make tests`
* the *ABC* example: `make ABC`
* for the list of the valid targets: `make help`

All the output files will be stored in subdirectories of `build/` (out of source builds).

Windows may need various expedients about which you can read in the [Windows walkthrough][windows].

## Documentation ##
There is a [comprehensive wiki][wiki].

## License ##
[Mozilla Public License v2.0][mpl2] (also available in the accompanying [LICENSE][license] file).



[cii]: https://bestpractices.coreinfrastructure.org/projects/1012
[cmake]: https://cmake.org/
[download]: https://github.com/morinim/vita/archive/master.zip
[eos]: https://www.eosdev.it/
[features]: https://github.com/morinim/vita/wiki/features
[homepage]: https://github.com/morinim/vita
[issue]: https://github.com/morinim/vita/issues
[license]: https://github.com/morinim/vita/blob/master/LICENSE
[mpl2]: https://www.mozilla.org/MPL/2.0/
[news]: https://github.com/morinim/vita/blob/master/NEWS.md
[python]: http://www.python.org/
[travis]: https://travis-ci.org/morinim/vita
[twitter]: https://twitter.com/intent/tweet?text=%23Vita+genetic+programming:&url=https%3A%2F%2Fgithub.com%2Fmorinim%2Fvita
[wiki]: https://github.com/morinim/vita/wiki
[windows]: https://github.com/morinim/vita/wiki/win_build
