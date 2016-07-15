/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#define MASTER_TEST_SET

#define BOOST_TEST_MODULE MASTER_TEST_SUITE
#include <boost/test/unit_test.hpp>

constexpr double epsilon(0.00001);

using namespace boost;

#include "test/factory_fixture1.h"
#include "test/factory_fixture2.h"
#include "test/factory_fixture3.h"
#include "test/factory_fixture4.h"
#include "test/factory_fixture5.h"

#include "test/cache.cc"
#include "test/discretization.cc"
#include "test/evolution.cc"
#include "test/evolution_selection.cc"
#include "test/fitness.cc"
#include "test/ga.cc"
//#include "test_ga_perf.cc"
#include "test/i_mep.cc"
#include "test/i_ga.cc"
#include "test/lambda.cc"
#include "test/matrix.cc"
#include "test/population.cc"
#include "test/primitive_d.cc"
#include "test/primitive_i.cc"
#include "test/small_vector.cc"
#include "test/src_constant.cc"
#include "test/src_problem.cc"
#include "test/summary.cc"
#include "test/symbol_set.cc"
#include "test/team.cc"
#include "test/terminal.cc"
//#include "test/variant.cc"
