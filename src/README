VITA <http://code.google.com/p/vita/>

These are the release notes for Vita. Read them carefully, as they tell you
what this is all about, explain how to install the software and what to do if
something goes wrong.

OVERVIEW

  Vita is a scalable, high performance C++ environment / toolkit for
  classification, symbolic regression, content base image retrieval, data mining
  and agent control.

  It's distributed under the Mozilla Public License v2.0 - see the accompanying
  LICENSE file for more details.

REQUIREMENTS

  Vita is designed to have fairly minimal requirements to build and use with
  your projects, but there are some. Currently, we support Linux and Windows.
  We will also make our best effort to support other platforms (e.g. Mac OS X,
  Solaris, AIX).
  However, since core members of the Vita project have no access to these
  platforms, Vita may have outstanding issues there. If you notice any
  problems on your platform, please notify vita@googlecode.com or use the
  issue tracking system (http://code.google.com/p/vita/issues/list). Patches for
  fixing them are even more welcome!

  ### Linux requirements ###

  * GNU-compatible make
  * POSIX-standard shell
  * Boost library (<http://www.boost.org/>)
  * A C++11-standard-compliant compiler (or, at least, partially compatible:
    e.g. gcc v4.8.x or newer, clang 3.3 or newer)
  * Python v3 (for some functionalities)

  ### Windows requirements ###

  * MinGW/MSYS with a C++11-standard-compliant compiler (or, at least,
    partially compatible: e.g. gcc v4.8.x or newer, clang 3.3 or newer).
  * Boost library (<http://www.boost.org/>)
  * Python v3 (for some functionalities)

GETTING THE SOURCE

  There are two primary ways of getting Vita's source code: you can download a
  stable source release in your preferred archive format or directly check out
  the source from the Mercurial (HG) repository.
  The HG checkout requires a few extra steps and some extra software packages
  on your system, but lets you track the latest development and make patches
  much more easily, so we highly encourage it.

  ### Source Package ###

  Vita is released in versioned source packages which can be downloaded from
  the download page [1]. Several different archive formats are provided, but
  the only difference is the tools used to manipulate them and the size of the
  resulting file. Download whichever you are most comfortable with.

  [1] http://code.google.com/p/vita/wiki/Downloads

  Once the package is downloaded, expand it using whichever tools you prefer
  for that type. This will result in a new directory with the name
  "vita-X.Y.Z" which contains all of the source code. Here are some examples
  on Linux:

    tar -xvJf vita-X.Y.Z.txz
    tar -xvjf vita-X.Y.Z.tar.bz2
    unzip vita-X.Y.Z.zip

  ### HG Checkout ###

  Run the following Mercurial command:

    hg clone https://code.google.com/p/vita/

THE VITA DISTRIBUTION

  This is a sketch of the resulting directory structure:

  vita/
    AUTHORS
    copyright
    LICENSE
    Makefile
    README
    examples/..............Various examples
    kernel/ ...............Vita kernel(core library)
    sr/ ...................Symbolic regression and classification utility
    test/ .................Vita-wide test-suite
    tools/ ................Utilities (C++ lint checker...)

SETTING UP THE BUILD

  If they are not already installed, you need the following libraries:

  * BOOST (<http://www.boost.org/>)

  Kernel library can be built with a header-only Boost library; other programs
  may need boost separately-compiled library binaries (e.g. SR utility needs
  Program Options Library to compile). To build Boost library under MinGW
  see below.

  ### GCC / MinGW ###

  cd vita

  * To build all: make
  * To build the kernel library (libvita.a): make kernel
  * To build the sr tool: make sr
  * To build tests: make tests
  * To build example <n>: make example<n>

  If you see errors, try to tweak the contents of Makefile to make them go
  away.
  The default compiler is g++, to use clang++ type:

    make CXX=clang++

  All output files will per default be stored together with the source code.

  ### Additional instructions for building Boost on MinGW / MSYS ###

  1. Set up a MinGW/MSYS environment (our standard Windows test environment
     is Win-Builds/MSYS <http://win-builds.org/stable/msys-cygwin.html>);
  2. Get the latest Boost (<http://www.boost.org/>);
  3. Extract Boost archive under vita/ directory (you can expand Boost in
     a different path but you'll have to change the BOOST_INCLUDE variable
     in the Makefile);
  4. Open the Windows command prompt (cmd.exe, *not* MSYS) and cd to your
     boost directory;
  6. Execute
       bootstrap.bat mingw
     and
       b2 link=static runtime-link=static threading=multi toolset=gcc variant=release --build-type=minimal --layout=system
  7. Go and make yourself *several* cups of green tea;
  8. The stage subdir now contains libs.

  If you prefer something ready-to-use, one of the best distro is
  <http://nuwen.net/mingw.html>.