/**
 *
 *  \file problem.cc
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

#include "kernel/problem.h"
#include "kernel/search.h"

namespace vita
{
  ///
  /// New empty instance.
  ///
  problem::problem()
  {
    clear();
  }

  ///
  /// Resets the object.
  ///
  void problem::clear()
  {
    active_eva_ = 0;
    evaluators_.clear();
  }

  ///
  /// \return the active evaluator.
  ///
  evaluator *problem::get_evaluator()
  {
    return active_eva_;
  }

  ///
  /// param[in] eva a pointer to an evaluator.
  ///
  /// Add a new avaluator to the set. Evaluators are used to score individual's
  /// fitness.
  ///
  void problem::add_evaluator(evaluator *const eva)
  {
    evaluators_.push_back(eva);

    if (!active_eva_)
      active_eva_ = eva;
  }

  ///
  /// The basic rule of memory management in Vita is that any object that is
  /// created by the client must be deleted by the client... but, you know,
  /// every rule has an exception: subclasses of \a problem need to free
  /// memory in their destructors and the deallocation process is "factored"
  /// in the \a delete_evaluators member function.
  ///
  void problem::delete_evaluators()
  {
    for (std::vector<evaluator *>::const_iterator i(evaluators_.begin());
         i != evaluators_.end();
         ++i)
      delete *i;
  }

  ///
  /// \param[in] i index of the evaluator that should be set as active.
  ///
  void problem::set_evaluator(unsigned i)
  {
    active_eva_ = evaluators_[i];
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool problem::check() const
  {
    return env.check();
  }
}  // namespace vita
