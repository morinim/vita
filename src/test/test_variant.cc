/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <iostream>

#include "kernel/variant.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE variant_test
#include "boost/test/unit_test.hpp"

using namespace boost;
#endif

struct has_destructor
{
  has_destructor(bool *f) : flag(f) {}
  ~has_destructor() { *flag = true; }

  bool *flag;
};

struct is_string_visitor
{
  using result_type = bool;

  bool operator()(int) const { return false; }
  bool operator()(bool) const { return false; }
  bool operator()(std::string) const { return true; }
};

BOOST_AUTO_TEST_SUITE(variant)

BOOST_AUTO_TEST_CASE(type_info)
{
  BOOST_TEST_CHECKPOINT("TYPE_INFO SIZE");
  auto size(vita::detail::type_info<char>::size);
  BOOST_CHECK_EQUAL(size, sizeof(char));
  size = vita::detail::type_info<int>::size;
  BOOST_CHECK_EQUAL(size, sizeof(int));

  size = vita::detail::type_info<char, int, double, std::string>::size;
  BOOST_CHECK_EQUAL(size, std::max({sizeof(char), sizeof(int), sizeof(double),
                                    sizeof(std::string)}));

  BOOST_TEST_CHECKPOINT("TYPE_INFO DUPLICATES");
  bool no_duplicates(
    vita::detail::type_info<bool, int, std::string>::no_duplicates);
  BOOST_CHECK(no_duplicates);

  no_duplicates = vita::detail::type_info<int, bool, int>::no_duplicates;
  BOOST_CHECK_EQUAL(no_duplicates, false);

  no_duplicates = vita::detail::type_info<bool, int, int>::no_duplicates;
  BOOST_CHECK_EQUAL(no_duplicates, false);

  no_duplicates = vita::detail::type_info<int, int, bool>::no_duplicates;
  BOOST_CHECK_EQUAL(no_duplicates, false);

  no_duplicates = vita::detail::type_info<int, int &>::no_duplicates;
  BOOST_CHECK(no_duplicates);

  no_duplicates = vita::detail::type_info<int, const int>::no_duplicates;
  BOOST_CHECK(no_duplicates);

  BOOST_TEST_CHECKPOINT("TYPE_INFO ALIGNMENT");
  auto alignment(vita::detail::type_info<char>::alignment);
  BOOST_CHECK_EQUAL(alignment, alignof(char));
  alignment = vita::detail::type_info<double>::alignment;
  BOOST_CHECK_EQUAL(alignment, alignof(double));
}

BOOST_AUTO_TEST_CASE(assignment)
{
  vita::variant<int, bool, double, std::string> v(4);
  BOOST_CHECK_EQUAL(v.get<int>(), 4);

  for (int i(-100); i <= 100; ++i)
  {
    v = i;
    BOOST_CHECK(v.get<int>() == i);
  }

  v = true;
  BOOST_CHECK(v.get<bool>() == true);
  v = false;
  BOOST_CHECK(v.get<bool>() == false);

  v = 1.0;
  BOOST_CHECK_CLOSE(v.get<double>(), 1.0, 0.001);
  v = -1.0;
  BOOST_CHECK_CLOSE(v.get<double>(), -1.0, 0.001);

  v = std::string("abcdef");
  BOOST_CHECK_EQUAL(v.get<std::string>(), "abcdef");
}

BOOST_AUTO_TEST_CASE(which)
{
  vita::variant<int, bool, double, std::string> v(4);
  BOOST_CHECK(v.which() == 0);

  v = true;
  BOOST_CHECK(v.which() == 1);

  v = 3.14;
  BOOST_CHECK(v.which() == 2);

  v = std::string("a");
  BOOST_CHECK(v.which() == 3);
}

BOOST_AUTO_TEST_CASE(variant_destructor)
{
  bool destructed(false);

  {
    vita::variant<int, has_destructor> v = has_destructor(&destructed);
  }

  BOOST_CHECK(destructed);
}

BOOST_AUTO_TEST_CASE(assignment_destruction)
{
  bool destructed(false);

  vita::variant<int, has_destructor> v = has_destructor(&destructed);

  v = 5;
  BOOST_CHECK(destructed);
}

BOOST_AUTO_TEST_CASE(visiting)
{
  vita::variant<int, bool, std::string> v(4);
  is_string_visitor is_string;
  BOOST_CHECK(!v.visit(is_string));

  v = std::string("abc");
  BOOST_CHECK(v.visit(is_string));
}

BOOST_AUTO_TEST_SUITE_END()
