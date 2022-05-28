/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020, 2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/primitive/factory.h"
#include "kernel/gp/src/primitive/int.h"
#include "kernel/gp/src/primitive/real.h"
#include "kernel/gp/src/primitive/string.h"
#include "kernel/gp/src/constant.h"

namespace vita
{

namespace
{
///
/// Identifies the domain of a term.
///
/// \param[in] s term to be tested
/// \return      the domain of `s`
///
domain_t find_domain(const std::string &s)
{
  if (is_number(s))
    return s.find('.') == std::string::npos ? domain_t::d_int
                                            : domain_t::d_double;
  return domain_t::d_string;
}
}  // unnamed namespace

///
/// The factory is preloaded with a number of common symbols.
///
symbol_factory::symbol_factory()
{
  register_symbol<real::abs>    ("FABS", 1);
  register_symbol<real::add>    ("FADD", 1);
  register_symbol<real::aq>     ("FAQ", 1);
  register_symbol<real::cos>    ("FCOS", 1);
  register_symbol<real::div>    ("FDIV", 1);
  register_symbol<real::idiv>   ("FIDIV", 1);
  register_symbol<real::ife>    ("FIFE", 2);
  register_symbol<real::ifl>    ("FIFL", 2);
  register_symbol<real::ifz>    ("FIFZ", 1);
  register_symbol<real::length> ("FLENGTH", 2);
  register_symbol<real::ln>     ("FLN", 1);
  register_symbol<real::max>    ("FMAX", 1);
  register_symbol<real::mod>    ("FMOD", 1);
  register_symbol<real::mul>    ("FMUL", 1);
  register_symbol<real::integer>("REAL", 1);
  register_symbol<real::sigmoid>("FSIGMOID", 1);
  register_symbol<real::sin>    ("FSIN", 1);
  register_symbol<real::sqrt>   ("FSQRT", 1);
  register_symbol<real::sub>    ("FSUB", 1);

  register_symbol<integer::add>   ("ADD", 1);
  register_symbol<integer::div>   ("DIV", 1);
  register_symbol<integer::ife>   ("IFE", 2);
  register_symbol<integer::ifl>   ("IFL", 2);
  register_symbol<integer::ifz>   ("IFZ", 1);
  register_symbol<integer::mod>   ("MOD", 1);
  register_symbol<integer::mul>   ("MUL", 1);
  register_symbol<integer::number>("INT", 1);
  register_symbol<integer::shl>   ("SHL", 1);
  register_symbol<integer::sub>   ("SUB", 1);

  register_symbol<str::ife>("SIFE", 2);
}

///
/// Creates a specific instance of a symbol.
///
/// \param[in] name name of the symbol to be created (case sensitive)
/// \param[in] c    a list of categories used by the the symbol constructor
/// \return         an abstract pointer to the created symbol
///
/// As the factory only returns an abstract pointer, the client code (which
/// requests the object from the factory) does not know - and is not burdened
/// by - the actual concrete type of the object which was just created.
/// However, the type of the concrete object is known by the abstract
/// factory via the `name` and `c` arguments.
///
/// \attention
/// If `name` is not recognized as a preregistered symbol, it's registered on
/// the fly as a constant.
///
/// \note
/// * The client code has no knowledge whatsoever of the concrete type, not
///   needing to include any header files or class declarations relating to
///   the concrete type. The client code deals only with the abstract type.
///   Objects of a concrete type are indeed created by the factory, but
///   the client code accesses such objects only through their abstract
///   interface.
/// * Adding new concrete types is done by modifying the client code to use
///   a different factory, a modification which is typically one line in
///   one file (the different factory then creates objects of a different
///   concrete type, but still returns a pointer of the same abstract type
///   as before - thus insulating the client code from change). This is
///   significantly easier than modifying the client code to instantiate a
///   new type, which would require changing every location in the code
///   where a new object is created (as well as making sure that all such
///   code locations also have knowledge of the new concrete type, by
///   including for instance a concrete class header file).
///
std::unique_ptr<symbol> symbol_factory::make(const std::string &name, cvect c)
{
  Expects(!name.empty());
  Expects(!c.empty());

  if (const auto it = factory_.find(name); it != factory_.end())
  {
    while (c.size() < it->second.args)
      c.push_back(category_t(0));

    return it->second.make(c);
  }

  switch (find_domain(name))
  {
  case domain_t::d_double:
    return std::make_unique<constant<double>>(name, c[0]);
  case domain_t::d_int:
    return std::make_unique<constant<int>>(name, c[0]);
  case domain_t::d_string:
    return std::make_unique<constant<std::string>>(name, c[0]);
  default:
    return nullptr;
  }
}

///
/// Creates an instance of a number.
///
/// \param[in] d   domain of the symbol
/// \param[in] min lower bound for the number value
/// \param[in] max upper bound for the number value
/// \param[in] c   a category used by the symbol constructor
/// \return        an abstract pointer to the created symbol
///
/// This is an alternative way to build a number. The other `make` method finds
/// the domain of the number checking the input string's format.
///
/// Here there're explicit parameters for domain / category and we can also
/// specify a range (`[min; max]`).
///
std::unique_ptr<symbol> symbol_factory::make(domain_t d, int min, int max,
                                             category_t c)
{
  Expects(d == domain_t::d_double || d == domain_t::d_int);

  switch (d)
  {
  case domain_t::d_double:
    return std::make_unique<real::integer>(cvect{c}, min, max);
  case domain_t::d_int:
    return std::make_unique<integer::number>(cvect{c}, min, max);
  default:
    return nullptr;
  }
}

///
/// \param[in] name name of the symbol (case sensitive)
/// \return         number of distinct categories needed to build the symbol
///
std::size_t symbol_factory::args(const std::string &name) const
{
  const auto it(factory_.find(name));

  return it == factory_.end() ? 1 : it->second.args;
}

///
/// Unregister the symbol from the factory.
///
/// \param[in] name name of the symbol (case sensitive)
/// \return         `true` if the symbol has been unregistered
///
/// \note
/// Constants and variable aren't registered in the factory, so they cannot be
/// unregistered.
///
bool symbol_factory::unregister_symbol(const std::string &name)
{
  return factory_.erase(name) == 1;
}

}  // namespace vita
