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

#if !defined(VITA_DETAIL_ANY_H)
#define      VITA_DETAIL_ANY_H

namespace vita { namespace detail { namespace any_ {

  struct empty {};

  // Function pointer table.
  struct fxn_ptr_table
  {
    const std::type_info &(*get_type)();
    void (*static_delete)(void **);
    void (*destruct)(void **);
    void (*clone)(void *const *, void **);
    void (*move)(void *const *, void **);
    std::istream &(*stream_in)(std::istream &, void **);
    std::ostream &(*stream_out)(std::ostream &, void *const *);
  };

  // Static functions for small value-types
  template<bool Small> struct fxns;

  template<> struct fxns<true>
  {
    template<class T>
    struct type
    {
      static constexpr const std::type_info &get_type() { return typeid(T); }

      static void static_delete(void **x) { reinterpret_cast<T *>(x)->~T(); }

      static void destruct(void **x) { reinterpret_cast<T *>(x)->~T(); }

      static void clone(void *const *src, void **dest)
      {
        new (dest) T(*reinterpret_cast<const T *>(src));
      }

      static void move(void *const *src, void **dest)
      {
        *reinterpret_cast<T *>(dest) = *reinterpret_cast<const T *>(src);
      }

      static std::istream &stream_in(std::istream &i, void **obj)
      {
        i >> *reinterpret_cast<T *>(obj);
        return i;
      }

      static std::ostream &stream_out(std::ostream &o, void *const *obj)
      {
        o << *reinterpret_cast<const T *>(obj);
        return o;
      }
    };  // struct type
  };  // struct fxns<true>

  // Static functions for big value-types (bigger than a void*)
  template<>
  struct fxns<false>
  {
    template<class T>
    struct type
    {
      static constexpr const std::type_info &get_type() { return typeid(T); }

      static void static_delete(void **x)
      {
        // destruct and free memory
        delete (*reinterpret_cast<T **>(x));
      }

      static void destruct(void **x)
      {
        // destruct only, we'll reuse memory
        (*reinterpret_cast<T **>(x))->~T();
      }

      static void clone(void *const *src, void **dest)
      {
        *dest = new T(**reinterpret_cast<T *const *>(src));
      }

      static void move(void *const *src, void **dest)
      {
        **reinterpret_cast<T **>(dest) = **reinterpret_cast<T *const *>(src);
      }

      static std::istream &stream_in(std::istream &i, void **obj)
      {
        i >> **reinterpret_cast<T**>(obj);
        return i;
      }

      static std::ostream &stream_out(std::ostream &o, void * const*obj)
      {
        o << **reinterpret_cast<T *const *>(obj);
        return o;
      }
    };  // struct type
  };  // struct fxns<false>

  template<class T>
  struct get_table
  {
    static constexpr bool is_small = sizeof(T) <= sizeof(void *);

    static fxn_ptr_table *get()
    {
      static fxn_ptr_table static_table =
      {
        fxns<is_small>::template type<T>::get_type,
        fxns<is_small>::template type<T>::static_delete,
        fxns<is_small>::template type<T>::destruct,
        fxns<is_small>::template type<T>::clone,
        fxns<is_small>::template type<T>::move,
        fxns<is_small>::template type<T>::stream_in,
        fxns<is_small>::template type<T>::stream_out
      };

      return &static_table;
    }
  };  // class get_table

  inline std::istream &operator>>(std::istream &i, empty &)
  {
    // If this assertion fires you tried to insert from a std istream
    // into an empty hold_any instance. This simply can't work, because
    // there is no way to figure out what type to extract from the
    // stream.
    // The only way to make this work is to assign an arbitrary
    // value of the required type to the hold_any instance you want to
    // stream to. This assignment has to be executed before the actual
    // call to the operator>>().
    assert(false);

    return i;
  }

  inline std::ostream & operator<<(std::ostream &o, const empty &)
  {
    return o;
  }
}}}  // namespace vita::detail::any

#endif  // Include guard
