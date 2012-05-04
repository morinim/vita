/**
 *
 *  \file argument.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(ARGUMENT_H)
#define      ARGUMENT_H

#include <string>

#include "kernel/vita.h"
#include "kernel/terminal.h"

namespace vita
{
  class interpreter;

  ///
  /// \a argument is a special \a terminal used by \a adf functions for
  /// input parameters passing.
  ///
  class argument : public terminal
  {
  public:
    explicit argument(unsigned);

    std::string display() const;

    unsigned index() const;

    boost::any eval(interpreter *) const;

    bool check() const;

  private:
    const unsigned index_;
  };
}  // namespace vita

#endif  // ARGUMENT_H
