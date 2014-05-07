/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#define MASTER_TEST_SET

#define BOOST_TEST_MODULE Master Test Suite
#include <boost/test/unit_test.hpp>

constexpr double epsilon(0.00001);

using namespace boost;

#include "factory_fixture1.h"
#include "factory_fixture2.h"
#include "factory_fixture3.h"
#include "factory_fixture4.h"

#include "test_evolution.cc"
#include "test_fitness.cc"
#include "test_individual.cc"
#include "test_lambda.cc"
#include "test_matrix.cc"
#include "test_population.cc"
#include "test_primitive_d.cc"
#include "test_primitive_i.cc"
#include "test_src_constant.cc"
#include "test_summary.cc"
#include "test_symbol_set.cc"
#include "test_team.cc"
#include "test_terminal.cc"
#include "test_ttable.cc"
