/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_FITNESS_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_FITNESS_TCC)
#define      VITA_FITNESS_TCC

///
/// \brief Fills the fitness with `n` copy of value `v`
///
/// \param[in] n number of components of the fitness.
/// \param[in] v default value.
///
/// Both Herb Sutter and Scott Meyers recommend to avoid class designs where
/// a `initializer_list` constructor overload can cause ambiguities to the
/// programmer. We use tags on the constructor to avoid such situations.
///
/// The tag also helps to clarify the meaning of the other arguments.
///
template<class T>
basic_fitness_t<T>::basic_fitness_t(unsigned n, fit_tag, T v) : vect_(n, v)
{
  Expects(n);
}

///
/// Builds a fitness from a list of values.
///
template<class T>
basic_fitness_t<T>::basic_fitness_t(std::initializer_list<T> l) : vect_(l)
{
}

///
/// Builds a fitness from a vector of values.
///
template<class T>
basic_fitness_t<T>::basic_fitness_t(values_t v) : vect_(std::move(v))
{
}

///
/// \return the size of the fitness vector.
///
template<class T>
unsigned basic_fitness_t<T>::size() const
{
  return static_cast<unsigned>(vect_.size());
}

///
/// \param[in] i index of an element
/// \return the i-th element of the fitness vector.
///
template<class T>
T basic_fitness_t<T>::operator[](unsigned i) const
{
  // This assert could be considered a bit too strict. In general taking the
  // address of one past the last element is allowed, e.g.
  //     std::copy(&f[0], &f[N], &dest);
  // but here the assertion will signal this use case. The workaround is:
  //     std::copy(&f[0], &f[0] + N, &dest);
  Expects(i < size());
  return vect_[i];
}

///
/// \param[in] i index of an element
/// \return a reference to the i-th element of the fitness vector.
///
template<class T>
T &basic_fitness_t<T>::operator[](unsigned i)
{
  Expects(i < size());
  return vect_[i];
}

///
/// \return returns an iterator to the first element of the container. If the
///         container is empty, the returned iterator will be equal to end().
///
template<class T>
typename basic_fitness_t<T>::iterator basic_fitness_t<T>::begin()
{
  return std::begin(vect_);
}

///
/// \return returns an iterator to the first element of the container. If the
///         container is empty, the returned iterator will be equal to end().
///
template<class T>
typename basic_fitness_t<T>::const_iterator basic_fitness_t<T>::begin() const
{
  return vect_.cbegin();
}

///
/// \return returns an iterator to the element following the last element of
///         the container. This element acts as a placeholder; attempting to
//          access it results in undefined behavior.
///
template<class T>
typename basic_fitness_t<T>::iterator basic_fitness_t<T>::end()
{
  return std::end(vect_);
}

