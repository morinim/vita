/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
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

#include "kernel/data.h"
#include "kernel/problem.h"

namespace vita
{
  class src_problem : public problem<>
  {
  public:
    src_problem();
    explicit src_problem(const std::string &, const std::string & = "",
                         const std::string & = "");

    bool operator!() const;
    std::pair<std::size_t, unsigned> load(const std::string &,
                                          const std::string & = "",
                                          const std::string & = "");
    unsigned load_symbols(const std::string &);
    std::size_t load_test_set(const std::string &);
    void setup_default_symbols();

    virtual vita::data *data() override { return &dat_; }

    virtual void clear(bool) override;

    /// Just a shortcut for checking number of classes.
    bool classification() { return classes() > 1; }

    unsigned categories() const;
    unsigned classes() const;
    unsigned variables() const;

    virtual bool debug(bool) const override;

  private:  // Private support methods
    bool compatible(const cvect &, const std::vector<std::string> &) const;
    void setup_terminals_from_data(const std::set<unsigned> & = {});

  private:  // Private data members
    vita::data dat_;
  };
}  // namespace vita

#endif  // Include guard
