/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_FITNESS_INL_H)
#define      VITA_FITNESS_INL_H

template<class T, unsigned N>
constexpr decltype(N) basic_fitness_t<T, N>::size;

///
/// Fills the fitness with value \a v.
///
template<class T, unsigned N>
basic_fitness_t<T, N>::basic_fitness_t(T v) : vect(make_array<T, N>(v))
{
  static_assert(N, "basic_fitness_t cannot have zero length");
}

///
/// Builds a fitness from a list of values.
///
template<class T, unsigned N>
template<class ...Args>
basic_fitness_t<T, N>::basic_fitness_t(Args ...args) : vect{{T(args)...}}
{
  static_assert(N, "basic_fitness_t cannot have zero length");
  static_assert(sizeof...(Args) == N, "Wrong number of arguments");

  // Do not change with something like:
  //
  // template<class T, unsigned N>
  // basic_fitness_t(const std::initializer_list<base_t> &l)
  // {
  //   unsigned i(0);
  //   for (auto p : l)
  //     vect[i++] = p;
  // }
  //
  // This is slower because of the runtime copy.
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
  // > for (decltype(N) i(0); i < N; ++i)
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

  for (decltype(N) i(0); i < N; ++i)
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

  for (decltype(N) i(0); i < N; ++i)
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

  for (decltype(N) i(0); i < N; ++i)
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
basic_fitness_t<T, N> &basic_fitness_t<T, N>::operator+=(
  const basic_fitness_t<T, N> &f)
{
  for (decltype(N) i(0); i < N; ++i)
    vect[i] += f[i];

  return *this;
}

///
/// \param[in] f a fitness.
/// \return the difference of \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> &basic_fitness_t<T, N>::operator-=(
  const basic_fitness_t<T, N> &f)
{
  for (decltype(N) i(0); i < N; ++i)
    vect[i] -= f[i];

  return *this;
}

///
/// \param[in] f a fitness.
/// \return the difference between \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator-(
  const basic_fitness_t<T, N> &f) const
{
  basic_fitness_t<T, N> tmp;
  for (decltype(N) i(0); i < N; ++i)
    tmp[i] = vect[i] - f[i];

  return tmp;
}

///
/// \param[in] f a fitness.
/// \return the product of \a this and \a f.
///
template<class T, unsigned N>
basic_fitness_t<T, N> basic_fitness_t<T, N>::operator*(
  const basic_fitness_t<T, N> &f) const
{
  basic_fitness_t<T, N> tmp;
  for (decltype(N) i(0); i < N; ++i)
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
  for (decltype(N) i(0); i < N; ++i)
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
  for (decltype(N) i(0); i < N; ++i)
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
  for (decltype(N) i(0); i < N; ++i)
    tmp[i] = std::fabs(vect[i]);

  return tmp;
}

///
/// \return a new vector obtained taking the square root of each component of
///         \a this.
///
template<class T, unsigned N>
basic_fitness_t<T, N> sqrt(basic_fitness_t<T, N> f)
{
  for (decltype(N) i(0); i < N; ++i)
    f[i] = std::sqrt(f[i]);

  return f;
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
/// \return \c true if each component of the fitness vector is small.
///
template<class T, unsigned N>
bool basic_fitness_t<T, N>::issmall() const
{
  for (const auto &i : vect)
    if (!vita::issmall(i))
      return false;
  return true;
}

///
/// \param[in] v1 a floating point number.
/// \param[in] v2 a floating point number.
/// \param[in] epsilon max relative error. If we want 99.999% accuracy then
///                    we should pass a \a epsilon of 0.00001.
/// \return \c true if the difference between \a v1 and \a v2 is "small"
///         compared to their magnitude.
///
/// \note
/// Code from Bruce Dawson:
/// <www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm>
///
template<class T>
bool almost_equal(T v1, T v2, T epsilon)
{
  const T diff(std::abs(v1 - v2));

  // Check if the numbers are really close -- needed
  // when comparing numbers near zero.
  if (diff <= 10.0 * std::numeric_limits<T>::min())
    return true;

  v1 = std::abs(v1);
  v2 = std::abs(v2);

  // In order to get consistent results, we should always compare the
  // difference to the larger of the two numbers.
  const T largest(std::max(v1, v2));

  return diff <= largest * epsilon;
}

///
/// See vita::almost_equal function for scalar types.
///
template<class T, unsigned N>
bool almost_equal(const basic_fitness_t<T, N> &f1,
                  const basic_fitness_t<T, N> &f2, T epsilon)
{
  for (decltype(N) i(0); i < N; ++i)
    if (!almost_equal(f1[i], f2[i], epsilon))
      return false;

  return true;
}

template<class T, unsigned N>
double basic_fitness_t<T, N>::distance(const basic_fitness_t<T, N> &f) const
{
  double d(0.0);

  for (decltype(N) i(0); i < N; ++i)
    d += std::fabs(vect[i] - f[i]);

  return d;
}

#endif  // Include guard
