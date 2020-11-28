/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_PRIMITIVE_FACTORY_H)
#define      VITA_PRIMITIVE_FACTORY_H

#include <map>

#include "kernel/gp/symbol.h"

namespace vita
{
///
/// An abstract factory for symbols.
///
/// The essence of the pattern is to provide an interface for creating families
/// of related or dependent objects, i.e. symbols, without specifying their
/// concrete classes (e.g. numbers, functions...).
///
/// The factory determines the actual concrete type of the symbol to be
/// created and it's here that the object is actually created. However,
/// the factory only returns an abstract pointer to the created concrete
/// object.
///
/// This insulates client code from object creation by having clients ask a
/// factory object to create an object of the desired abstract type and to
/// return an abstract pointer to the object.
///
class symbol_factory
{
public:
  symbol_factory();

  std::unique_ptr<symbol> make(const std::string &, cvect = {0});
  std::unique_ptr<symbol> make(domain_t, int, int, category_t = 0);

  template<class> bool register_symbol(const std::string &, std::size_t);
  bool unregister_symbol(const std::string &);

  std::size_t args(const std::string &) const;

private:
  using build_func = std::unique_ptr<symbol> (*)(const cvect &);

  template<class T> static std::unique_ptr<symbol> build(const cvect &c)
  { return std::make_unique<T>(c); }

  // Private data members.
  struct build_info
  {
    build_func  make;
    std::size_t args;
  };

  std::map<std::string, build_info> factory_;
};

///
/// Registers a new symbol inside the factory.
///
/// \param[in] name name of the symbol to be registered (case sensitive)
/// \param[in] n    number of arguments for the constructor of the symbol
/// \return         `true` if the symbol `T` has been added to the factory
///
/// \warning
/// `name` is a key: the function doesn't register different symbols with the
/// same name.
///
template<class T>
bool symbol_factory::register_symbol(const std::string &name, std::size_t n)
{
  return factory_.insert({name, build_info{build<T>, n}}).second;
}

}  // namespace vita

#endif  // include guard
