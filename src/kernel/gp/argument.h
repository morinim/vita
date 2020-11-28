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

#if !defined(VITA_ARGUMENT_H)
#define      VITA_ARGUMENT_H

#include <string>

#include "kernel/vitafwd.h"
#include "kernel/gp/terminal.h"

namespace vita
{
///
/// A special `terminal` used by adf functions for input parameters passing.
///
class argument : public terminal
{
public:
  explicit argument(unsigned);

  std::string name() const override;

  unsigned index() const;

  value_t eval(core_interpreter *) const override;

  bool is_valid() const override;

private:
  const unsigned index_;
};  // class argument

}  // namespace vita

#endif  // include guard
