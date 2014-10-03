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

#if !defined(VITA_DISTRIBUTION_INL_H)
#define      VITA_DISTRIBUTION_INL_H

///
/// Just the initial setup.
///
template<class T>
distribution<T>::distribution() : variance(), min(), max(), freq(),
                                  m2_(), mean_(), count_(0)
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
/// \return The mean value of the distribution
///
template<class T>
T distribution<T>::mean() const
{
  return mean_;
}

///
/// \brief Add a new value to the distribution
/// \param[in] val new value upon which statistics are recalculated.
///
template<class T>
void distribution<T>::add(T val)
{
  using std::isnan;

  if (!isnan(val))
  {
    if (!count())
      min = max = mean_ = val;
    else if (val < min)
      min = val;
    else if (val > max)
      max = val;

    ++count_;

    ++freq[round_to(val)];

    update_variance(val);
  }
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
  for (const auto &f : freq)  // f.first: fitness, f.second: frequency
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

  variance = m2_ / c1;
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

  return sqrt(variance);
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

  out << count() << std::endl
      << std::fixed << std::scientific
      << std::setprecision(std::numeric_limits<T>::digits10 + 1)
      << mean() << std::endl
      << variance  << std::endl
      << min  << std::endl
      << max  << std::endl
      << m2_ << std::endl;

  out << freq.size() << std::endl;
  for (const auto &elem : freq)
    out << elem.first << ' ' << elem.second << std::endl;

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

  decltype(variance) variance_;
  if (!(in >> variance_))
    return false;

  decltype(min) min_;
  if (!(in >> min_))
    return false;

  decltype(max) max_;
  if (!(in >> max_))
    return false;

  decltype(m2_) m2__;
  if (!(in >> m2__))
    return false;

  typename decltype(freq)::size_type n;
  if (!(in >> n))
    return false;

  decltype(freq) freq_;
  for (decltype(n) i(0); i < n; ++i)
  {
    typename decltype(freq)::key_type key;
    typename decltype(freq)::mapped_type val;
    if (!(in >> key >> val))
      return false;

    freq_[key] = val;
  }

  count_ = c;
  mean_ = m;
  variance = variance_;
  min = min_;
  max = max_;
  m2_ = m2__;
  freq = freq_;

  return true;
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T>
bool distribution<T>::debug(bool verbose) const
{
  // This way, for "regular" types we'll use std::infinite / std::isnan
  // ("taken in" by the using statement), while for our types the overload
  // will prevail due to Koenig lookup (<http://www.gotw.ca/gotw/030.htm>).
  using std::isfinite;
  using std::isnan;

  if (count() && isfinite(min) && isfinite(mean()) && min > mean())
  {
    if (verbose)
      std::cerr << k_s_debug << " Distribution: min=" << min << " > mean="
                << mean() << "." << std::endl;
    return false;
  }

  if (count() && isfinite(max) && isfinite(mean()) && max < mean())
  {
    if (verbose)
      std::cerr << k_s_debug << " Distribution: max=" << max << " < mean="
                << mean() << "." << std::endl;
    return false;
  }

  if (count() && (isnan(variance) || variance < T(0.0)))
  {
    if (verbose)
      std::cerr << k_s_debug << " Distribution: negative variance."
                << std::endl;
    return false;
  }

  return true;
}
#endif  // Include guard
