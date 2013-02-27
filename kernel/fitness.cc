/**
 *
 *  \file fitness.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <iomanip>

#include "fitness.h"

namespace vita
{
  ///
  /// \param[in] in input stream.
  /// \return \c true if fitness_t loaded correctly.
  ///
  /// \note
  /// If the load operation isn't successful the current fitness_t isn't
  /// changed.
  ///
  bool fitness_t::load(std::istream &in)
  {
    const size_t sup(size());

    fitness_t tmp(sup);

    for (size_t i(0); i < sup; ++i)
      if (!(in
            >> std::fixed >> std::scientific
            >> std::setprecision(std::numeric_limits<double>::digits10 + 1)
            >> tmp.vect[i]))
        return false;

    *this = tmp;

    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if fitness_t was saved correctly.
  ///
  bool fitness_t::save(std::ostream &out) const
  {
    for (const auto &i : vect)
      out << std::fixed << std::scientific
          << std::setprecision(std::numeric_limits<double>::digits10 + 1)
          << i << ' ';

    out << std::endl;

    return out.good();
  }

  ///
  /// Standard output operator for fitness_t.
  ///
  std::ostream &operator<<(std::ostream &o, const fitness_t &f)
  {
    o << '(';

    for (size_t i(0); i < f.size(); ++i)
    {
      o << f[i];
      if (i + 1 < f.size())
        o << ", ";
    }

    return o << ')';
  }
}  // namespace vita
