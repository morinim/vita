/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DISTRIBUTION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_DISTRIBUTION_TCC)
#define      VITA_DISTRIBUTION_TCC

///
/// Just the initial setup.
///
template<class T>
distribution<T>::distribution() : seen_(), m2_(), max_(), mean_(), min_(),
                                  count_(0)
{
}

///
/// Resets gathered statics.
///
template<class T>
void distribution<T>::clear()
{
  *this = distribution();
}

///
/// \return Number of elements of the distribution.
///
template<class T>
std::uintmax_t distribution<T>::count() const
{
  return count_;
}

///
/// \return The maximum value of the distribution
///
template<class T>
T distribution<T>::max() const
{
  assert(count());
  return max_;
}

///
/// \return The minimum value of the distribution
///
template<class T>
T distribution<T>::min() const
{
  assert(count());
  return min_;
}

///
/// \return The mean value of the distribution
///
template<class T>
T distribution<T>::mean() const
{
  assert(count());
  return mean_;
}

///
/// \return The variance of the distribution
///
template<class T>
T distribution<T>::variance() const
{
  assert(count());
  return m2_ / static_cast<double>(count());
}

///
/// Add a new value to the distribution.
///
/// \param[in] val new value upon which statistics are recalculated
///
/// \remark Function ignores NAN values.
///
template<class T>
template<class U>
void distribution<T>::add(U val)
{
  using std::isnan;

  if constexpr (std::is_floating_point_v<U>)
    if (isnan(val))
      return;

  const auto v1(static_cast<T>(val));

  if constexpr (std::is_floating_point_v<T>)
    if (isnan(v1))
      return;

  if (!count())
    min_ = max_ = mean_ = v1;
  else if (v1 < min())
    min_ = v1;
  else if (v1 > max())
    max_ = v1;

  ++count_;

  ++seen_[round_to(v1)];

  update_variance(v1);
}

template<class T>
const std::map<T, std::uintmax_t> &distribution<T>::seen() const
{
  return seen_;
}

///
/// \return the entropy of the distribution.
///
/// \f$H(X)=-\sum_{i=1}^n p(x_i) \dot log_b(p(x_i))\f$
/// We use an offline algorithm
/// (http://en.wikipedia.org/wiki/Online_algorithm).
///
template<class T>
double distribution<T>::entropy() const
{
  const double c(1.0 / std::log(2.0));

  double h(0.0);
  for (const auto &f : seen())  // f.first: fitness, f.second: sightings
  {
    const auto p(static_cast<double>(f.second) / static_cast<double>(count()));

    h -= p * std::log(p) * c;
  }

  return h;
}

///
/// \param[in] val new value upon which statistics are recalculated.
///
/// Calculate running variance and cumulative average of a set. The
/// algorithm used is due to Knuth (Donald E. Knuth - The Art of Computer
/// Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232.
/// Addison-Wesley).
///
/// \see
/// * <http://en.wikipedia.org/wiki/Online_algorithm>
/// * <http://en.wikipedia.org/wiki/Moving_average#Cumulative_moving_average>
///
template<class T>
void distribution<T>::update_variance(T val)
{
  assert(count());

  const auto c1(static_cast<double>(count()));

  const T delta(val - mean());
  mean_ += delta / c1;

  // This expression uses the new value of mean.
  if (count() > 1)
    m2_ += delta * (val - mean());
  else
    m2_ =  delta * (val - mean());
}

///
/// \return the standard deviation of the distribution.
///
template<class T>
T distribution<T>::standard_deviation() const
{
  // This way, for "regular" types we'll use std::sqrt ("taken in" by the
  // using statement), while for our types the overload will prevail due to
  // Koenig lookup (<http://www.gotw.ca/gotw/030.htm>).
  using std::sqrt;

  return sqrt(variance());
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the distribution on persistent storage.
///
template<class T>
bool distribution<T>::save(std::ostream &out) const
{
  SAVE_FLAGS(out);

  out << count() << '\n'
      << std::fixed << std::scientific
      << std::setprecision(std::numeric_limits<T>::digits10 + 1)
      << mean() << '\n'
      << min()  << '\n'
      << max()  << '\n'
      << m2_ << '\n';

  out << seen().size() << '\n';
  for (const auto &elem : seen())
    out << elem.first << ' ' << elem.second << '\n';

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the distribution from persistent storage.
///
/// \note
/// If the load operation isn't successful the current object isn't modified.
///
template<class T>
bool distribution<T>::load(std::istream &in)
{
  SAVE_FLAGS(in);

  decltype(count_) c;
  if (!(in >> c))
    return false;

  in >> std::fixed >> std::scientific
     >> std::setprecision(std::numeric_limits<T>::digits10 + 1);

  decltype(mean_) m;
  if (!(in >> m))
    return false;

  decltype(min_) mn;
  if (!(in >> mn))
    return false;

  decltype(max_) mx;
  if (!(in >> mx))
    return false;

  decltype(m2_) m2__;
  if (!(in >> m2__))
    return false;

  typename decltype(seen_)::size_type n;
  if (!(in >> n))
    return false;

  decltype(seen_) s;
  for (decltype(n) i(0); i < n; ++i)
  {
    typename decltype(seen_)::key_type key;
    typename decltype(seen_)::mapped_type val;
    if (!(in >> key >> val))
      return false;

    s[key] = val;
  }

  count_ = c;
  mean_ = m;
  min_ = mn;
  max_ = mx;
  m2_ = m2__;
  seen_ = s;

  return true;
}

///
/// \return `true` if the object passes the internal consistency check.
///
template<class T>
bool distribution<T>::is_valid() const
{
  // This way, for "regular" types we'll use std::infinite / std::isnan
  // ("taken in" by the using statement), while for our types the overload
  // will prevail due to Koenig lookup (<http://www.gotw.ca/gotw/030.htm>).
  using std::isfinite;
  using std::isnan;

  if (count() && isfinite(min()) && isfinite(mean()) && min() > mean())
  {
    vitaERROR << "Distribution: min=" << min() << " > mean=" << mean();
    return false;
  }

  if (count() && isfinite(max()) && isfinite(mean()) && max() < mean())
  {
    vitaERROR << "Distribution: max=" << max() << " < mean=" << mean();
    return false;
  }

  if (count() && (isnan(variance()) || !isnonnegative(variance())))
  {
    vitaERROR << "Distribution: negative variance";
    return false;
  }

  return true;
}
#endif  // Include guard
