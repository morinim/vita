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

#include "kernel/problem.h"
#include "kernel/src/dataframe.h"
#include "kernel/src/primitive/factory.h"

namespace vita
{
class src_problem : public problem
{
public:
  explicit src_problem(initialization = initialization::skip);
  explicit src_problem(const char []);
  explicit src_problem(const std::string &, const std::string & = "");

  bool operator!() const;
  std::pair<std::size_t, std::size_t> read(const dataframe &);
  std::pair<std::size_t, std::size_t> read(const std::string &,
                                           const std::string & = "");
  std::size_t read_symbols(const std::string &);
  bool setup_default_symbols();

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
  std::size_t setup_terminals_from_data(const std::set<unsigned> & = {});
  void select_impl(dataset_t) override final {};

  // Private data members.
  dataframe training_;
  dataframe validation_;
  symbol_factory factory_;
};

}  // namespace vita

#endif  // include guard
