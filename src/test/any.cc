/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

namespace std
{
std::istream &operator>>(std::istream &i, std::vector<int> &)
{
  return i;
}

std::ostream &operator<<(std::ostream &o, const std::vector<int> &)
{
  return o;
}
}

TEST_SUITE("ANY")
{

TEST_CASE("Default constructor")
{
  const vita::any value;

  CHECK(!value.has_value());
  CHECK(vita::any_cast<int>(&value) == nullptr);
  CHECK(value.type() == typeid(void));
}

TEST_CASE("Converting constructor")
{
  std::string text("test message");

  vita::any value(text);

  CHECK(value.has_value());
  CHECK(value.type() == typeid(std::string));
  CHECK(vita::any_cast<int>(&value) == nullptr);
  CHECK(vita::any_cast<std::string>(&value));
  CHECK(vita::any_cast<std::string>(value) == text);
  CHECK(vita::any_cast<std::string>(&value) != &text);
}

TEST_CASE("Copy constructor")
{
  std::string text("test message");
  vita::any original(text), copy(original);

  using vita::any_cast;

  CHECK(copy.has_value());
  CHECK(original.type() == copy.type());
  CHECK(any_cast<std::string>(original) == any_cast<std::string>(copy));
  CHECK(text == any_cast<std::string>(copy));
  CHECK(any_cast<std::string>(&original) != any_cast<std::string>(&copy));
}

TEST_CASE("Copy assign")
{
  std::string text("test message");
  vita::any original(text), copy;
  vita::any *assign_result = &(copy = original);

  using vita::any_cast;

  CHECK(copy.has_value());
  CHECK(original.type() == copy.type());
  CHECK(any_cast<std::string>(original) == any_cast<std::string>(copy));
  CHECK(text == any_cast<std::string>(copy));
  CHECK(any_cast<std::string>(&original) != any_cast<std::string>(&copy));
  CHECK(assign_result == &copy);
}

TEST_CASE("Bad cast")
{
  std::string text("test message");
  vita::any value(text);

  CHECK_THROWS_AS(vita::any_cast<char *>(value), vita::bad_any_cast);
}

TEST_CASE("Swap")
{
  std::string text("test message");
  vita::any original(text), swapped;
  std::string *original_ptr(vita::any_cast<std::string>(&original));
  vita::any *swap_result(&original.swap(swapped));

  CHECK(!original.has_value());
  CHECK(swapped.has_value());
  CHECK(swapped.type() == typeid(std::string));
  CHECK(text == vita::any_cast<std::string>(swapped));
  CHECK(original_ptr);
  CHECK(swap_result == &original);
}

TEST_CASE("Null copying")
{
  const vita::any null;
  vita::any copied = null, assigned;
  assigned = null;

  CHECK(!null.has_value());
  CHECK(!copied.has_value());
  CHECK(!assigned.has_value());
}

TEST_CASE("Cast to reference")
{
  vita::any a(137);
  const vita::any b(a);

  int                &ra(   vita::any_cast<               int &>(a));
  const int          &ra_c( vita::any_cast<         const int &>(a));
  volatile int       &ra_v( vita::any_cast<      volatile int &>(a));
  volatile const int &ra_cv(vita::any_cast<volatile const int &>(a));

  CHECK(&ra == &ra_c);
  CHECK(&ra == &ra_v);
  CHECK(&ra == &ra_cv);

  const int          &rb_c( vita::any_cast<         const int &>(b));
  volatile const int &rb_cv(vita::any_cast<volatile const int &>(b));

  CHECK(&rb_c == &rb_cv);
  CHECK(&ra != &rb_c);

  ++ra;
  int incremented(vita::any_cast<int>(a));
  CHECK(incremented == 138);

  CHECK_THROWS_AS(vita::any_cast<char &>(a), vita::bad_any_cast);
  CHECK_THROWS_AS(vita::any_cast<const char &>(b), vita::bad_any_cast);
}

TEST_CASE("Reset")
{
  std::string text("test message");
  vita::any value(text);

  CHECK(value.has_value());

  value.reset();
  CHECK(!value.has_value());

  value.reset();
  CHECK(!value.has_value());  // non-empty after second clear

  value = text;
  CHECK(value.has_value());

  value.reset();
  CHECK(!value.has_value());
}

// Covers the case from Boost #9462
// (<https://svn.boost.org/trac/boost/ticket/9462>).
TEST_CASE("Vectors")
{
  const std::size_t vs(100);
  auto make_vect([&]() { return vita::any(std::vector<int>(vs, 7)); });

  const std::vector<int> &vec(vita::any_cast<std::vector<int>>(make_vect()));

  CHECK(vec.size() == vs);
  CHECK(vec.back() == 7);
  CHECK(vec.front() ==  7);
}

}  // TEST_SUITE("ANY")
