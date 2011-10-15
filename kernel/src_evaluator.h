/**
 *
 *  \file src_evaluator.h
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

#if !defined(SRC_EVALUATOR_H)
#define      SRC_EVALUATOR_H

#include <vector>

#include "kernel/data.h"
#include "kernel/evaluator.h"
#include "kernel/primitive/sr_pri.h"

namespace vita
{
  class src_evaluator : public evaluator
  {
  public:
    src_evaluator(data *, std::vector<variable_ptr> *);

    void load_vars(const data::value_type &);

  protected:
    data                      *dat_;
    std::vector<variable_ptr> *var_;
  };

  class abs_evaluator : public src_evaluator
  {
  public:
    abs_evaluator(data *d, std::vector<variable_ptr> *v)
      : src_evaluator(d, v) {}

    fitness_t operator()(const individual &);
    fitness_t fast(const individual &);
  };

  ///
  /// Slotted Dynamic Class Boundary Determination
  /// (<http://www.mcs.vuw.ac.nz/comp/Publications/CS-TR-04-2.abs.html>).
  ///
  class dyn_slot_evaluator : public src_evaluator
  {
  public:
    dyn_slot_evaluator(data *d, std::vector<variable_ptr> *v)
      : src_evaluator(d, v)
    {
      assert(d);
      assert(v);
    }

    fitness_t operator()(const individual &);
    double accuracy(const individual &);

  private:
    static double normalize_01(double);

    void fill_slots(const individual &,
                    std::vector <std::vector<unsigned>> *,
                    std::vector<unsigned> *);
  };

  class gaussian_evaluator : public src_evaluator
  {
  public:
    gaussian_evaluator(data *d, std::vector<variable_ptr> *v)
      : src_evaluator(d, v)
    {
      assert(d);
      assert(v);
    }

    unsigned class_label(const individual &, const data::value_type &,
                         const std::vector< distribution<double> > &);

    fitness_t operator()(const individual &);
    double accuracy(const individual &);

  private:
    void gaussian_distribution(const individual &,
                               std::vector< distribution<double> > *);
  };
}  // namespace vita

#endif  // SRC_EVALUATOR_H
