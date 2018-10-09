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

#if !defined(VITA_SRC_PROBLEM_H)
#define      VITA_SRC_PROBLEM_H

#include <string>
#include <set>

#include "kernel/exceptions.h"
#include "kernel/problem.h"
#include "kernel/src/dataframe.h"
#include "kernel/src/primitive/factory.h"

namespace vita
{

class src_problem : public problem
{
public:
  struct default_symbols_t {};
  static const default_symbols_t default_symbols;

  explicit src_problem();
  src_problem(const std::string &, const default_symbols_t &);
  src_problem(const std::string &, const std::string &);

  bool operator!() const;
  std::size_t setup_symbols(const std::string & = "");
  std::size_t setup_terminals(const std::set<unsigned> & = {});

  const dataframe &data() const;
  dataframe &data();
  const dataframe &data(dataset_t) const;
  dataframe &data(dataset_t);

  /// Just a shorthand for checking number of classes.
  bool classification() const { return classes() > 1; }

  unsigned categories() const;
  unsigned classes() const;
  unsigned variables() const;

  bool has(dataset_t) const override;

  bool debug() const override;

private:
  // Private support methods.
  bool compatible(const cvect &, const std::vector<std::string> &) const;
  void select_impl(dataset_t) override final {};
  std::size_t setup_symbols_impl();
  std::size_t setup_symbols_impl(const std::string &);

  // Private data members.
  dataframe training_;
  dataframe validation_;
  symbol_factory factory_;
};

}  // namespace vita

#endif  // include guard
