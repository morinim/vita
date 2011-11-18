/**
 *
 *  \file factory.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(PRIMITIVE_FACTORY_H)
#define      PRIMITIVE_FACTORY_H

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

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
      : terminal(c, t, false, false, default_weight*2), val(c) {}

    /// The argument is not used: the value of a constant is stored with the
    /// class, we don't need an interpreter to discover it.
    boost::any eval(interpreter *) const { return val; }

  private:
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
  class symbol_factory
  {
  private:
    symbol_factory();

    typedef std::shared_ptr<symbol> (*make_func1)(category_t);
    typedef std::shared_ptr<symbol> (*make_func2)(category_t, category_t);
    typedef std::pair<std::string, domain_t> map_key;
    typedef std::pair<unsigned, make_func1> map_data1;
    typedef std::pair<unsigned, make_func2> map_data2;

    template<typename T> static std::shared_ptr<symbol> make1(category_t c)
    { return std::make_shared<T>(c); }

    template<typename T> static std::shared_ptr<symbol> make2(category_t c1,
                                                              category_t c2)
    { return std::make_shared<T>(c1, c2); }

    std::map<map_key, map_data1> factory1_;
    std::map<map_key, map_data2> factory2_;

  public:
    ///
    /// \return an instance of the singleton object symbol_factory.
    ///
    static symbol_factory &instance()
    {
      static symbol_factory singleton;
      return singleton;
    }

    std::shared_ptr<symbol> make(
      const std::string &, domain_t,
      const std::vector<category_t> & = std::vector<category_t>());
    std::shared_ptr<symbol> make(const std::string &, domain_t, int, int,
                                 category_t = 0);

    unsigned args(const std::string &, domain_t) const;

    template<typename T> bool register_symbol1(const std::string &, domain_t);
    template<typename T> bool register_symbol2(const std::string &, domain_t);

    bool unregister_symbol(const std::string &, domain_t);
  };

  ///
  /// \param[in] name name of the symbol to be registered (UPPERCASE!).
  /// \param[in] domain of the symbol.
  /// \return \c true if the symbol \a T has been added to the factory.
  ///
  template<typename T>
  bool symbol_factory::register_symbol1(const std::string &name,
                                        domain_t d)
  {
    const std::string un(boost::to_upper_copy(name));
    const map_key k{un, d};

    if (factory1_.find(k) != factory1_.end())
      return false;

    factory1_[k] = {1, &make1<T>};

    return true;
  }

  template<typename T>
  bool symbol_factory::register_symbol2(const std::string &name,
                                        domain_t d)
  {
    const std::string un(boost::to_upper_copy(name));
    const map_key k{un, d};

    if (factory2_.find(k) != factory2_.end())
      return false;

    factory2_[k] = {2, &make2<T>};

    return true;
  }
}  // namespace vita

#endif  // PRIMITIVE_FACTORY_H
