/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_PROBLEM_H)
#define      VITA_SRC_PROBLEM_H

#include <filesystem>
#include <string>

#include "kernel/exceptions.h"
#include "kernel/problem.h"
#include "kernel/gp/src/category_set.h"
#include "kernel/gp/src/primitive/factory.h"

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
  // --- Constructors ---
  src_problem();

  explicit src_problem(const std::filesystem::path &, typing = typing::weak);
  explicit src_problem(std::istream &, typing = typing::weak);

  struct default_symbols_t {};
  static const default_symbols_t default_symbols;
  src_problem(const std::filesystem::path &, const default_symbols_t &,
              typing = typing::weak);
  src_problem(const std::filesystem::path &, const std::filesystem::path &,
              typing = typing::weak);
  // --------------------

  bool operator!() const;
  std::size_t setup_symbols(typing = typing::weak);
  std::size_t setup_symbols(const std::filesystem::path &,
                            typing = typing::weak);
  void setup_terminals(typing);

  const dataframe &data(dataset_t = dataset_t::training) const;
  dataframe &data(dataset_t = dataset_t::training);

  /// Just a shorthand for checking number of classes.
  bool classification() const { return classes() > 1; }

  unsigned categories() const;
  unsigned classes() const;
  unsigned variables() const;

  bool is_valid() const override;

private:
  // Private support methods.
  bool compatible(const cvect &, const std::vector<std::string> &,
                  const category_set &) const;
  std::size_t setup_symbols_impl();
  std::size_t setup_symbols_impl(const std::filesystem::path &);

  // Private data members.
  dataframe training_;
  dataframe validation_;
  symbol_factory factory_;
};

}  // namespace vita

#endif  // include guard
