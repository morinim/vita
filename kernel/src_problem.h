/**
 *
 *  \file src_problem.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(SRC_PROBLEM_H)
#define      SRC_PROBLEM_H

#include <cmath>
#include <string>
#include <vector>

#include "kernel/vita.h"
#include "kernel/data.h"
#include "kernel/problem.h"
#include "kernel/primitive/sr_pri.h"

namespace vita
{
  class src_problem : public problem
  {
  public:
    src_problem();
    ~src_problem();

    bool load_data(const std::string &);
    std::string load_symbols(const std::string &);

    void clear();

    unsigned classes() const;
    unsigned variables() const;

    bool check() const;

  private:
    std::vector<vita::sr::variable *> vars_;
    data dat_;
  };
}  // namespace vita

#endif  // SRC_PROBLEM_H
