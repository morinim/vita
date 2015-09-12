/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/log.h"

namespace vita
{
log print;

log::log() : level_(L_ALL)
{
}

log &log::verbosity(level l)
{
  level_ = l;
  return *this;
}

log::level log::verbosity() const
{
  return level_;
}

}  // namespace vita
