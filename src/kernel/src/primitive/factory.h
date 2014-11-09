/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_PRIMITIVE_FACTORY_H)
#define      VITA_PRIMITIVE_FACTORY_H

#include <map>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>

#include "kernel/symbol.h"
#include "kernel/utility.h"

namespace vita
{
  ///
  /// \a symbol_factory is an abstract factory (the essence of the pattern is
  /// to provide an interface for creating families of related or dependent
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
  public:
    static symbol_factory &instance();

    std::unique_ptr<symbol> make(const std::string &, cvect = cvect{0});
    std::unique_ptr<symbol> make(domain_t, int, int, category_t = 0);

    unsigned args(const std::string &) const;

    template<class> bool register_symbol(const std::string &, unsigned);
    bool unregister_symbol(const std::string &);

  private:  // Private support methods
    symbol_factory();
    DISALLOW_COPY_AND_ASSIGN(symbol_factory);

    using build_func = std::unique_ptr<symbol> (*)(const cvect &);

    template<class T> static std::unique_ptr<symbol> build(const cvect &c)
    { return vita::make_unique<T>(c); }

  private:  // Private data members
    using map_key = std::string;

    struct build_info
    {
      build_func f;
      unsigned   n;
    };

    std::map<map_key, build_info> factory_;
  };

  ///
  /// \param[in] name name of the symbol to be registered (UPPERCASE!).
  /// \param[in] n number of argument for the constructor of the symbol.
  /// \return \c true if the symbol \a T has been added to the factory.
  ///
  template<class T>
  bool symbol_factory::register_symbol(const std::string &name, unsigned n)
  {
    const map_key k(boost::to_upper_copy(name));

    const bool missing(factory_.find(k) == factory_.end());

    if (missing)
      factory_[k] = {&build<T>, n};

    return missing;
  }
}  // namespace vita

#endif  // Include guard
