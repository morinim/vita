/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2019 EOS di Manlio Morini.
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

/// Data/simulations are categorised in three sets:
/// - *training* used directly for learning;
/// - *validation* for controlling overfitting and measuring the performance
///   of an individual;
/// - *test* for a forecast of how well an individual will do in the real
///   world.
/// The `vita::search` class asks the `problem` class to setup the requested
/// simulation/dataset via the `select` function.
enum class dataset_t {training = 0, validation, test};

///
/// Provides a GP-specific interface to the generic `problem` class.
///
/// The class is a facade that provides a simpler interface to represent
/// symbolic regression / classification tasks.
///
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

  const dataframe &data(dataset_t  = dataset_t::training) const;
  dataframe &data(dataset_t = dataset_t::training);

  /// Just a shorthand for checking number of classes.
  bool classification() const { return classes() > 1; }

  unsigned categories() const;
  unsigned classes() const;
  unsigned variables() const;

  bool debug() const override;

private:
  // Private support methods.
  bool compatible(const cvect &, const std::vector<std::string> &) const;
  std::size_t setup_symbols_impl();
  std::size_t setup_symbols_impl(const std::string &);

  // Private data members.
  dataframe training_;
  dataframe validation_;
  symbol_factory factory_;
};

}  // namespace vita

#endif  // include guard
