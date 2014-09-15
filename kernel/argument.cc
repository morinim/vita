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

#include "kernel/argument.h"
#include "kernel/gene.h"
#include "kernel/i_mep.h"
#include "kernel/interpreter.h"

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
  argument::argument(unsigned n) : terminal("ARG", category_t(0)), index_(n)
  {
    weight = 0;

    assert(debug());
  }

  ///
  /// \return the index of the argument.
  ///
  unsigned argument::index() const
  {
    return index_;
  }

  ///
  /// \return the string representiation of the argument.
  ///
  std::string argument::display() const
  {
    return "ARG_" + std::to_string(index_);
  }

  ///
  /// \param[in] agent current interpreter
  /// \return the value of the argument.
  ///
  any argument::eval(core_interpreter *agent) const
  {
    assert(typeid(*agent) == typeid(interpreter<i_mep>));

    return static_cast<interpreter<i_mep> *>(agent)->fetch_adf_arg(index_);
  }

  ///
  /// \return \a true if the object passes the internal consistency check.
  ///
  bool argument::debug() const
  {
    return index_ < gene::k_args && terminal::debug();
  }
}  // Namespace vita
