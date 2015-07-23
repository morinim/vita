/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
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
#include "kernel/vitafwd.h"

namespace vita
{
///
/// `argument` is a special `terminal` used by adf functions for
/// input parameters passing.
///
class argument : public terminal
{
public:
  explicit argument(unsigned);

  virtual std::string display() const override;

  unsigned index() const;

  virtual any eval(core_interpreter *) const override;

  virtual bool debug() const override;

private:
  const unsigned index_;
};  // class argument

}  // namespace vita

#endif  // Include guard
