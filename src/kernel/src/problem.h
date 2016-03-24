/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2016 EOS di Manlio Morini.
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
#include "kernel/src/data.h"
#include "kernel/src/primitive/factory.h"

namespace vita
{
class src_problem : public problem
{
public:
  explicit src_problem(bool = false);
  explicit src_problem(const char []);
  explicit src_problem(const std::string &, const std::string & = "",
                       const std::string & = "");

  bool operator!() const;
  std::pair<std::size_t, std::size_t> load(const std::string &,
                                           const std::string & = "",
                                           const std::string & = "");
  unsigned load_symbols(const std::string &);
  std::size_t load_test_set(const std::string &);
  void setup_default_symbols();

  vita::src_data &data() { return dat_; }

  virtual void clear(bool) override;

  /// Just a shorthand for checking number of classes.
  bool classification() const { return classes() > 1; }

  unsigned categories() const;
  unsigned classes() const;
  unsigned variables() const;

  virtual bool debug() const override;

private:
  // Private support methods.
  bool compatible(const cvect &, const std::vector<std::string> &) const;
  void setup_terminals_from_data(const std::set<unsigned> & = {});

  // Private data members.
  vita::src_data dat_;
  vita::symbol_factory factory_;
};

}  // namespace vita

#endif  // include guard
