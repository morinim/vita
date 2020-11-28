/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INDIVIDUAL_H)
#define      VITA_INDIVIDUAL_H

#include <fstream>

#include "kernel/cache.h"
#include "kernel/environment.h"
#include "kernel/gp/locus.h"
#include "kernel/problem.h"
#include "kernel/vitafwd.h"

namespace vita
{
///
/// A single member of a `population`.
///
/// \tparam Derived the derived class in the CRTP
///
/// Each individual contains a genome which represents a possible solution to
/// the task being tackled (i.e. a point in the search space).
///
/// This class is the (nondependent CRTP) base class of every type of
/// individual and factorizes out common code / data members.
///
/// \note AKA chromosome.
///
template<class Derived>
class individual
{
public:
  unsigned age() const;
  void inc_age();

  // Serialization.
  bool load(std::istream &, const symbol_set & = symbol_set());
  bool save(std::ostream &) const;

protected:
  // Protected to prevent individual<Derived> from being instantiated as a non
  // base class.
  individual() = default;
  ~individual() = default;

  void set_older_age(unsigned);

  // Note that syntactically distinct (but logically equivalent) individuals
  // have the same signature. This is a very interesting  property, useful
  // for individual comparison, information retrieval, entropy calculation...
  mutable hash_t signature_;

private:
  unsigned age_;
};  // class individual

/// The SFINAE way of recognizing if an individual has introns.
/// In general we assume it hasn't. Specific class can specialize the
/// `has_introns` struct to signal their presence.
template<class T> struct has_introns : std::false_type {};

namespace out
{
bool long_form_flag(std::ostream &);
print_format_t print_format_flag(std::ostream &);

class print_format
{
public:
  explicit print_format(print_format_t t) : t_(t) {}

  friend std::ostream &operator<<(std::ostream &, print_format);

private:
  print_format_t t_;
};

std::ostream &c_language(std::ostream &);
std::ostream &cpp_language(std::ostream &);
std::ostream &dump(std::ostream &);
std::ostream &graphviz(std::ostream &);
std::ostream &in_line(std::ostream &);
std::ostream &list(std::ostream &);
std::ostream &mql_language(std::ostream &);
std::ostream &python_language(std::ostream &);
std::ostream &tree(std::ostream &);

std::ostream &long_form(std::ostream &);
std::ostream &short_form(std::ostream &);
}  // namespace out

#include "kernel/individual.tcc"

}  // namespace vita
#endif  // include guard
