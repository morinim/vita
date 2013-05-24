/**
 *
 *  \file argument.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "argument.h"
#include "gene.h"
#include "interpreter.h"

namespace vita
{
  ///
  /// \param[in] n argument index.
  ///
  /// An adf function may have up to \a k_args arguments. Arguments' category is
  /// special (here it is initialized with \c 0 but we could say they haven't a
  /// type) because arguments are communication channels among adf functions
  /// and their calling environments. So the type that is travelling on channel
  /// \c i (argument(i)) varies depending on the function being evaluated
  /// (instead, adf functions have a precise, fixed signature).
  ///
  argument::argument(size_t n)
    : terminal("ARG", category_t(0), false, false, 0), index_(n)
  {
    assert(debug());
  }

  ///
  /// \return the index of the argument.
  ///
  size_t argument::index() const
  {
    return index_;
  }

  ///
  /// \return the string representiation of the argument.
  ///
  std::string argument::display() const
  {
    return "ARG_" + boost::lexical_cast<std::string>(index_);
  }

  ///
  /// \param[in] agent current interpreter
  /// \return the value of the argument.
  ///
  any argument::eval(interpreter *agent) const
  {
    return agent->eval_adf_arg(index_);
  }

  ///
  /// \return \a true if the object passes the internal consistency check.
  ///
  bool argument::debug() const
  {
    return index_ < gene::k_args && terminal::debug();
  }
}  // Namespace vita
