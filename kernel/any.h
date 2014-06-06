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

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

#include "kernel/vita.h"

#if defined(USE_BOOST_ANY)
#  include <boost/any.hpp>
#else
#  include <typeinfo>
#  include "kernel/detail/any.h"
#endif

namespace vita
{
#if defined(USE_BOOST_ANY)
  using boost::any_cast<T>;
#else
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
  ///        support for the streaming operators.
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

    /// \post \c this->empty()
    any() : table(detail::any_::get_table<detail::any_::empty>::get()) {}

    /// Copy constructor that copies content of \a x into new instance, so
    /// that any content is equivalent in both type and value to the content of
    /// \a x or empty if \a x is empty.
    any(const any &x)
      : table(detail::any_::get_table<detail::any_::empty>::get())
    {
      assign(x);
    }

    /// Move constructor that moves content of \a x into new instance and
    /// leaves \a x empty.
    /// \post \c x->empty()
    any(any &&x) : any() { swap(x); }

    /// Releases any and all resources used in management of instance.
    ~any() { table->static_delete(&object); }

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

    /// The typeid of the contained value if instance is non-empty, otherwise
    /// typeid(void).
    /// \note
    /// Useful for querying against types known either at compile time or only
    /// at runtime.
    const std::type_info &type() const { return table->get_type(); }

    /// \c true if instance is empty, otherwise \c false.
    bool empty() const
    {
      return table == detail::any_::get_table<detail::any_::empty>::get();
    }

    void clear()
    {
      if (!empty())
      {
        table->static_delete(&object);
        table = detail::any_::get_table<detail::any_::empty>::get();
        object = nullptr;
      }
    }

    // These functions have been added in the assumption that the embedded
    // type has a corresponding operator defined, which is completely safe
    // because any is used only in contexts where these operators do exist
    friend inline std::istream &operator>>(std::istream& i, any &obj)
    {
      return obj.table->stream_in(i, &obj.object);
    }

    friend inline std::ostream &operator<<(std::ostream& o, const any &obj)
    {
      return obj.table->stream_out(o, &obj.object);
    }

  public:  // Types
    template<class T> friend T *any_cast(any *);

  private:  // Assignment and casting
    template<class T> any &assign(const T &);

    any &assign(const any &x)
    {
      if (&x != this)
      {
        // Are we copying between the same type?
        if (table == x.table)  // if so, we can avoid reallocation
          table->move(&x.object, &object);
        else
        {
          clear();
          x.table->clone(&x.object, &object);
          table = x.table;
        }
      }

      return *this;
    }

    template<class T>
    const T &cast() const
    {
      if (type() != typeid(T))
        throw bad_any_cast(type(), typeid(T));

      return detail::any_::get_table<T>::is_small ?
             *reinterpret_cast<const T *>(&object) :
             *reinterpret_cast<const T *>(object);
    }

  private:  // Private data members
    detail::any_::fxn_ptr_table *table;

    void *object = nullptr;
  };  // class any
#endif  // !defined(USE_BOOST_ANY)

  template<class T> T to(const any &);

#include "kernel/any_inl.h"

}  // namespace vita

#endif  // Include guard
