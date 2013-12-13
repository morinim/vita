/**
 *
 *  \file evolution_summary.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_SUMMARY_H)
#define      EVOLUTION_SUMMARY_H

#include "kernel/analyzer.h"

namespace vita
{
  template<class T>
  class summary
  {
  public:  // Constructor and support functions.
    summary();

    void clear();

  public:   // Serialization.
    bool load(std::istream &, const environment &, const symbol_set &);
    bool save(std::ostream &) const;

  public:  // Public data members.
    analyzer<T> az;

    struct best_
    {
      T             ind;
      fitness_t fitness;
    };

    boost::optional<best_> best;

    double speed;

    std::uintmax_t crossovers, mutations;

    unsigned gen, last_imp;
  };

#include "kernel/evolution_summary_inl.h"
}  // namespace vita

#endif  // EVOLUTION_SUMMARY_H
