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

#if !defined(VITA_ANY_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_ANY_TCC)
#define      VITA_ANY_TCC

#if !defined(USE_BOOST_ANY)
///
/// \param[in] x any will contain object \a x.
///
/// Makes a copy of \a x, so that the initial content of the new instance is
/// equivalent in both type and value to \a x.
///
template <class T>
any::any(const T &x) : table(detail::any_::get_table<T>::get())
{
  if (detail::any_::get_table<T>::is_small)
    new (&object) T(x);
  else
    object = new T(x);
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
/// \return \c this object after assignment.
///
/// Forwards \a rhs, discarding previous content, so that the new content of is
/// equivalent in both type and value to \a rhs before forward.
///
template <class T>
any &any::operator=(T &&rhs)
{
  return assign(std::forward<T>(rhs));
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
/// \return the value contained in \a operand.
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
/// \return a constant reference to the value contained in \a operand.
///
template<class T>
const T &any_cast(const any &operand)
{
  using nonref = typename std::remove_reference<T>::type;

  return any_cast<const nonref &>(const_cast<any &>(operand));
}
#endif  // !defined(USE_BOOST_ANY)

#endif  // Include guard
