/**
 *
 *  \file tests.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#define MASTER_TEST_SET

#define BOOST_TEST_MODULE Master Test Suite
#include <boost/test/unit_test.hpp>

using namespace boost;

#include "factory_fixture1.h"
#include "factory_fixture2.h"
#include "factory_fixture3.h"

#include "test_evolution.cc"
#include "test_fitness.cc"
#include "test_individual.cc"
#include "test_population.cc"
#include "test_primitive.cc"
#include "test_src_constant.cc"
#include "test_terminal.cc"
#include "test_ttable.cc"
