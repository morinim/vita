/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_PRIMITIVE_H)
#define      VITA_GA_PRIMITIVE_H

#include <string>

#include "kernel/function.h"
#include "kernel/ga/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"
#include "kernel/utility.h"

namespace vita
{
  /// We assume that errors during floating-point operations aren't terminal
  /// error. So we dont't try to prevent domain errors (e.g. square root of a
  /// negative number) or range error (e.g. pow(10.0, 1e6)) checking arguments
  /// beforehand (domain errors could be prevented by carefully bounds checking
  /// the arguments before calling functions and taking alternative action if
  /// the bounds are violated; range errors usually can not be prevented, as
  /// they are dependent on the implementation of floating-point numbers, as
  /// well as the function being applied).
  /// Instead we detect them and take alternative action (usually returning
  /// an empty \c any()).
  namespace ga
  {
    typedef decltype(gene::par) base_t;

    ///
    /// \param[in] v the value that must be casted to base type (\c base_t).
    /// \return the content of \a v.
    ///
    /// Just a simple shortcut.
    ///
    inline base_t cast(const any &v) { return any_cast<base_t>(v); }

    class real : public terminal
    {
    public:
      explicit real(const cvect &c, base_t m = -1000.0, base_t u = 1000.0)
        : terminal("REAL", c[0]), min(m), upp(u)
      {
        assert(c.size() == 1);
        assert(m < u);
        parametric_ = true;
      }

      virtual double init() const override
      { return random::between<base_t>(min, upp); }

      virtual std::string display(double v) const override
      { return std::to_string(v); }

      virtual any eval(core_interpreter *i) const override
      {
        return any(any_cast<base_t>(
                     static_cast<interpreter<i_num_ga> *>(i)->fetch_param(
                       category())));
      }

    private:  // Private data members
      const base_t min, upp;
    };

    inline std::unique_ptr<symbol> parameter(unsigned i, base_t m = -1000.0,
                                             base_t u = 1000.0)
    {
      return make_unique<real>(cvect{i}, m, u);
    }
  }  // namespace ga
}  // namespace vita

#endif  // Include guard
