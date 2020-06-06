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

#if !defined(VITA_UTILITY_H)
#define      VITA_UTILITY_H

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "kernel/common.h"
#include "kernel/value.h"

#include "tinyxml2/tinyxml2.h"

namespace vita
{
bool is_number(std::string);
bool iequals(const std::string &, const std::string &);
std::string replace(std::string, const std::string &, const std::string &);
std::string replace_all(std::string, const std::string &, const std::string &);
std::string trim(const std::string &);

///
/// \param[in] v value to check
/// \return      `true` if `v` is less than `epsilon`-tolerance
///
/// \note
/// `epsilon` is the smallest T-value that can be added to `1.0` without
/// getting `1.0` back (this is a much larger value than `DBL_MIN`).
///
template<class T> bool issmall(T v)
{
  static constexpr auto e(std::numeric_limits<T>::epsilon());

  return std::abs(v) < 2.0 * e;
}

///
/// \param[in] v value to check
/// \return      `true` if `v` is nonnegative
///
template<class T> bool isnonnegative(T v)
{
  return v >= static_cast<T>(0);
}

///
/// Reduced version of `boost::lexical_cast`.
///
/// \tparam T type we want to cast to
///
/// \param[in] s a string
/// \return      the content of string `s` converted in an object of type `T`
///
template<class T> T lexical_cast(const std::string &s)
{ return std::stoi(s); }
template<> inline double lexical_cast(const std::string &s)
{ return std::stod(s); }
template<> inline std::string lexical_cast(const std::string &s)
{ return s; }
template<class T> T lexical_cast(const value_t &);

///
/// A RAII class to restore the state of a stream to its original state.
///
/// `iomanip` manipulators are "sticky" (except `setw` which only affects the
/// next insertion). Often we need a way to apply an arbitrary number of
/// manipulators to a stream and revert the state to whatever it was before.
///
/// \note
/// An alternative is to shuffle everything into a temporary `stringstream` and
/// finally put that on the real stream (which has never changed its flags at
/// all). This approach is exception-safe but a little less performant.
///
class ios_flag_saver
{
public:
  explicit ios_flag_saver(std::ios &s) : s_(s), flags_(s.flags()),
                                         precision_(s.precision()),
                                         width_(s.width())
  {}

  ~ios_flag_saver()
  {
    s_.flags(flags_);
    s_.precision(precision_);
    s_.width(width_);
  }

  DISALLOW_COPY_AND_ASSIGN(ios_flag_saver);

private:
  std::ios &s_;
  std::ios::fmtflags flags_;
  std::streamsize precision_;
  std::streamsize width_;
};
#define SAVE_FLAGS(s) ios_flag_saver save ## __LINE__(s)

///
/// A single-pass output iterator that writes successive objects of type `T`
/// into the `std::basic_ostream` object for which it was constructed, using
/// `operator<<`. Optional delimiter string is written to the output stream
/// after the SECOND write operation.
///
/// Same interface as an std::ostream_iterator.
///
/// \remark Lifted from Jerry Coffin's `prefix_ostream_iterator`.
///
template <class T, class C = char, class traits = std::char_traits<C>>
class infix_iterator
{
public:
  // Type alias
  using iterator_category = std::output_iterator_tag;
  using value_type        = void;
  using difference_type   = void;
  using pointer           = void;
  using reference         = void;
  using char_type         = C;
  using traits_type       = traits;
  using ostream_type      = std::basic_ostream<C, traits>;

  explicit infix_iterator(ostream_type &s, const C *d = nullptr)
    : os_(&s), delimiter_(d), first_elem_(true)
  {}

  infix_iterator &operator=(const T &item)
  {
    // Here's the only real change from ostream_iterator:
    // normally, the '*os << item;' would come before the 'if'.
    if (!first_elem_ && delimiter_)
      *os_ << delimiter_;

    *os_ << item;
    first_elem_ = false;
    return *this;
  }