///
/// \return returns an iterator to the element following the last element of
///         the container. This element acts as a placeholder; attempting to
///         access it results in undefined behavior.
///
template<class T>
typename basic_fitness_t<T>::const_iterator basic_fitness_t<T>::end() const
{
  return vect_.cend();
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
///
/// Operation is performed by first comparing sizes and, if they match,
/// the elements are compared sequentially using algorithm equal, which
/// stops at the first mismatch.
///
/// \relates basic_fitness_t
///
template<class T>
bool operator==(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return lhs.size() == rhs.size() &&
         std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
///
/// Operation is performed by first comparing sizes and, if they match,
/// the elements are compared sequentially using algorithm equal, which
/// stops at the first mismatch.
///
/// \relates basic_fitness_t
///
template<class T>
bool operator!=(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return !operator==(lhs, rhs);
}

///
/// \brief Lexicographic ordering
///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return `true` if the contents of the `lhs` are lexicographically less than
///         the contents of `rhs`, `false` otherwise.
///
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
///
/// \relates basic_fitness_t
///
template<class T>
bool operator<(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return std::lexicographical_compare(std::begin(lhs), std::end(lhs),
                                      std::begin(rhs), std::end(rhs));

  // An alternative implementation:
  // > for (unsigned i(0); i < size(); ++i)
  // >   if (operator[](i) != f[i])
  // >     return operator[](i) > f[i];
  // > return false;
}


///
/// \brief Lexicographic ordering
///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return `true` if the contents of the `lhs` are lexicographically greater
///         than or equal the contents of `rhs`, `false` otherwise.
///
/// \see basic_fitness_t::operator<
///
/// \relates basic_fitness_t
///
template<class T>
bool operator>(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return operator<(rhs, lhs);
}

///
/// \brief Lexicographic ordering
///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return `true` if the contents of the `lhs` are lexicographically greater
///         than or equal the contents of `rhs`, `false` otherwise.
///
/// \see basic_fitness_t::operator<
///
/// \relates basic_fitness_t
///
template<class T>
bool operator>=(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return !operator<(lhs, rhs);
}

///
/// \brief Lexicographic ordering
///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparision.
/// \return `true` if the contents of the `lhs` are lexicographically less
///         than or equal the contents of `rhs`, `false` otherwise.
///
/// \see basic_fitness_t::operator<
///
/// \relates basic_fitness_t
///
template<class T>
bool operator<=(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  return !operator>(lhs, rhs);
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if `lhs` is a Pareto improvement of `rhs`.
///
/// `lhs` dominates `rhs` (is a Pareto improvement) if:
/// - each component of `lhs` is not strictly worst (less) than the
///   correspondig component of `rhs`;
/// - there is at least one component in which `lhs` is better than `rhs`.
///
/// \note
/// An interesting property is that if a vector `x` does not dominate a
/// vector `y`, this does not imply that `y` dominates `x` (they can be both
/// non-dominated).
///
/// \relates basic_fitness_t
///
template<class T>
bool dominating(const basic_fitness_t<T> &lhs, const basic_fitness_t<T> &rhs)
{
  bool one_better(false);

  const auto n(lhs.size());
  for (unsigned i(0); i < n; ++i)
    if (lhs[i] > rhs[i])
      one_better = true;
    else if (lhs[i] < rhs[i])
      return false;

  return one_better;
}

///
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current basic_fitness_t isn't
/// changed.
///
template<class T>
bool basic_fitness_t<T>::load(std::istream &in)
{
  unsigned s;
  if (!(in >> s))
    return false;

  basic_fitness_t<T> tmp(s, fit_tag::components);

  for (auto &e : tmp.vect_)
    if (!load_float_from_stream(in, &e))
      return false;

  *this = tmp;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if object has been saved correctly.
///
template<class T>
bool basic_fitness_t<T>::save(std::ostream &out) const
{
  out << size() << '\n';

  for (const auto &i : vect_)
  {
    save_float_to_stream(out, i);
    out << ' ';
  }

  out << '\n';

  return out.good();
}

///
/// Standard output operator for basic_fitness_t.
///
/// \relates basic_fitness_t
///
template<class T>
std::ostream &operator<<(std::ostream &o, basic_fitness_t<T> f)
{
  o << '(';

  std::copy(&f[0], &f[0] + f.size(), infix_iterator<T>(o, ", "));

  return o << ')';
}

///
/// \param[in] f a fitness.
/// \return the sum of `this` and `f`.
///
template<class T>
basic_fitness_t<T> &basic_fitness_t<T>::operator+=(const basic_fitness_t<T> &f)
{
  const auto n(size());
  for (unsigned i(0); i < n; ++i)
    operator[](i) += f[i];

  return *this;
}

///
/// \param[in] lhs first addend.
/// \param[in] rhs second addend.
/// \return the sum of `lhs` and `rhs`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> operator+(basic_fitness_t<T> lhs,
                             const basic_fitness_t<T> &rhs)
{
  // operator+ shouldn't be a member function otherwise it won't work as
  // naturally as user may expect (i.e. asymmetry in implicit conversion from
  // other types.
  // Implementing `+` in terms of `+=` makes the code simpler and guarantees
  // consistent semantics as the two functions are less likely to diverge
  // during maintenance.
  return lhs += rhs;
}


///
/// \param[in] f a fitness.
/// \return the difference of `this` and `f`.
///
template<class T>
basic_fitness_t<T> &basic_fitness_t<T>::operator-=(const basic_fitness_t<T> &f)
{
  const auto n(size());
  for (unsigned i(0); i < n; ++i)
    operator[](i) -= f[i];

  return *this;
}

///
/// \param[in] lhs the minuend.
/// \param[in] rhs the subtrahend.
/// \return the difference between `lhs` and `rhs`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> operator-(basic_fitness_t<T> lhs,
                             const basic_fitness_t<T> &rhs)
{
  return lhs -= rhs;
}

///
/// \param[in] f a fitness.
/// \return the product of `this` and `f`.
///
template<class T>
basic_fitness_t<T> &basic_fitness_t<T>::operator*=(const basic_fitness_t &f)
{
  const auto n(size());
  for (unsigned i(0); i < n; ++i)
    operator[](i) *= f[i];

  return *this;
}

///
/// \param[in] lhs first factor.
/// \param[in] rhs second factor.
/// \return the product of `lhs` and `rhs`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> operator*(basic_fitness_t<T> lhs,
                             const basic_fitness_t<T> &rhs)
{
  return lhs *= rhs;
}

///
/// \param[in] f a fitness value.
/// \param[in] v a scalar.
/// \return a new vector obtained dividing each component of `f` by the scalar
///         value `v`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> operator/(basic_fitness_t<T> f, T v)
{
  for (auto &f_i : f)
    f_i /= v;

  return f;
}

///
/// \param[in] f a fitness value.
/// \param[in] v a scalar.
/// \return a new vector obtained multiplying each component of `f` by the
///         scalar value `v`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> operator*(basic_fitness_t<T> f, T v)
{
  for (auto &f_i : f)
    f_i *= v;

  return f;
}

///
/// \param[in] f a fitness value.
/// \return a new vector obtained taking the absolute value of each component
///         of `f`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> abs(basic_fitness_t<T> f)
{
  for (auto &f_i : f)
    f_i = std::abs(f_i);

  return f;

  // An alternative is:
  //     std::transform(std::begin(f), std::end(f), std::begin(f),
  //                    static_cast<T (*)(T)>(std::abs));
}

///
/// \param[in] f a fitness.
/// \return a new vector obtained "rounding" each component of `f`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> round_to(basic_fitness_t<T> f)
{
  for (auto &f_i : f)
    f_i = round_to(f_i);

  return f;
}

///
/// \param[in] f a fitness.
/// \return a new vector obtained taking the square root of each component of
///         `f`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> sqrt(basic_fitness_t<T> f)
{
  for (auto &f_i : f)
    f_i = std::sqrt(f_i);

  return f;
}

///
/// \param[in] f fitness to check.
/// \return `true` if every component of the fitness is finite.
///
/// \relates basic_fitness_t
///
template<class T>
bool isfinite(const basic_fitness_t<T> &f)
{
  return std::all_of(std::begin(f), std::end(f),
                     static_cast<bool (*)(T)>(std::isfinite));
}

///
/// \param[in] f fitness to check.
/// \return `true` if a component of the fitness is `NAN`.
///
/// \relates basic_fitness_t
///
template<class T>
bool isnan(const basic_fitness_t<T> &f)
{
  return std::any_of(std::begin(f), std::end(f),
                     static_cast<bool (*)(T)>(std::isnan));
}

///
/// \param[in] f fitness to check.
/// \return `true` if each component of the fitness vector is small.
///
/// \relates basic_fitness_t
///
template<class T>
bool issmall(const basic_fitness_t<T> &f)
{
  return std::all_of(std::begin(f), std::end(f),
                     static_cast<bool (*)(T)>(vita::issmall));
}

///
/// \param[in] f a fitness to check.
/// \return true if every element of `f` is nonnegative.
///
/// \relates basic_fitness_t
///
template<class T> bool isnonnegative(const basic_fitness_t<T> &f)
{
  return std::all_of(std::begin(f), std::end(f),
                     static_cast<bool (*)(T)>(vita::isnonnegative));
}

///
/// See vita::almost_equal function for scalar types.
///
/// \relates basic_fitness_t
///
template<class T>
bool almost_equal(const basic_fitness_t<T> &f1,
                  const basic_fitness_t<T> &f2, T ae_epsilon)
{
  const auto n(f1.size());
  assert(f2.size() == n);

  for (unsigned i(0); i < n; ++i)
    if (!almost_equal(f1[i], f2[i], ae_epsilon))
      return false;

  return true;
}

///
/// \param[in] f1 first fitness value
/// \param[in] f2 second fitness value
/// \return the distance between `f1` and `f2`.
///
/// \relates basic_fitness_t
///
template<class T>
double distance(const basic_fitness_t<T> &f1, const basic_fitness_t<T> &f2)
{
  double d(0.0);

  const auto n(f1.size());
  assert(f2.size() == n);

  for (unsigned i(0); i < n; ++i)
    d += std::abs(f1[i] - f2[i]);

  return d;
}


///
/// \param[in] f1 first fitness value
/// \param[in] f2 second fitness value
/// \return the fitness vector obtained joining `f1` and `f2`.
///
/// \relates basic_fitness_t
///
template<class T>
basic_fitness_t<T> combine(const basic_fitness_t<T> &f1,
                           const basic_fitness_t<T> &f2)
{
  typename basic_fitness_t<T>::values_t ret;
  ret.reserve(f1.size() + f2.size());

  ret.insert(std::end(ret), std::begin(f1), std::end(f1));
  ret.insert(std::end(ret), std::begin(f2), std::end(f2));

  return ret;
}

#endif  // Include guard
