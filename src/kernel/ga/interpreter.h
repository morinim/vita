/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_INTERPRETER_H)
#define      VITA_GA_INTERPRETER_H

#include "kernel/core_interpreter.h"
#include "kernel/vitafwd.h"
#include "kernel/ga/i_ga.h"

namespace vita
{
///
/// \brief A template specialization of the interpreter class for i_ga
///        individuals
///
/// \note
/// The run() method is useless for this specialization, there is nothing
/// to execute: a i_ga object is just a point in a multidimensional space.
///
template<>
class interpreter<i_ga> : public core_interpreter
{
public:
  explicit interpreter(const i_ga *);

  double fetch_param(unsigned) const;

private:  // Methods of the non-virtual interface
  virtual any run_nvi() override;
  virtual double penalty_nvi() override;
  virtual bool debug_nvi() const override;

private:  // Private data members
  const i_ga &p_;
};

///
/// Just a shortcut for interpreter<i_ga>::penalty()
///
inline double base_penalty(const i_ga &ind)
{
  return interpreter<i_ga>(&ind).penalty();
}
}  // namespace vita

#endif  // Include guard
