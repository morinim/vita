/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
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

#include "kernel/vita.h"

namespace vita
{
bool iequals(const std::string &, const std::string &);
std::string trim(const std::string &);

///
/// An implementation of `make_unique()` as proposed by Herb Sutter in
/// GotW #102.
///
template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

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
  SAVE_FLAGS(in);

  return !!(in >> std::fixed >> std::scientific
               >> std::setprecision(std::numeric_limits<T>::digits10 + 1)
               >> *i);
}

///
/// \brief Simple parser for CSV files
///
/// \warning The class doesn't support multi-line fields
///
/// \warning
/// Usually, in C++, a fluent interface returns a **reference**.
/// Here we return a **copy** of `this` object. The design decision is due to
/// the fact that a `csv_parser' is a sort of Python generator and tends to
/// be used in for loops.
/// Users often write:
/// `for (auto record : csv_parser(f).filter_hook(filter))`
/// but it's broken (it only works if `filter_hook` returns by value).
/// `csv_parser` is a lighweight parser and this shouldn't be a performance
/// concern.
/// See <http://stackoverflow.com/q/10593686/3235496>.
///
class csv_parser
{
public:
  using value_type = std::vector<std::string>;
  using filter_hook_t = bool (*)(const value_type &);

  explicit csv_parser(std::istream &is)
    : is_(&is), filter_hook_(nullptr), delimiter_(','), trim_ws_(false)
  {}

  /// \param[in] delim separator character for fields.
  /// \return a reference to `this` object (fluent interface).
  csv_parser delimiter(char delim)
  {
    delimiter_ = delim;
    return *this;
  }

  /// \param[in] t if `true` trims leading and trailing spaces adjacent to
  ///              commas (this practice is contentious and in fact is
  ///              specifically prohibited by RFC 4180, which states: "Spaces
  ///              are considered part of a field and should not be ignored.").
  /// \return a reference to `this` object (fluent interface).
  csv_parser trim_ws(bool t)
  {
    trim_ws_ = t;
    return *this;
  }

  /// \param[in] filter a filter function for CSV records.
  /// \return a reference to `this` object (fluent interface).
  ///
  /// \note A filter function returns `true` for records to be keep.
  ///
  csv_parser filter_hook(filter_hook_t filter)
  {
    filter_hook_ = filter;
    return *this;
  }

  class const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

private:
  std::istream *is_;

  filter_hook_t filter_hook_;
  char delimiter_;
  bool trim_ws_;
};  // class csv_parser

///
/// \brief A forward iterator for CSV records
///
class csv_parser::const_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = csv_parser::value_type;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  const_iterator(std::istream *is = nullptr,
                 csv_parser::filter_hook_t f = nullptr,
                 char delim = ',', bool trim = false)
    : ptr_(is), filter_hook_(f), delimiter_(delim), trim_ws_(trim),
      value_({})
  {
    if (ptr_)
      get_input();
  }

  /// \return an iterator pointing to the next record of the CSV file.
  const_iterator &operator++()
  {
    get_input();
    return *this;
  }

  /// \return reference to the current record of the CSV file.
  const_reference operator*() const { return value_; }

  /// \return pointer to the current record of the CSV file.
  const_pointer operator->() const { return &operator*(); }

  /// \param[in] lhs first term of comparison.
  /// \param[in] rhs second term of comparison.
  ///
  /// Returns `true` if iterators point to the same line.
  friend bool operator==(const const_iterator &lhs, const const_iterator &rhs)
  {
    return lhs.ptr_ == rhs.ptr_ && lhs.value_ == rhs.value_ &&
           (!lhs.ptr_ || lhs.ptr_->tellg() == rhs.ptr_->tellg());
  }

  friend bool operator!=(const const_iterator &lhs, const const_iterator &rhs)
  {
    return !(lhs == rhs);
  }

private:
  value_type parse_line(const std::string &);
  void get_input();

  // Data members MUST BE INITIALIZED IN THE CORRECT ORDER:
  // * `value_` is initialized via the `get_input` member function
  // * `get_input` works only if `_ptr`, `delimiter_` and `value_` are already
  //   initialized
  // So it's important that `value_` is the last value to be initialized.
  std::istream *ptr_;
  csv_parser::filter_hook_t filter_hook_;
  char delimiter_;
  bool trim_ws_;
  value_type value_;
};  // class csv_parser::const_iterator

}  // namespace vita

#endif  // Include guard
