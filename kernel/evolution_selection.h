/**
 *
 *  \file evolution_selection.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_SELECTION_H)
#define      EVOLUTION_SELECTION_H

#include <vector>

#include "vita.h"

namespace vita
{
  class evolution;

  ///
  /// The selection strategy (tournament, fitness proportional...) for the
  /// \a evolution \c class. In the strategy design pattern, this \c class is
  /// the strategy interface and \a evolution is the context.
  ///
  class selection_strategy
  {
  public:
    explicit selection_strategy(const evolution *const);
    virtual ~selection_strategy() {}

    virtual std::vector<index_t> operator()() = 0;

  protected:
    const evolution *const evo_;
  };

  ///
  /// selection_factory \c class creates a new \a selection_strategy (the
  /// strategy) for the \a evolution \c class (the context).
  ///
  class selection_factory
  {
  public:
    enum strategy {k_tournament = 0};

    explicit selection_factory(const evolution *const);
    ~selection_factory();

    selection_strategy &operator[](unsigned);
    unsigned add(selection_strategy *const);

  private:
    std::vector<selection_strategy *> strategy_;
  };
}  // namespace vita

#endif  // EVOLUTION_SELECTION_H
