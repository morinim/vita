/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

#include "kernel/vita.h"

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

namespace detail { namespace any_ {

struct empty;
struct fxn_ptr_table;
template<bool> struct fxns;
template<class> struct get_table;

}}

///
/// \brief The exception thrown in the event of a failed any_cast of an any
///        value
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
/// \brief A boost::any equivalent with small buffer optimization and
///        support for the streaming operators
///
/// This class is derived from and equivalent to boost::spirit::hold_any
/// (which is built based on the any class published in
/// <http://www.codeproject.com/cpp/dynamic_typing.asp>).
///
/// vita::any:
/// - doesn't depend on Boost library (but needs C++11);
/// - correctly supports copy assignment (see
///   <http://stackoverflow.com/q/24065769/3235496> and
///   <https://svn.boost.org/trac/boost/ticket/8268> for details about a
///   major bug in boost::spirit::hold_any v1.55);
/// - assuming C++11 is a bit simpler.
///
class any
{
public:  // Constructors
  template<class T> explicit any(const T &);

  any();
  any(const any &);

  /// Move constructor that moves content of \a x into new instance and
  /// leaves \a x empty.
  /// \post \c x->empty()
  any(any &&x) : any() { swap(x); }

  ~any();

public:  // Assignment operator
  template<class T> any &operator=(T &&);

  /// Copies content of rhs into current instance, discarding previous
  /// content, so that the new content is equivalent in both type and value
  /// to the content of \a rhs, or empty if \c rhs.empty().
  /// \post \c rhs->empty()
  any &operator=(const any &rhs) { return assign(rhs); }

public:  // Utility functions
  /// Exchange of the contents of \c *this and \a rhs.
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

public:  // Types
  template<class T> friend T *any_cast(any *);

private:  // Assignment and casting
  template<class T> any &assign(const T &);

  any &assign(const any &);

  template<class T> const T &cast() const;

private:  // Private data members
  detail::any_::fxn_ptr_table *table;

  void *object = nullptr;
};  // class any
#endif  // !USE_BOOST_ANY guard

template<class T> T to(const any &);

#include "kernel/any.tcc"

}  // namespace vita

#endif  // Include guard
