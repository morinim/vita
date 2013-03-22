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
    size_t sup;
    if (!(in >> sup))
      return false;

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
    out << size() << ' ';
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

  ///
  /// \param[in] f a fitness.
  /// \return the sum of \a this and \a f.
  ///
  fitness_t fitness_t::operator+=(const fitness_t &f)
  {
    const size_t sup(size());

    for (size_t i(0); i < sup; ++i)
      vect[i] += f[i];

    return *this;
  }

  ///
  /// \param[in] f a fitness.
  /// \return the difference between \a this and \a f.
  ///
  fitness_t fitness_t::operator-(const fitness_t &f) const
  {
    const size_t sup(size());
    fitness_t tmp(sup);
    for (size_t i(0); i < sup; ++i)
      tmp[i] = vect[i] - f[i];

    return tmp;
  }

  ///
  /// \param[in] f a fitness.
  /// \return the product of \a this and \a f.
  ///
  fitness_t fitness_t::operator*(const fitness_t &f) const
  {
    const size_t sup(size());
    fitness_t tmp(sup);
    for (size_t i(0); i < sup; ++i)
      tmp[i] = vect[i] * f[i];

    return tmp;
  }

  ///
  /// \param[in] val a scalar.
  /// \return a new vector obtained dividing each component of \a this by tha
  ///         scalar value \a val.
  ///
  fitness_t fitness_t::operator/(fitness_t::base_t val) const
  {
    const size_t sup(size());
    fitness_t tmp(sup);
    for (size_t i(0); i < sup; ++i)
      tmp[i] = vect[i] / val;

    return tmp;
  }

  ///
  /// \return a new vector obtained taking the square root of each component of
  ///         \a this.
  ///
  fitness_t fitness_t::sqrt() const
  {
    const size_t sup(size());
    fitness_t tmp(sup);
    for (size_t i(0); i < sup; ++i)
      tmp[i] = std::sqrt(vect[i]);

    return tmp;
  }

  ///
  /// \return \c true if every component of the fitness is finite.
  ///
  bool fitness_t::isfinite() const
  {
    for (const auto &i : vect)
      if (!std::isfinite(i))
        return false;
    return true;
  }

  ///
  /// \return \c true if a component of the fitness is NAN.
  ///
  bool fitness_t::isnan() const
  {
    for (const auto &i : vect)
      if (std::isnan(i))
        return true;
    return false;
  }

  ///
  /// \return \c true if each component of the fitness is less than or equal to
  ///         \c float_epsilion.
  ///
  bool fitness_t::issmall() const
  {
    for (const auto &i : vect)
      if (i > float_epsilon)
        return false;
    return true;
  }
}  // namespace vita
