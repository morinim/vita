[![Vita](https://github.com/morinim/vita/wiki/img/logo.png)][homepage]
[![Version](https://img.shields.io/github/tag/morinim/vita.svg)][news]

![C++17](https://img.shields.io/badge/c%2B%2B-17-blue.svg)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/morinim/vita?svg=true)][appveyor]
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/morinim/vita.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/morinim/vita/context:cpp)
[![CII](https://bestpractices.coreinfrastructure.org/projects/1012/badge)][cii]
[![License](https://img.shields.io/badge/license-MPLv2-blue.svg)][mpl2]
[![Twitter](https://img.shields.io/twitter/url/https/github.com/morinim/vita.svg?style=social)][twitter]

## Overview ##

Vita is a scalable, high performance framework for genetic programming and genetic algorithms.

It's suitable for [classification][classification], [symbolic regression][sr], content base image retrieval, data mining and [software agent][agent] implementation. Main features:

* flexible and fast
* easy integration with other systems
* simple addition of features and modules
* fast experimentation with detailed run-log
* modern, standard ISO C++17 source code
* [more][features]

This software was originally developed by [EOS][eos] without open source in mind. Later (early 2011) the code has been commented, restructured, documented and released as open source.

Although the core development team is still anchored at EOS, Vita is now open source and we would like it to be run by an international team of AI enthusiasts.

## Symbolic regression example ##

```C++
// DATA SAMPLE
// (the target function is `x + sin(x)`)
std::istringstream training(R"(
  -9.456,-10.0
  -8.989, -8.0
  -5.721, -6.0
  -3.243, -4.0
  -2.909, -2.0
   0.000,  0.0
   2.909,  2.0
   3.243,  4.0
   5.721,  6.0
   8.989,  8.0
)");

// READING INPUT DATA
vita::src_problem prob(training);

// SETTING UP SYMBOLS
prob.insert<vita::real::sin>();
prob.insert<vita::real::cos>();
prob.insert<vita::real::add>();
prob.insert<vita::real::sub>();
prob.insert<vita::real::div>();
prob.insert<vita::real::mul>();

// SEARCHING
vita::src_search s(prob);
auto result(s.run());
```

It's pretty straightforward (further details in the [specific tutorial][sr]).

## Documentation ##

There is a [comprehensive wiki][wiki]. You should probably start with the [tutorials][tutorials].

## Build requirements ##

Vita is designed to have fairly minimal requirements to build and use with your projects, but there are some. Currently, we support Linux and Windows. We will also make our best effort to support other platforms (e.g. Mac OS X, Solaris, AIX).
However, since core members of the Vita project have no access to these platforms, Vita may have outstanding issues there. If you notice any problems on your platform, please use the
[issue tracking system][issue]; patches for fixing them are even more welcome!

### Mandatory ###

* A C++17-standard-compliant compiler
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
  doc/
  misc/
  src/
    CMakeLists.txt
    examples/ .............Various examples
      forex/ ..............Forex example
      sr/ .................Symbolic regression and classification utility
    kernel/ ...............Vita kernel (core library)
    test/ .................Test-suite
    third_party/ ..........Third party libraries
    utility/ ..............Support libraries / files
  tools/ ..................C++ lint checker and other tools
  CODE_OF_CONDUCT.md.......Standards for how to engage in this community
  CONTRIBUTING.md
  LICENSE
  NEWS.md..................Don't let your friends dump git logs into NEWS
  README.md
```

## Setting up the build ##

```shell
cd vita
cmake -B build/ src/
```

To suggest a specific compiler you can write:

```shell
CXX=clang++ cmake -B build/ src/
```

You're now ready to build using the underlying build system tool:

* everything: `cmake --build build/`
* kernel library (`libvita.a`): `cmake --build build/ --target vita`
* `sr` tool: `cmake --build build/ --target sr`
* tests: `cmake --build build/ --target tests`
* the *ABC* example: `cmake --build build/ --target ABC`
* for a list of valid targets: `cmake --build build/ --target help`

The output files are stored in subdirectories of `build/` (out of source build).

Windows may need various expedients about which you can read in the [Windows walkthrough][windows].

## Installing Vita ##

To install Vita use the command:

```shell
cmake --install build/
```
(requires superuser, i.e. root, privileges)

Manually installing is also very easy. There are just two files, both inside the `build/kernel` directory:

- a static library (e.g. `libvita.a` under Unix);
- an automatically generated global/single header (`auto_vita.h` which can be renamed).

As a side note, the command to build the global header is:

```shell
./tools/single_include.py --src-include-dir src/ --src-include kernel/vita.h --dst-include mysingleheaderfile.h
```
(must be executed from the repository main directory)

## License ##

[Mozilla Public License v2.0][mpl2] (also available in the accompanying [LICENSE][license] file).

## Versioning ##

Vita uses [semantic versioning][semver]. Releases are tagged.

Note that the major version will change rapidly and API changes are fairly common. Read the [NEWS.md][news] file for details about the breaking changes.


[agent]: https://github.com/morinim/vita/wiki/forex_tutorial
[appveyor]: https://ci.appveyor.com/project/morinim/vita
[cii]: https://bestpractices.coreinfrastructure.org/projects/1012
[classification]: https://github.com/morinim/vita/wiki/titanic_tutorial
[cmake]: https://cmake.org/
[download]: https://github.com/morinim/vita/archive/master.zip
[eos]: https://www.eosdev.it/
[features]: https://github.com/morinim/vita/wiki/features
[homepage]: https://github.com/morinim/vita
[issue]: https://github.com/morinim/vita/issues
[license]: https://github.com/morinim/vita/blob/master/LICENSE
[mpl2]: https://www.mozilla.org/MPL/2.0/
[news]: https://github.com/morinim/vita/blob/master/NEWS.md
[python]: https://www.python.org/
[semver]: https://semver.org/
[sr]: https://github.com/morinim/vita/wiki/symbolic_regression
[tutorials]: https://github.com/morinim/vita/wiki/tutorials
[twitter]: https://twitter.com/intent/tweet?text=%23Vita+genetic+programming:&url=https%3A%2F%2Fgithub.com%2Fmorinim%2Fvita
[wiki]: https://github.com/morinim/vita/wiki
[windows]: https://github.com/morinim/vita/wiki/win_build