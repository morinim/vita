/**
 *
 *  \file evolution_replacement.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_REPLACEMENT_H)
#define      EVOLUTION_REPLACEMENT_H

#include <vector>

#include "kernel/vita.h"
#include "kernel/individual.h"

namespace vita
{
  class evolution;
  class summary;

  ///
  /// The replacement strategy (random, elitist...) for the \a evolution
  /// \c class. In the strategy design pattern, this \c class is the strategy
  /// interface and \a evolution is the context.
  ///
  class replacement_strategy
  {
  public:
    explicit replacement_strategy(evolution *const);

    virtual void operator()(const std::vector<index_t> &,
                            const std::vector<individual> &,
                            summary *const) = 0;

  protected:
    evolution *const evo_;
  };

  ///
  /// replacement_factory \c class creates a new \a replacement_strategy (the
  /// strategy) for the \a evolution \c class (the context).
  ///
  class replacement_factory
  {
  public:
    enum replacement {tournament = 0};

    explicit replacement_factory(evolution *const);
    ~replacement_factory();

    replacement_strategy &operator[](unsigned);
    unsigned add(replacement_strategy *const);

  private:
    std::vector<replacement_strategy *> strategy_;
  };
}  // namespace vita

#endif  // EVOLUTION_REPLACEMENT_H
