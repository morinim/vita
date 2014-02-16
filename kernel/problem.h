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

#if !defined(VITA_PROBLEM_H)
#define      VITA_PROBLEM_H

#include "kernel/data.h"
#include "kernel/environment.h"
#include "kernel/symbol_set.h"

namespace vita
{
  class problem
  {
  public:
    explicit problem(bool);

    /// \return an access point for the dataset.
    virtual vita::data *data() { return nullptr; }

    virtual void clear(bool);

    virtual bool debug(bool) const;

  public:  // Public data members.
    environment env;

    symbol_set sset;
  };
}  // namespace vita

#endif  // Include guard