  infix_iterator &operator*() { return *this; }
  infix_iterator &operator++() { return *this; }
  infix_iterator &operator++(int) { return *this; }

private:
  std::basic_ostream<C, traits> *os_;
  const C *delimiter_;
  bool first_elem_;
};  // class infix_iterator

///
/// \param[in] val a value to be rounded
/// \return        `val` rounded to a fixed, vita-specific, number of decimals
///
template<class T>
T round_to(T val)
{
  constexpr T float_epsilon(0.0001);

  val /= float_epsilon;
  val = std::round(val);
  val *= float_epsilon;

  return val;
}

///
/// \param[in] v1 a floating point number
/// \param[in] v2 a floating point number
/// \param[in] e  max relative error. If we want 99.999% accuracy then we
///               should pass a `e` of 0.00001
/// \return       `true` if the difference between `v1` and `v2` is "small"
///               compared to their magnitude
///
/// \note Code from Bruce Dawson:
/// <www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm>
///
template<class T>
bool almost_equal(T v1, T v2, T e = 0.00001)
{
  const T diff(std::abs(v1 - v2));

  // Check if the numbers are really close -- needed when comparing numbers
  // near zero.
  if (issmall(diff))
    //if (diff <= 10.0 * std::numeric_limits<T>::min())
    return true;

  v1 = std::abs(v1);
  v2 = std::abs(v2);

  // In order to get consistent results, we always compare the difference to
  // the largest of the two numbers.
  const T largest(std::max(v1, v2));

  return diff <= largest * e;
}

///
/// \param[out] out the output stream
/// \param[in]  i   the floating-point value to be saved
/// \return         a reference to the output stream
///
template<class T>
std::ostream &save_float_to_stream(std::ostream &out, T i)
{
  static_assert(std::is_floating_point<T>::value,
                "save_float_to_stream requires a floating point type");

  SAVE_FLAGS(out);

  out << std::fixed << std::scientific
      << std::setprecision(std::numeric_limits<T>::digits10 + 1)
      << i;

  return out;
}

///
/// \param[in]  in the input stream
/// \param[out] i  the floating-point value to be loaded
/// \return        `true` if the operation is successful
///
template<class T>
bool load_float_from_stream(std::istream &in, T *i)
{
  static_assert(std::is_floating_point<T>::value,
                "load_float_from_stream requires a floating point type");

  SAVE_FLAGS(in);

  return !!(in >> std::fixed >> std::scientific
               >> std::setprecision(std::numeric_limits<T>::digits10 + 1)
               >> *i);
}

void set_text(tinyxml2::XMLElement *, const std::string &,
              const std::string &);

///
/// A convenient arrangement for inserting stream-aware objects into
/// `XMLDocument`.
///
/// \tparam T type of the value
///
/// \param[out] p parent element
/// \param[in]  e new xml element
/// \param[in]  s new xml element's value
///
template<class T>
void set_text(tinyxml2::XMLElement *p, const std::string &e, const T &v)
{
  std::ostringstream ss;
  ss << v;
  set_text(p, e, ss.str());
}

///
/// Encapsulate the logic to convert a scoped enumeration element to its
/// integer value.
///
/// \tparam E a scoped enumeration
///
/// \param[in] v element of an enum class
/// \return      the integer value of `v`
///
template<class E>
constexpr std::underlying_type_t<E> as_integer(E v)
{
  static_assert(std::is_enum_v<E>);
  return static_cast<std::underlying_type_t<E>>(v);
}

///
/// A generic function to "print" any scoped enum.
///
/// \tparam E a scoped enumeration
///
/// \param[in, out] s an output stream
/// \param[in]      v element of an enum class
/// \return           the modified output stream
///
template<class E>
std::enable_if_t<std::is_enum_v<E>, std::ostream> &
operator<<(std::ostream &s, E v)
{
  return s << as_integer(v);
}

}  // namespace vita

#endif  // include guard
