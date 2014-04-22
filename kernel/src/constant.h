/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_CONSTANT_H)
#define      VITA_SRC_CONSTANT_H

#include "kernel/terminal.h"

namespace vita
{
  ///
  /// \brief A constant value (bool, int, double, string)
  ///
  template<class T>
  class constant : public terminal
  {
  public:
    explicit constant(const std::string &c, category_t t = 0)
      : terminal(c, t, false, false, k_base_weight),
        val_(lexical_cast<T>(c)) {}
    explicit constant(T c, category_t t = 0)
      : constant(std::to_string(c), t) {}

    ///
    /// \return the value of the constant (as a \c any).
    ///
    /// The argument is not used: the value of a constant is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    virtual any eval(interpreter<individual> *) const override
    { return any(val_); }

  private:  // Private data members
    T val_;
  };

  template<>
  class constant<std::string> : public terminal
  {
  public:
    explicit constant(const std::string &c, category_t t = 0)
      : terminal("\"" + c + "\"", t, false, false, k_base_weight), val_(c) {}
    explicit constant(const char c[], category_t t = 0)
      : constant(std::string(c), t) {}

    ///
    /// \return the value of the constant (as a \c any).
    ///
    /// The argument is not used: the value of a constant is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    virtual any eval(interpreter<individual> *) const override
    { return any(val_); }

  private:  // Private data members.
    std::string val_;
  };
}  // namespace vita
#endif // Include guard
