/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017, 2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_INDIVIDUAL_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_INDIVIDUAL_TCC)
#define      VITA_INDIVIDUAL_TCC

///
/// A measurement of the age of an individual (mainly used for ALPS).
///
/// \return The individual's age
///
/// This is a measure of how long an individual's family of genotypic
/// material has been in the population. Randomly generated individuals,
/// such as those that are created when the search algorithm are started,
/// start with an age of `0`. Each generation that an individual stays in the
/// population (such as through elitism) its age is increased by `1`.
/// **Individuals that are created through mutation or recombination take the
/// age of their oldest parent**.
///
/// \note
/// This differs from conventional measures of age, in which individuals
/// created through applying some type of variation to an existing
/// individual (e.g. mutation or recombination) start with an age of `0`.
template<class Derived>
inline unsigned individual<Derived>::age() const
{
  return age_;
}

///
/// Increments the individual's age.
///
template<class Derived>
inline void individual<Derived>::inc_age()
{
  ++age_;
}

///
/// \param[in] ss active symbol set
/// \param[in] in input stream
/// \return       `true` if the object has been loaded correctly
///
/// \note If the load operation isn't successful the object isn't modified.
///
template<class Derived>
bool individual<Derived>::load(std::istream &in, const symbol_set &ss)
{
  decltype(age()) t_age;
  if (!(in >> t_age))
    return false;

  if (!static_cast<Derived *>(this)->load_impl(in, ss))
    return false;

  age_ = t_age;

  // We don't save/load signature: it can be easily calculated on the fly.
  signature_.clear();

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if the object has been saved correctly
///
template<class Derived>
bool individual<Derived>::save(std::ostream &out) const
{
  out << age() << '\n';

  return static_cast<const Derived *>(this)->save_impl(out);
}

///
/// Updates the age of this individual if it's smaller than `rhs_age`.
///
/// \param[in] rhs_age the age of an individual
///
template<class Derived>
void individual<Derived>::set_older_age(unsigned rhs_age)
{
  if (age() < rhs_age)
    age_ = rhs_age;
}

#endif  // include guard
