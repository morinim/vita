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

#include "kernel/environment.h"
#include "kernel/fitness.h"

namespace vita
{
  class evaluator;

  typedef std::shared_ptr<evaluator> evaluator_ptr;

  class problem
  {
  public:
    explicit problem(fitness_t);

    void add_evaluator(evaluator_ptr);
    evaluator *get_evaluator();
    void set_evaluator(unsigned);

    virtual void clear();
    virtual bool check() const;

    environment env;

    fitness_t threashold;

  private:
    std::vector<evaluator_ptr> evaluators_;
    evaluator_ptr              active_eva_;
  };
}  // namespace vita

#endif  // PROBLEM_H
