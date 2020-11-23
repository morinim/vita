/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *
 *  \mainpage VITA v2.1.0
 *
 *  \section Introduction
 *  Welcome to the Vita project.
 *
 *  This is the reference guide for the APIs. Introductory documentation is
 *  available at the [repository's Wiki](https://github.com/morinim/vita/wiki).
 *
 *  \section note_sec Notes
 *  New versions of this framework will be available at
 *  https://github.com/morinim/vita
 *
 *  Please reports any suggestions and/or bugs via the
 *  [issue tracking system](https://github.com/morinim/vita/issues).
 *
 */

/**
 *  \page page1 VITA Architecture
 *  - [Tutorials](https://github.com/morinim/vita/wiki/tutorials)
 *  - [Anatomy of Vita](https://github.com/morinim/vita/wiki/anatomy)
 *
 *  \page page2 Contributor guidelines
 *  - [Coding style](https://github.com/morinim/vita/wiki/coding_style)
 *  - [Development cycle](https://github.com/morinim/vita/wiki/development_cycle)
 *  - [Contributor Code of Conduct](https://github.com/morinim/vita/blob/master/CODE_OF_CONDUCT.md)
 */

/**
 *  \namespace vita
 *  The main namespace for the project.
 *
 *  \namespace alps
 *  Contains support functions for the ALPS algorithm.
 *
 *  \namespace out
 *  Contains flags and manipulators to control the output format of individuals.
 *
 *  \namespace detail
 *  The contents of this namespace isn't intended for general consumption: it
 *  contains implementation details, not the interface.
 *  It is recommended giving details their own file and tucking it away in a
 *  detail folder.
 */

/**
 *  \example pathfinding01.cc
 *  Simple pathfinding (Genetic Algorithms).
 *  \example pathfinding02.cc
 *  More complex pathfinding (Genetic Algorithms).
 *
 *  \example rastrigin.cc
 *  More complex pathfinding (Genetic Algorithms).
 *
 *  \example titanic01.cc
 *  "Hello world" classification task (Genetic Programming).
 *  \example titanic02.cc
 *  "Hello world" classification task (calculating accuracy metric).
 *  \example titanic03.cc
 *  "Hello world" classification task (setting some parameters).
 *  \example titanic04.cc
 *  "Hello world" classification task (team-based classification).
 *
 *  \example scheduling.cc
 *  Scheduling concurrent jobs for several machines (Differential Evolution).
 */

// This is a global "project documentation" file that supplies the front page
// docs for the project, sets up the groups (for use with /ingroup tags) and
// documents the namespaces all in one place. This allows all the "overviews"
// to be held in one logical place rather than scattered to the winds.
