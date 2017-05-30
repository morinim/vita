/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

#include "kernel/common.h"

#if defined(USE_BOOST_ANY)
#  include <boost/any.hpp>
#else
#  include <typeinfo>
#endif

namespace vita
{
#if defined(USE_BOOST_ANY)

using boost::any_cast<T>;

#else

namespace detail
{ namespace any_
{
struct empty;
struct fxn_ptr_table;
template<bool> struct fxns;
template<class> struct get_table;
}  // namespace any_
}  // namespace detail

///
/// The exception thrown in the event of a failed any_cast of an any value.
///
struct bad_any_cast : std::bad_cast
{
  bad_any_cast(const std::type_info &src, const std::type_info &dest)
    : from(src.name()), to(dest.name())
  {}

  virtual const char *what() const throw() { return "bad any cast"; }

  const char *from;
  const char *to;
};

///
/// A boost::any equivalent with small buffer optimization and support for the
/// streaming operators.
///
/// This class is derived from and equivalent to boost::spirit::hold_any
/// (which is built based on the any class published in
/// <http://www.codeproject.com/cpp/dynamic_typing.asp>).
///
/// vita::any doesn't depend on Boost library but needs C++14.
///
class any
{
public:
  // Constructors / destructor.
  template<class T> explicit any(const T &);
  any();
  any(const any &);

  /// Move constructor that moves content of `x` into new instance and leaves
  /// `x` empty.
  ///
  /// \post `x->empty()`
  any(any &&x) : any() { swap(x); }

  ~any();

  // Assignment operators.
  template<class T> any &operator=(T &&);

  /// Copies content of rhs into current instance, discarding previous
  /// content, so that the new content is equivalent in both type and value
  /// to the content of `rhs`, or empty if `rhs.empty()`.
  ///
  /// \post `rhs.empty()`
  any &operator=(const any &rhs) { return assign(rhs); }

  // Utility functions
  /// Exchange of the contents of `*this` and `rhs`.
  any &swap(any &rhs)
  {
    std::swap(table, rhs.table);
    std::swap(object, rhs.object);
    return *this;
  }

  const std::type_info &type() const;

  bool empty() const;

  void clear();

  friend std::istream &operator>>(std::istream&, any &);
  friend std::ostream &operator<<(std::ostream&, const any &);

  // Types
  template<class T> friend T *any_cast(any *);

private:
  template<class T> explicit any(const T &, std::true_type *);
  template<class T> explicit any(const T &, std::false_type *);

  // Assignment and casting
  template<class T> any &assign(const T &);

  any &assign(const any &);

  template<class T> const T &cast() const;

  // Private data members
  detail::any_::fxn_ptr_table *table;

  void *object = nullptr;
};  // class any
#endif  // !USE_BOOST_ANY guard

template<class T> T to(const any &);

#include "utility/any.tcc"

}  // namespace vita

#endif  // include guard
