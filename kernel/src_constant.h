/**
 *
 *  \file src_constant.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_CONSTANT_H)
#define      SRC_CONSTANT_H

#include <boost/lexical_cast.hpp>

#include "terminal.h"

namespace vita
{
  ///
  /// A constant value (bool, int, double, string).
  ///
  template<class T>
  class constant : public terminal
  {
  public:
    explicit constant(const std::string &c, category_t t = 0)
      : terminal(c, t, false, false, default_weight * 2),
        val_(boost::lexical_cast<T>(c)) {}

    explicit constant(T c, category_t t = 0)
      : terminal(std::to_string(c), t, false, false, default_weight * 2),
        val_(c) {}

  public:   // Serialization.
    virtual bool load(std::istream &);
    virtual bool save(std::ostream &) const;

    ///
    /// \return the value of the constant (as a \c any).
    ///
    /// The argument is not used: the value of a constant is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    virtual any eval(vita::interpreter *) const { return any(val_); }

  private:  // Private data members.
    T val_;
  };

  ///
  /// \return \c true if constant was loaded correctly.
  ///
  template<class T>
  bool constant<T>::load(std::istream &in)
  {
    const bool ok(symbol::load(in));

    in >> val_;

    return ok && in.good();
  }

  ///
  /// \return \c true if constant was saved correctly.
  ///
  template<class T>
  bool constant<T>::save(std::ostream &out) const
  {
    const bool ok(symbol::save(out));

    out << val_ << std::endl;

    return ok && out.good();
  }

  template<>
  class constant<std::string> : public terminal
  {
  public:
    explicit constant(const std::string &c, category_t t = 0)
      : terminal("\"" + c + "\"", t, false, false, default_weight * 2),
        val_(c) {}
    explicit constant(const char c[], category_t t = 0)
      : terminal("\"" + std::string(c) + "\"", t, false, false,
                 default_weight * 2), val_(c) {}

    ///
    /// \return the value of the constant (as a \c any).
    ///
    /// The argument is not used: the value of a constant is stored within the
    /// object and we don't need an \c interpreter to discover it.
    ///
    virtual any eval(vita::interpreter *) const { return any(val_); }

  public:   // Serialization.
    virtual bool load(std::istream &);
    virtual bool save(std::ostream &) const;

  private:  // Private data members.
    std::string val_;
  };

  ///
  /// \return \c true if constant<std::string> was loaded correctly.
  ///
  inline bool constant<std::string>::load(std::istream &in)
  {
    const bool ok(symbol::load(in));

    std::getline(in, val_);

    return ok && in.good();
  }

  ///
  /// \return \c true if constant<std::string> was saved correctly.
  ///
  inline bool constant<std::string>::save(std::ostream &out) const
  {
    const bool ok(symbol::save(out));

    out << val_ << std::endl;

    return ok && out.good();
  }
}  // namespace vita
#endif // SRC_CONSTANT_H
