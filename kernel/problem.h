/**
 *
 *  \file problem.h
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

#if !defined(PROBLEM_H)
#define      PROBLEM_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/environment.h"
#include "kernel/evaluator.h"

namespace vita
{
  class problem
  {
  public:
    problem();

    void add_evaluator(evaluator *const);
    evaluator *get_evaluator();
    void set_evaluator(unsigned);

    virtual void clear();
    virtual bool check() const;

    environment env;

  protected:
    void delete_evaluators();

  private:
    std::vector<evaluator *> evaluators_;
    vita::evaluator         *active_eva_;
  };
}  // namespace vita

#endif  // PROBLEM_H
