/**
 *
 *  \file problem.h
 *
 *  \author Manlio Morini
 *  \date 2011/03/14
 *
 *  This file is part of VITA
 *
 */

#if !defined(PROBLEM_H)
#define      PROBLEM_H

#include <vector>

#include "vita.h"
#include "environment.h"
#include "evaluator.h"

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
    std::vector<evaluator *> _evaluators;
    vita::evaluator *_active_eva;
  };

}  // namespace vita

#endif  // PROBLEM_H
