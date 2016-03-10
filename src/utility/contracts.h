/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_CONTRACTS_H)
#define      VITA_CONTRACTS_H

#include <cassert>

namespace vita
{

#if defined(VITA_CONCEPTS_THROW)

#include <exception>
#include <stdexcept>

#define VITA_CONCEPT_STRING(x) #x

struct concept_fail : public std::runtime_error
{
  explicit concept_fail(const char message[]) : std::runtime_error(message) {}
};

#define Expects(expression)                                        \
  if (!(expression))                                               \
    throw concept_fail("VITA: precondition failure at " __FILE__   \
                       ": " VITA_CONCEPT_STRING(__LINE__));

#define Ensures(expression)                                        \
  if (!(expression))                                               \
    throw concept_fail("VITA: postcondition failure at " __FILE__  \
                       ": " VITA_CONCEPT_STRING(__LINE__));

#elif defined(NDEBUG)

#define Expects(expression)
#define Ensures(expression)

#else

/// Preconditions can be stated in many ways, including comments, `if`
/// statements and `assert()`. This can make them hard to distinguish from
/// ordinary code, hard to update, hard to manipulate by tools and may have
/// the wrong semantics (do you always want to abort in debug mode and check
/// nothing in productions runs?).
/// \see C++ Core Guidelines I.6 <https://github.com/isocpp/CppCoreGuidelines/>
#define Expects(expression)  assert(expression)

/// Postconditions are often informally stated in a comment that states the
/// purpose of a function; `Ensures()` can be used to make this more
/// systematic, visible and checkable.
/// Postconditions are especially important when they relate to something that
/// isn't directly reflected in a returned result, such as a state of a data
/// structure used.
/// \note
/// Postconditions of the form "this resource must be released" are best
/// expressed by RAII.
/// \see C++ Core Guidelines I.7 <https://github.com/isocpp/CppCoreGuidelines/>
#define Ensures(expression)  assert(expression)

#endif

}  // namespace vita

#endif  // include guard
