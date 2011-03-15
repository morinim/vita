/**
 *
 *  \file problem.cc
 *
 *  \author Manlio Morini
 *  \date 2011/03/11
 *
 *  This file is part of VITA
 *
 */

#include "problem.h"
#include "search.h"

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
  void
  problem::clear()
  {
    _active_eva = 0;
    _evaluators.clear();
  }

  ///
  /// \return the active evaluator.
  ///
  evaluator *
  problem::get_evaluator()
  {
    return _active_eva;
  }

  ///
  /// param[in] eva a pointer to an evaluator.
  ///
  /// Add a new avaluator to the set. Evaluators are used to score individual's
  /// fitness.
  ///
  void
  problem::add_evaluator(evaluator *const eva)
  {
    _evaluators.push_back(eva);

    if (!_active_eva)
      _active_eva = eva;
  }

  void
  problem::delete_evaluators()
  {
    for (std::vector<evaluator *>::const_iterator i(_evaluators.begin()); 
         i != _evaluators.end();
         ++i)
      delete *i;
  }

  ///
  /// \param[in] i index of the evaluator that should be set as active.
  ///
  void
  problem::set_evaluator(unsigned i)
  {
    _active_eva = _evaluators[i];
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  problem::check() const
  {
    return env.check();
  }

}  // namespace vita
