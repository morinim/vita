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

#if !defined(VITA_ARGUMENT_H)
#define      VITA_ARGUMENT_H

#include <string>

#include "kernel/terminal.h"

namespace vita
{
  template<class T> class interpreter;

  ///
  /// \a argument is a special \a terminal used by \a adf functions for
  /// input parameters passing.
  ///
  class argument : public terminal
  {
  public:
    explicit argument(unsigned);

    virtual std::string display() const override;

    unsigned index() const;

    virtual any eval(interpreter<i_mep> *) const override;

    virtual bool debug() const override;

  private:
    const unsigned index_;
  };
}  // namespace vita

#endif  // Include guard
