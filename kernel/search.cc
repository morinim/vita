/**
 *
 *  \file search.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/search.h"
#include "kernel/individual.h"

namespace vita {  namespace impl {
  ///
  /// \param[in] base \a individual we are examining to extract building blocks.
  /// \param[in] env the current environment.
  /// \param[in] fitness the active evaluator.
  /// \param[in] prob the current problem.
  ///
  /// Adaptive Representation through Learning (ARL). The algorithm extract
  /// common knowledge (building blocks) emerging during the evolutionary
  /// process and acquires the necessary structure for solving the problem
  /// (see ARL - Justinian P. Rosca and Dana H. Ballard).
  ///
  template<>
  void arl(const vita::individual &base, const environment &env,
           evaluator<vita::individual> &fitness, problem *prob)
  {
    const auto base_fit(fitness(base));
    if (!base_fit.isfinite())
      return;  // We need a finite fitness to search for an improvement

    // Logs ADFs
    const auto filename(env.stat_dir + "/" + environment::arl_filename);
    std::ofstream log(filename.c_str(), std::ios_base::app);
    if (env.stat_arl && log.good())
    {
      for (unsigned i(0); i < prob->sset.adts(); ++i)
      {
        const symbol &f(*prob->sset.get_adt(i));
        log << f.display() << ' ' << f.weight << std::endl;
      }
      log << std::endl;
    }

    const unsigned adf_args(0);
    auto blk_idx(base.blocks());
    for (const locus &l : blk_idx)
    {
      auto candidate_block(base.get_block(l));

      // Building blocks must be simple.
      if (candidate_block.eff_size() <= 5 + adf_args)
      {
        // This is an approximation of the fitness due to the current block.
        // The idea is to see how the individual (base) would perform without
        // (base.destroy_block) the current block.
        // Useful blocks have delta values greater than 0.
        const auto delta(base_fit[0] -
                         fitness(base.destroy_block(l.index))[0]);

        // Semantic introns cannot be building blocks...
        // When delta is greater than 10% of the base fitness we have a
        // building block.
        if (std::isfinite(delta) && std::fabs(base_fit[0] / 10.0) < delta)
        {
          std::unique_ptr<symbol> p;
          if (adf_args)
          {
            std::vector<locus> replaced;
            auto generalized(candidate_block.generalize(adf_args, &replaced));
            std::vector<category_t> categories(replaced.size());
            for (unsigned j(0); j < replaced.size(); ++j)
              categories[j] = replaced[j].category;

            p = make_unique<adf>(generalized, categories, 10);
          }
          else  // !adf_args
            p = make_unique<adt>(candidate_block, 100);

          if (env.stat_arl && log.good())
          {
            log << p->display() << " (Base: " << base_fit
                << "  DF: " << delta
                << "  Weight: " << std::fabs(delta / base_fit[0]) * 100.0
                << "%)" << std::endl;
            candidate_block.list(log);
            log << std::endl;
          }

          prob->sset.insert(std::move(p));
        }
      }
    }
  }
} }  // namespace vita::impl
