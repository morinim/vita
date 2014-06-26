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

#include "kernel/ga/interpreter.h"
#include "kernel/ga/i_num_ga.h"

namespace vita
{
  ///
  /// \param[in] ind individual whose value we are interested in.
  /// \param[in] f a multivariable real function.
  ///
  interpreter<i_num_ga>::interpreter(const i_num_ga &ind, const function &f)
    : core_interpreter(), ind_(ind), f_(f)
  {
  }

  ///
  /// \return the output value of function \a f_ with arguments from \c this
  ///         \a individual.
  ///
  /// The output value is empty in case of infinite / NAN numbers (for
  /// uniformity with GP interpreters).
  ///
  any interpreter<i_num_ga>::run()
  {
    const auto sz(ind_.size());
    std::vector<decltype(gene::par)> v(sz);

    for (auto i(decltype(sz){0}); i < sz; ++i)
      v[i] = ind_[i].par;

    const auto f_v(f_(v));
    return std::isfinite(f_v) ? any(f_v) : any();
  }

  ///
  /// \param[in] i the index of the parameter we are interested in.
  /// \return the output value of the current terminal symbol.
  ///
  any interpreter<i_num_ga>::fetch_param(unsigned i)
  {
    const gene &g(ind_[i]);

    assert(g.sym->parametric());
    return any(g.par);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter<i_num_ga>::debug() const
  {
    return ind_.debug();
  }
}  // namespace vita
