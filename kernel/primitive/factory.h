/**
 *
 *  \file factory.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(PRIMITIVE_FACTORY_H)
#define      PRIMITIVE_FACTORY_H

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

#include <map>

#include "kernel/terminal.h"

namespace vita
{
  ///
  /// A variable is an input argument for a symbolic regression or
  /// classification problem.
  ///
  class variable : public terminal
  {
  public:
    explicit variable(const std::string &name, category_t t = 0)
      : terminal(name, t, true) {}

    boost::any eval(vita::interpreter *) const { return val; }

  public:  // Data members.
    boost::any val;
  };

  class constant : public terminal
  {
    //private:
    // C++11 allows constructors to call other peer constructors (known as
    // delegation). This allows constructors to utilize another constructor's
    // behavior with a minimum of added code.
    //constant(const std::string &name, const boost::any &c, category_t = 0)
    //  : terminal(name, t, false, false, default_weight*2), val(c) {}
  public:
    explicit constant(bool c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight*2), val(c) {}
    explicit constant(double c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight*2), val(c) {}
    explicit constant(int c, category_t t = 0)
      : terminal(boost::lexical_cast<std::string>(c), t, false, false,
                 default_weight*2), val(c) {}
    explicit constant(const std::string &c, category_t t = 0)
      : terminal("\""+c+"\"", t, false, false, default_weight*2), val(c) {}

    /// The argument is not used: the value of a constant is stored with the
    /// class, we don't need an interpreter to discover it.
    boost::any eval(interpreter *) const { return val; }

  private:  // Data members.
    const boost::any val;
  };

  ///
  /// \a symbol_factory is an abstract factory (the essence of the pattern is to
  /// provide an interface for creating families of related or dependent
  /// objects, i.e. symbols, without specifying ther concrete classes, e.g.
  /// numbers, functions...).
  /// The factory determines the actual concrete type of the symbol to be
  /// created and it is here that the object is actually created. However,
  /// the factory only returns an abstract pointer to the created concrete
  /// object.
  /// This insulates client code from object creation by having clients ask a
  /// factory object to create an object of the desired abstract type and to
  /// return an abstract pointer to the object.
  ///
  class symbol_factory : boost::noncopyable
  {
  public:
    static symbol_factory &instance();

    symbol_ptr make(
      const std::string &,
      const std::vector<category_t> & = std::vector<category_t>());
    symbol_ptr make(domain_t, int, int, category_t = 0);

    unsigned args(const std::string &) const;

    template<typename T> bool register_symbol1(const std::string &);
    template<typename T> bool register_symbol2(const std::string &);

    bool unregister_symbol(const std::string &);

  private:
    symbol_factory();

    typedef symbol_ptr (*make_func1)(category_t);
    typedef symbol_ptr (*make_func2)(category_t, category_t);

    template<typename T> static symbol_ptr make1(category_t c)
    { return std::make_shared<T>(c); }

    template<typename T> static symbol_ptr make2(category_t c1, category_t c2)
    { return std::make_shared<T>(c1, c2); }

  private:  // Data members.
    typedef std::string map_key;

    std::map<map_key, make_func1> factory1_;
    std::map<map_key, make_func2> factory2_;
  };

  ///
  /// \param[in] name name of the symbol to be registered (UPPERCASE!).
  /// \return \c true if the symbol \a T has been added to the factory.
  ///
  template<typename T>
  bool symbol_factory::register_symbol1(const std::string &name)
  {
    const map_key k(boost::to_upper_copy(name));

    const bool missing(factory1_.find(k) == factory1_.end());

    if (missing)
      factory1_[k] = &make1<T>;

    return missing;
  }

  template<typename T>
  bool symbol_factory::register_symbol2(const std::string &name)
  {
    const map_key k(boost::to_upper_copy(name));

    const bool missing(factory2_.find(k) == factory2_.end());

    if (missing)
      factory2_[k] = &make2<T>;

    return missing;
  }
}  // namespace vita

#endif  // PRIMITIVE_FACTORY_H
