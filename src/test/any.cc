/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <cstdlib>
#include <iostream>
#include <vector>

#include "kernel/cache_hash.h"
#include "utility/any.h"

#if !defined(MASTER_TEST_SET)
#define BOOST_TEST_MODULE Any
#include <boost/test/unit_test.hpp>

using namespace boost;
#endif

namespace std
{
template<class T>
struct hash<std::vector<T>>
{
  std::size_t operator()(const std::vector<T> &obj) const
  {
    std::size_t ret(0);

    for (const auto &i : obj)
      ret += 11 * std::hash<T>()(i);

    return ret;
  }
};


std::istream &operator>>(std::istream &i, std::vector<int> &)
{
  return i;
}

std::ostream &operator<<(std::ostream &o, const std::vector<int> &)
{
  return o;
}
}

BOOST_AUTO_TEST_SUITE(test_any)

BOOST_AUTO_TEST_CASE(default_ctor)
{
  const vita::any value;

  BOOST_CHECK(!value.has_value());
  BOOST_CHECK(vita::any_cast<int>(&value) == nullptr);
  BOOST_CHECK(value.type() == typeid(void));
}

BOOST_AUTO_TEST_CASE(converting_ctor)
{
  std::string text("test message");

  vita::any value(text);

  BOOST_CHECK(value.has_value());
  BOOST_CHECK(value.type() == typeid(std::string));
  BOOST_CHECK(vita::any_cast<int>(&value) == nullptr);
  BOOST_CHECK(vita::any_cast<std::string>(&value));
  BOOST_CHECK(vita::any_cast<std::string>(value) == text);
  BOOST_CHECK(vita::any_cast<std::string>(&value) != &text);
}

BOOST_AUTO_TEST_CASE(copy_ctor)
{
  std::string text("test message");
  vita::any original(text), copy(original);

  BOOST_CHECK(copy.has_value());
  BOOST_CHECK(original.type() == copy.type());
  BOOST_CHECK(vita::any_cast<std::string>(original)
              == vita:: any_cast<std::string>(copy));
  BOOST_CHECK(text == vita::any_cast<std::string>(copy));
  BOOST_CHECK(vita::any_cast<std::string>(&original)
              != vita::any_cast<std::string>(&copy));
}

BOOST_AUTO_TEST_CASE(copy_assign)
{
  std::string text("test message");
  vita::any original(text), copy;
  vita::any *assign_result = &(copy = original);

  BOOST_CHECK(copy.has_value());
  BOOST_CHECK(original.type() == copy.type());
  BOOST_CHECK(vita::any_cast<std::string>(original)
              == vita::any_cast<std::string>(copy));
  BOOST_CHECK(text == vita::any_cast<std::string>(copy));
  BOOST_CHECK(vita::any_cast<std::string>(&original)
              != vita::any_cast<std::string>(&copy));
  BOOST_CHECK(assign_result == &copy);
}

BOOST_AUTO_TEST_CASE(bad_cast)
{
  std::string text("test message");
  vita::any value(text);

  BOOST_CHECK_THROW(vita::any_cast<char *>(value), vita::bad_any_cast);
}

BOOST_AUTO_TEST_CASE(swap)
{
  std::string text("test message");
  vita::any original(text), swapped;
  std::string *original_ptr(vita::any_cast<std::string>(&original));
  vita::any *swap_result(&original.swap(swapped));

  BOOST_CHECK(!original.has_value());
  BOOST_CHECK(swapped.has_value());
  BOOST_CHECK(swapped.type() == typeid(std::string));
  BOOST_CHECK(text == vita::any_cast<std::string>(swapped));
  BOOST_CHECK(original_ptr);
  BOOST_CHECK(swap_result == &original);
}

BOOST_AUTO_TEST_CASE(null_copying)
{
  const vita::any null;
  vita::any copied = null, assigned;
  assigned = null;

  BOOST_CHECK(!null.has_value());
  BOOST_CHECK(!copied.has_value());
  BOOST_CHECK(!assigned.has_value());
}

BOOST_AUTO_TEST_CASE(cast_to_reference)
{
  vita::any a(137);
  const vita::any b(a);

  int                &ra(   vita::any_cast<               int &>(a));
  const int          &ra_c( vita::any_cast<         const int &>(a));
  volatile int       &ra_v( vita::any_cast<      volatile int &>(a));
  volatile const int &ra_cv(vita::any_cast<volatile const int &>(a));

  BOOST_CHECK(&ra == &ra_c);
  BOOST_CHECK(&ra == &ra_v);
  BOOST_CHECK(&ra == &ra_cv);

  const int          &rb_c( vita::any_cast<         const int &>(b));
  volatile const int &rb_cv(vita::any_cast<volatile const int &>(b));

  BOOST_CHECK(&rb_c == &rb_cv);
  BOOST_CHECK(&ra != &rb_c);

  ++ra;
  int incremented(vita::any_cast<int>(a));
  BOOST_CHECK(incremented == 138);

  BOOST_CHECK_THROW(vita::any_cast<char &>(a), vita::bad_any_cast);
  BOOST_CHECK_THROW(vita::any_cast<const char &>(b), vita::bad_any_cast);
}

BOOST_AUTO_TEST_CASE(reset)
{
  std::string text("test message");
  vita::any value(text);

  BOOST_CHECK(value.has_value());

  value.reset();
  BOOST_CHECK(!value.has_value());

  value.reset();
  BOOST_CHECK(!value.has_value());  // non-empty after second clear

  value = text;
  BOOST_CHECK(value.has_value());

  value.reset();
  BOOST_CHECK(!value.has_value());
}

// Covers the case from Boost #9462
// (<https://svn.boost.org/trac/boost/ticket/9462>).
BOOST_AUTO_TEST_CASE(vectors)
{
  const std::size_t vs(100);
  auto make_vect([&]() { return vita::any(std::vector<int>(vs, 7)); });

  const std::vector<int> &vec(vita::any_cast<std::vector<int>>(make_vect()));

  BOOST_CHECK(vec.size() == vs);
  BOOST_CHECK(vec.back() == 7);
  BOOST_CHECK(vec.front() ==  7);
}

BOOST_AUTO_TEST_CASE(hash)
{
  const int v_int(1234);
  const double v_double(3.14159265);
  const std::string v_string("Hello world!");

  vita::any a_int(v_int);
  vita::any a_double(v_double);
  vita::any a_string(v_string);
  vita::any a_empty;

  BOOST_CHECK(std::hash<vita::any>()(a_int) == std::hash<int>()(v_int));
  BOOST_CHECK(std::hash<vita::any>()(a_double)
              == std::hash<double>()(v_double));
  BOOST_CHECK(std::hash<vita::any>()(a_string)
              == std::hash<std::string>()(v_string));
  BOOST_CHECK(std::hash<vita::any>()(a_empty) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
