/**
 *
 *  \file fitness_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(FITNESS_INL_H)
#define      FITNESS_INL_H

///
///
///
template<class T, unsigned N>
basic_fitness_t<T, N>::basic_fitness_t(T v)
{
  static_assert(N, "basic_fitness_t cannot have zero length");

  vect.fill(v);
}

/*
/// This constructor is slower than the following one (based on variadic
/// templates).
/// The problem is the copy at runtime.
template<class T, unsigned N>
basic_fitness_t<T, N>::basic_fitness_t(const std::initializer_list<base_t> &l)
{
  static_assert(N, "basic_fitness_t cannot have zero length");

  unsigned i(0);
  for (auto p : l)
    vect[i++] = p;
}
*/

///
///
///
template<class T, unsigned N>
template<class ...Args>
basic_fitness_t<T, N>::basic_fitness_t(Args ...args) : vect{{T(args)...}}
{
  static_assert(N, "basic_fitness_t cannot have zero length");
  static_assert(sizeof...(Args) == N, "Wrong number of arguments");
}

///
/// Operation is performed by first comparing sizes and, if they match,
/// the elements are compared sequentially using algorithm equal, which
/// stops at the first mismatch.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator==(const basic_fitness_t<T, N> &f) const
{
  return vect == f.vect;
}

/// Operation is performed by first comparing sizes and, if they match,
/// the elements are compared sequentially using algorithm equal, which
/// stops at the first mismatch.
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator!=(const basic_fitness_t<T, N> &f) const
{
  return vect != f.vect;
}

/// Behaves as if using algorithm lexicographical_compare, which compares
/// the elements sequentially, stopping at the first mismatch.
///
/// \note
/// A lexicographical comparison is the kind of comparison generally used
/// to sort words alphabetically in dictionaries; it involves comparing
/// sequentially the elements that have the same position in both ranges
/// against each other until one element is not equivalent to the other.
/// The result of comparing these first non-matching elements is the result
/// of the lexicographical comparison.
/// If both sequences compare equal until one of them ends, the shorter
/// sequence is lexicographically less than the longer one.
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator>(const basic_fitness_t<T, N> &f) const
{
  return vect > f.vect;

  // An alternative implementation:
  // > for (size_t i(0); i < N; ++i)
  // >   if (vect[i] != f.vect[i])
  // >     return vect[i] > f.vect[i];
  // > return false;
}

/// Lexicographic ordering.
/// \see basic_fitness_t::operator>
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator>=(const basic_fitness_t<T, N> &f) const
{
  return vect >= f.vect;
}

/// Lexicographic ordering.
/// \see basic_fitness_t::operator>
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator<(const basic_fitness_t<T, N> &f) const
{
  return vect < f.vect;
}

/// Lexicographic ordering.
/// \see basic_fitness_t::operator>
template<class T, unsigned N>
bool basic_fitness_t<T, N>::operator<=(const basic_fitness_t<T, N> &f) const
{
  return vect <= f.vect;
}

///
/// \param[in] f second term of comparison.
/// \return \c true if \a this is a Pareto improvement of \a f.
///
/// \a this dominates \a f (is a Pareto improvement) if:
/// * each component of \a this is not strictly worst (less) than the
///   correspondig component of \a f;
/// * there is at least one component in which \a this is better than \a f.
///
/// \note
/// An interesting property is that if a vector x does not dominate a
/// vector y, this does not imply that y dominates x (for example they can
/// be both non-dominated).
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::dominating(const basic_fitness_t<T, N> &f) const
{
  bool one_better(false);

  for (size_t i(0); i < N; ++i)
    if (vect[i] > f.vect[i])
      one_better = true;
    else if (vect[i] < f.vect[i])
      return false;

  return one_better;
}

///
/// \param[in] in input stream.
/// \return \c true if basic_fitness_t loaded correctly.
///
/// \note
/// If the load operation isn't successful the current basic_fitness_t isn't
/// changed.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::load(std::istream &in)
{
  basic_fitness_t<T, N> tmp;

  for (size_t i(0); i < N; ++i)
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
/// \return \c true if basic_fitness_t was saved correctly.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::save(std::ostream &out) const
{
  for (const auto &i : vect)
    out << std::fixed << std::scientific
        << std::setprecision(std::numeric_limits<double>::digits10 + 1)
        << i << ' ';

  out << std::endl;

  return out.good();
}

///
/// Standard output operator for basic_fitness_t.
///
template<class T, unsigned N>
std::ostream &operator<<(std::ostream &o, const basic_fitness_t<T, N> &f)
{
  o << '(';

  for (size_t i(0); i < N; ++i)
  {
    o << f[i];
    if (i + 1 < N)
      o << ", ";
  }

  return o << ')';
}

///
/// \param[in] f a fitness.
/// \return the sum of \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> &basic_fitness_t<T, N>::operator+=(const basic_fitness_t<T, N> &f)
{
  for (size_t i(0); i < N; ++i)
    vect[i] += f[i];

  return *this;
}

///
/// \param[in] f a fitness.
/// \return the difference of \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> &basic_fitness_t<T, N>::operator-=(const basic_fitness_t<T, N> &f)
{
  for (size_t i(0); i < N; ++i)
    vect[i] -= f[i];

  return *this;
}

///
/// \param[in] f a fitness.
/// \return the difference between \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator-(const basic_fitness_t<T, N> &f) const
{
  basic_fitness_t<T, N> tmp;
  for (size_t i(0); i < N; ++i)
    tmp[i] = vect[i] - f[i];

  return tmp;
}

///
/// \param[in] f a fitness.
/// \return the product of \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator*(const basic_fitness_t<T, N> &f) const
{
  basic_fitness_t<T, N> tmp;
  for (size_t i(0); i < N; ++i)
    tmp[i] = vect[i] * f[i];

  return tmp;
}

///
/// \param[in] val a scalar.
/// \return a new vector obtained dividing each component of \a this by tha
///         scalar value \a val.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator/(T val) const
{
  basic_fitness_t<T, N> tmp;
  for (size_t i(0); i < N; ++i)
    tmp[i] = vect[i] / val;

  return tmp;
}

///
/// \param[in] val a scalar.
/// \return the product of \a this and \a val.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator*(T val) const
{
  basic_fitness_t<T, N> tmp;
  for (size_t i(0); i < N; ++i)
    tmp[i] = vect[i] * val;

  return tmp;
}

///
/// \return a new vector obtained taking the absolute value of each component
///         of \a this.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::abs() const
{
  basic_fitness_t<T, N> tmp;
  for (unsigned i(0); i < N; ++i)
    tmp[i] = std::fabs(vect[i]);

  return tmp;
}

///
/// \return a new vector obtained taking the square root of each component of
///         \a this.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::sqrt() const
{
  basic_fitness_t<T, N> tmp;
  for (size_t i(0); i < N; ++i)
    tmp[i] = std::sqrt(vect[i]);

  return tmp;
}

///
/// \return \c true if every component of the fitness is finite.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::isfinite() const
{
  for (const auto &i : vect)
    if (!std::isfinite(i))
      return false;
  return true;
}

///
/// \return \c true if a component of the fitness is NAN.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::isnan() const
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
template<class T, unsigned N>
bool basic_fitness_t<T, N>::issmall() const
{
  for (const auto &i : vect)
    if (i > float_epsilon)
      return false;
  return true;
}
#endif  // FITNESS_INL_H
