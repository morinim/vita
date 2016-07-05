/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_UTILITY_H)
#define      VITA_UTILITY_H

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "tinyxml2/tinyxml2.h"

#include "kernel/vita.h"

namespace vita
{
bool iequals(const std::string &, const std::string &);
std::string trim(const std::string &);

///
/// \return an `std::array` filled with value `v`.
///
/// This function is used to initialize array in member initialization list
/// of a constructor (so to be compliant with Effective C++).
/// To be efficient the compiler need to perform the RVO optimization / copy
/// elision.
///
/// \see <http://stackoverflow.com/q/21993780/3235496>
///
template<class T, unsigned N> std::array<T, N> make_array(T v)
{
  std::array<T, N> temp;
  temp.fill(v);
  return temp;
}

///
/// \param[in] v value to check.
/// \return `true` if v is less than epsilon-tolerance.
///
/// \note
/// `epsilon` is the smallest T-value that can be added to `1.0` without
/// getting `1.0` back. Note that this is a much larger value than `DBL_MIN`.
///
template<class T> bool issmall(T v)
{
  static constexpr auto e(std::numeric_limits<T>::epsilon());

  return std::abs(v) < 2.0 * e;
}

///
/// \param[in] v value to check.
/// \return `true` if v is nonnegative.
///
template<class T> bool isnonnegative(T v)
{
  return v >= static_cast<T>(0.0);
}

///
/// \param[in] container a STL container.
/// \param[in] pred a unary predicate.
///
/// A shorthand for the well known C++ erase-remove idiom.
///
template<class T, class Pred> void erase_if(T &container, Pred pred)
{
  container.erase(std::remove_if(container.begin(),
                                 container.end(),
                                 pred),
                  container.end());
}

///
/// \param[in] s a string
/// \return the content of string `s` converted in an object of type `T`.
///
/// Reduced version of `boost::lexical_cast`.
///
template<class T> T lexical_cast(const std::string &s)
{ return std::stoi(s); }
template<> inline double lexical_cast(const std::string &s)
{ return std::stod(s); }
template<> inline std::string lexical_cast(const std::string &s)
{ return s; }

///
/// \brief A RAII class to restore the state of a stream to its original
///        state
///
/// `iomanip` manipulators are "sticky" (except `setw` which only affects the
/// next insertion). Often we need a way to apply an arbitrary number of
/// manipulators to a stream and revert the state to whatever it was before.
///
/// \note
/// An alternative is to shuffle everything into a temporary `stringstream` and
/// finally put that on the real stream (which has never changed its flags at
/// all).
/// This approach is exception-safe but a little less performant.
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
/// A single-pass output iterator that writes successive objects of type T
/// into the `std::basic_ostream` object for which it was constructed, using
/// `operator<<`. Optional delimiter string is written to the output stream
/// after the SECOND write operation.
///
/// Same interface as an std::ostream_iterator.
///
/// Lifted from Jerry Coffin's `prefix_ostream_iterator`.
///
template <class T, class C = char, class traits = std::char_traits<C>>
class infix_iterator : public std::iterator<std::output_iterator_tag, void,
                                            void, void, void>
{
public:  // Type alias
  using char_type    = C;
  using traits_type  = traits;
  using ostream_type = std::basic_ostream<C, traits>;

public:
  explicit infix_iterator(ostream_type &s, const C *d = nullptr)
    : os_(&s), delimiter_(d), first_elem_(true)
  {}

  infix_iterator<T, C, traits> &operator=(const T &item)
  {
    // Here's the only real change from ostream_iterator:
    // normally, the '*os << item;' would come before the 'if'.
    if (!first_elem_ && delimiter_)
      *os_ << delimiter_;

    *os_ << item;
    first_elem_ = false;
    return *this;
  }

  infix_iterator<T, C, traits> &operator*() { return *this; }
  infix_iterator<T, C, traits> &operator++() { return *this; }
  infix_iterator<T, C, traits> &operator++(int) { return *this; }

private:
  std::basic_ostream<C, traits> *os_;
  const C *delimiter_;
  bool first_elem_;
};  // class infix_iterator

///
/// \brief Rounds `val` to the number of decimals of `float_epsilon`.
///
/// \param val a value to be rounded.
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
/// \param[in] v1 a floating point number.
/// \param[in] v2 a floating point number.
/// \param[in] e max relative error. If we want 99.999% accuracy then we
///              should pass a `e` of 0.00001.
/// \return `true` if the difference between `v1` and `v2` is "small" compared
///         to their magnitude.
///
/// \note
/// Code from Bruce Dawson:
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
/// \param[out] out the output stream.
/// \param[in] i the floating-point value to be saved.
/// \return a reference to the output stream.
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
/// \param[in] in the input stream.
/// \param[out] i the floating-point value to be loaded.
/// \return `true` if the operation is successful.
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
/// \brief A convenient arrangement for inserting stream-aware objects into
///        `XMLDocument`
/// \param[out] p parent element
/// \param[in] e new xml element
/// \param[in] s new xml element's value
///
template<class T>
void set_text(tinyxml2::XMLElement *p, const std::string &e, const T &v)
{
  std::ostringstream ss;
  ss << v;
  set_text(p, e, ss.str());
}

///
/// \brief Encapsulate the logic to convert a scoped enumeration element to its
///        integer value.
/// \tparam E a scoped enumeration.
/// \param[in] v element of an enum class.
/// \return the integer value of `v`.
///
template<class E>
constexpr typename std::underlying_type<E>::type as_integer(E v)
{
  static_assert(std::is_enum<E>::value,
                "as_integer needs a scoped enumeration");
  return static_cast<typename std::underlying_type<E>::type>(v);
}

///
/// \brief A generic function to "print" any scoped enum.
/// \tparam E a scoped enumeration.
/// \param[in, out] s an output stream.
/// \param[in] v element of an enum class.
/// \return the modified output stream.
///
template<class E>
typename std::enable_if<std::is_enum<E>::value, std::ostream>::type &
operator<<(std::ostream &s, E v)
{
  return s << as_integer(v);
}

///
/// \brief Null-coalescing operator.
/// \param[in] v1 first operand.
/// \param[in] v2 second operand.
/// \return the first operand if it isn't `empty`; otherwise it returns the
///         second operand.
///
template<class T>
const T &coalesce(const T &v1, const T &v2)
{
  return v1.empty() ? v2 : v1;
}

}  // namespace vita

#endif  // Include guard
