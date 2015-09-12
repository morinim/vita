/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014, 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ANY_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_ANY_TCC)
#define      VITA_ANY_TCC

#if !defined(USE_BOOST_ANY)
namespace detail { namespace any_ {

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
  // If this assertion fires you tried to insert from a std::istream
  // into an empty `hold_any` instance. This simply can't work, because
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
}}  // namespace detail::any_

/// \post `this->empty()`
inline any::any() : table(detail::any_::get_table<detail::any_::empty>::get())
{
}

/// Copy constructor that copies content of `x` into new instance, so that any
/// content is equivalent in both type and value to the content of `x` or empty
/// if `x` is empty.
inline any::any(const any &x)
  : table(detail::any_::get_table<detail::any_::empty>::get())
{
  assign(x);
}

///
/// \param[in] x any will contain object `x`.
///
/// Makes a copy of `x`, so that the initial content of the new instance is
/// equivalent in both type and value to `x`.
///
template <class T>
any::any(const T &x) : table(detail::any_::get_table<T>::get())
{
  if (detail::any_::get_table<T>::is_small)
    new (&object) T(x);
  else
    object = new T(x);
}

/// \brief Releases any and all resources used in management of instance
inline any::~any()
{
  table->static_delete(&object);
}

inline any &any::assign(const any &x)
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
any &any::assign(const T &x)
{
  detail::any_::fxn_ptr_table *x_table(detail::any_::get_table<T>::get());

  // Are we copying between the same type?
  if (table == x_table)
  {
    // If so, we can avoid deallocating and re-use memory.
    table->destruct(&object);    // first destruct the old content
    if (detail::any_::get_table<T>::is_small)
      new (&object) T(x);  // create copy on-top of object pointer itself
    else
      new (object) T(x);   // create copy on-top of old version
  }
  else
  {
    if (detail::any_::get_table<T>::is_small)
    {
      // Create copy on-top of object pointer itself.
      table->destruct(&object);  // first destruct the old content
      new (&object) T(x);
    }
    else
    {
      clear();  // first delete the old content
      object = new T(x);
    }

    table = x_table;      // update table pointer
  }

  return *this;
}

///
/// \param[in] rhs new any value.
/// \return `this` object after assignment.
///
/// Forwards `rhs`, discarding previous content, so that the new content of is
/// equivalent in both type and value to `rhs` before forward.
///
template <class T>
any &any::operator=(T &&rhs)
{
  return assign(std::forward<T>(rhs));
}

///
/// \return the typeid of the contained value if instance is non-empty,
///         otherwise typeid(void).
///
/// \note
/// Useful for querying against types known either at compile time or only
/// at runtime.
///
inline const std::type_info &any::type() const
{
  return table->get_type();
}

///
/// return `true` if instance is empty, otherwise `false`.
///
inline bool any::empty() const
{
  return table == detail::any_::get_table<detail::any_::empty>::get();
}

///
/// \brief Resets the content of the any.
///
inline void any::clear()
{
  if (!empty())
  {
    table->static_delete(&object);
    table = detail::any_::get_table<detail::any_::empty>::get();
    object = nullptr;
  }
}

template<class T>
const T &any::cast() const
{
  if (type() != typeid(T))
    throw bad_any_cast(type(), typeid(T));

  return detail::any_::get_table<T>::is_small ?
    *reinterpret_cast<const T *>(&object) :
    *reinterpret_cast<const T *>(object);
}

// This function has been added in the assumption that the embedded
// type has a corresponding operator defined, which is completely safe
// because any is used only in contexts where these operators do exist
inline std::istream &operator>>(std::istream& i, any &obj)
{
  return obj.table->stream_in(i, &obj.object);
}

// This function has been added in the assumption that the embedded
// type has a corresponding operator defined, which is completely safe
// because any is used only in contexts where these operators do exist
inline std::ostream &operator<<(std::ostream& o, const any &obj)
{
  return obj.table->stream_out(o, &obj.object);
}

///
/// \brief Custom keyword cast for extracting a value of a given type from any
/// \param[in] operand a pointer to any.
/// \return a similarly qualified pointer to the value content if successful,
///         otherwise null is returned.
///
template<class T>
T *any_cast(any *operand)
{
  if (operand && operand->type() == typeid(T))
    return detail::any_::get_table<T>::is_small ?
      reinterpret_cast<T *>(&operand->object) :
      reinterpret_cast<T *>(operand->object);

  return nullptr;
}

///
/// \brief Custom keyword cast for extracting a value of a given type from any
/// \param[in] operand a pointer to any.
/// \return a similarly qualified pointer to the value content if successful,
///         otherwise null is returned.
///
template<class T>
const T *any_cast(const any *operand)
{
  return any_cast<T>(const_cast<any *>(operand));
}

///
/// \brief Custom keyword cast for extracting a value of a given type from any
/// \param operand an any.
/// \return the value contained in `operand`.
///
template<class T>
T any_cast(any &operand)
{
  using nonref = typename std::remove_reference<T>::type;

  nonref *result = any_cast<nonref>(&operand);
  if (!result)
    throw bad_any_cast(operand.type(), typeid(T));

  return *result;
}

///
/// \brief Custom keyword cast for extracting a value of a given type from any
/// \param operand an any.
/// \return a constant reference to the value contained in `operand`.
///
template<class T>
const T &any_cast(const any &operand)
{
  using nonref = typename std::remove_reference<T>::type;

  return any_cast<const nonref &>(const_cast<any &>(operand));
}
#endif  // !USE_BOOST_ANY guard

#endif  // Include guard
