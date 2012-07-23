/**
 *
 *  \file classifier.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(CLASSIFIER_H)
#define      CLASSIFIER_H

#include <string>

#include "kernel/data.h"
#include "kernel/individual.h"

namespace vita
{
  class classifier
  {
  public:
    explicit classifier(const individual &ind) : ind_(ind)
    { assert(ind.check()); }

    virtual std::string operator()(const data::example &) const = 0;

  protected:
    individual ind_;
  };
}  // namespace vita

#endif  // CLASSIFIER_H
