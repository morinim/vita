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
#include "kernel/ga/i_ga.h"

namespace vita
{
  ga_function interpreter<i_ga>::function = nullptr;

  ///
  /// \param[in] ind individual whose value we are interested in.
  ///
  interpreter<i_ga>::interpreter(const i_ga &ind) : core_interpreter(),
                                                    ind_(ind)
  {
  }

  ///
  /// \return the output value of \a function with arguments from \c this
  ///         \a individual.
  ///
  /// The output value is empty in case of infinite / NAN numbers (for
  /// uniformity with GP interpreters).
  ///
  any interpreter<i_ga>::run()
  {
    assert(function);

    const auto ps(ind_.parameters());
    std::vector<gene::param_type> v(ps);

    for (auto i(decltype(ps){0}); i < ps; ++i)
      v[i] = ind_[i];

    const auto f_v(function(v));
    return std::isfinite(f_v) ? any(f_v) : any();
  }

  ///
  /// \param[in] i the index of the parameter we are interested in.
  /// \return the output value of the current terminal symbol.
  ///
  gene::param_type interpreter<i_ga>::fetch_param(unsigned i)
  {
    return ind_[i];
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool interpreter<i_ga>::debug() const
  {
    return ind_.debug();
  }
}  // namespace vita
