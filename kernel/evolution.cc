/**
 *
 *  \file evolution.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

#include "kernel/evolution.h"
#include "kernel/environment.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// \param[in] env environment (mostly used for population initialization).
  /// \param[in] eva evaluator used during the evolution.
  ///
  evolution::evolution(environment *const env, evaluator *const eva)
    : selection(this), operation(this, &stats_), replacement(this), pop_(env),
      eva_(new evaluator_proxy(eva, env->ttable_size))
  {
    assert(eva);

    assert(check());
  }

  ///
  /// \return access to the population being evolved.
  ///
  vita::population &evolution::population()
  {
    return pop_;
  }

  ///
  /// \return constant reference to the population being evolved.
  ///
  const vita::population &evolution::population() const
  {
    return pop_;
  }

  ///
  /// \param[out] az repository for the statistical informations.
  ///
  /// Gathers statistical informations about the elements of the population.
  ///
  void evolution::pick_stats(analyzer *const az)
  {
    az->clear();

    for (unsigned i(0); i < pop_.size(); ++i)
      az->add(pop_[i], (*eva_)(pop_[i]));
  }

  void evolution::pick_stats()
  {
    pick_stats(&stats_.az);
  }

  ///
  /// \param[out] probes number of probes in the transposition table.
  /// \param[out] hits number of hits in the transposition table (hits <=
  ///             probes).
  ///
  void evolution::get_probes(boost::uint64_t *probes,
                             boost::uint64_t *hits) const
  {
    *probes = *hits = 0;

    if (typeid(*eva_) == typeid(evaluator_proxy))
    {
      *probes = static_cast<evaluator_proxy *>(eva_.get())->probes();
      *hits   = static_cast<evaluator_proxy *>(eva_.get())->hits();
    }
  }

  ///
  /// \param[in] run_count run number.
  ///
  /// Saves working / statistical informations in a log file.
  /// Data are written in a CSV-like fashion and are partitioned in blocks
  /// separated by two blank lines:
  /// [BLOCK_1]\n\n
  /// [BLOCK_2]\n\n
  /// ...
  /// [BLOCK_x]
  /// where each block is a set of line like this:
  /// data_1 [space] data_2 [space] ... [space] data_n
  /// We use this format, instead of XML, because statistics are produced
  /// incrementally and so it's simple and fast to append new data to a
  /// CSV-like file. Note also that data sets are ready to be plotted by
  /// GNUPlot.
  ///
  void evolution::log(unsigned run_count) const
  {
    static unsigned last_run(0);

    if (pop_.env().stat_dynamic)
    {
      const std::string f_dynamic(pop_.env().stat_dir + "/dynamic");
      std::ofstream dynamic(f_dynamic.c_str(), std::ios_base::app);
      if (dynamic.good())
      {
        if (last_run != run_count)
        {
          dynamic << std::endl << std::endl;
          last_run = run_count;
        }

        boost::uint64_t hits(0), probes(0);
        get_probes(&probes, &hits);

        dynamic << run_count
                << ' ' << stats_.gen
                << ' ' << stats_.f_best
                << ' ' << stats_.az.fit_dist().mean
                << ' ' << stats_.az.fit_dist().standard_deviation()
                << ' ' << stats_.az.fit_dist().entropy()
                << ' ' << stats_.az.fit_dist().min
                << ' ' << unsigned(stats_.az.length_dist().mean)
                << ' ' << stats_.az.length_dist().standard_deviation()
                << ' ' << unsigned(stats_.az.length_dist().max)
                << ' ' << stats_.mutations
                << ' ' << stats_.crossovers
                << ' ' << stats_.az.functions(0)
                << ' ' << stats_.az.terminals(0)
                << ' ' << stats_.az.functions(1)
                << ' ' << stats_.az.terminals(1)
                << ' ' << hits
                << ' ' << probes;

        for (unsigned active(0); active <= 1; ++active)
          for (analyzer::const_iterator i(stats_.az.begin());
               i != stats_.az.end();
               ++i)
            dynamic << ' ' << (i->first)->display() << ' '
                    << i->second.counter[active];

        dynamic << " \"";
        stats_.best->in_line(dynamic);
        dynamic << '"' << std::endl;
      }
    }
  }

  ///
  /// \return true if evolution should be interrupted.
  ///
  bool evolution::stop_condition() const
  {
    return
      (pop_.env().g_since_start && stats_.gen > pop_.env().g_since_start) ||

      // We use an accelerated stop condition when all the individuals have
      // the same fitness and after gwi/2 generations the situation isn't
      // changed.
      (pop_.env().g_without_improvement &&
       (stats_.gen-stats_.last_imp > pop_.env().g_without_improvement ||
        (stats_.gen-stats_.last_imp > pop_.env().g_without_improvement/2 &&
         stats_.az.fit_dist().variance <= float_epsilon)));
  }

  ///
  /// \param[in] ind individual whose fitness we are interested in.
  /// \return the fitness of \a ind.
  ///
  fitness_t evolution::fitness(const individual &ind) const
  {
    return (*eva_)(ind);
  }

  ///
  /// \param[in] ind individual whose fitness we are interested in.
  /// \return the fitness of \a ind.
  ///
  fitness_t evolution::fast_fitness(const individual &ind) const
  {
    return eva_->fast(ind);
  }

  ///
  /// \param[in] ind individual whose accuracy we are interested in.
  /// \return the accuracy of \a ind (on the current test set).
  ///
  double evolution::accuracy(const individual &ind) const
  {
    return eva_->accuracy(ind);
  }

  ///
  /// \param[in] verbose if \c true prints verbose informations.
  /// \param[in] run_count run number (used for print and log).
  /// \param[in] sel_id index of the active selection strategy.
  /// \param[in] op_id index of the active operation strategy.
  /// \param[in] rep_id index of the active replacement strategy.
  ///
  /// the genetic programming loop. We begin the loop by choosing a
  /// genetic operation: reproduction, mutation or crossover. We then select
  /// the individual(s) to participate in the genetic operation using
  /// tournament selection. If we are doing reproduction or mutation, we only
  /// select one individual. For crossover, two individuals need to be selected.
  /// The genetic operation is then performed and a new offspring individual is
  /// created.
  /// The offspring is then placed into the original population (steady state)
  /// replacing a bad individual.
  /// This whole process repeats until the termination criteria is satisfied.
  /// With any luck, this process will produce an individual that solves the
  /// problem at hand.
  ///
  const summary &evolution::operator()(bool verbose, unsigned run_count,
                                       unsigned sel_id, unsigned op_id,
                                       unsigned rep_id)
  {
    stats_.clear();
    stats_.best.reset(new individual(pop_[0]));
    stats_.f_best    = (*eva_)(*stats_.best);
    stats_.accu_best =                   0.0;

    eva_->clear();

    std::clock_t start_c(clock());
    for (stats_.gen = 0; !stop_condition(); ++stats_.gen)
    {
      pick_stats(&stats_.az);

      log(run_count);

      for (unsigned k(0); k < pop_.size(); ++k)
      {
        if ( verbose && k % std::max(pop_.size()/100, size_t(1)) )
        {
          std::cout << "Run " << run_count << '.'
                    << stats_.gen << " (" << std::setw(3) << 100*k/pop_.size()
                    << "%)\r" << std::flush;
        }

        // --------- SELECTION ---------
        std::vector<locus_t> parents(selection[sel_id]());

        // --------- CROSSOVER / MUTATION ---------
        std::vector<individual> off(operation[op_id](parents));

        // --------- REPLACEMENT --------
        const fitness_t before(stats_. f_best);
        replacement[rep_id](parents, off, &stats_);

        if (verbose && stats_.f_best != before)
        {
          std::cout << "Run " << run_count << '.' << std::setw(6)
                    << stats_.gen << " (" << std::setw(3)
                    << 100*k/pop_.size() << "%): fitness "
                    << std::setw(16) << stats_.f_best;

          if (stats_.accu_best >= 0.0)
            std::cout << std::setprecision(2) << " (" << std::fixed
                      << std::setw(6) << 100.0*stats_.accu_best << "%)"
                      << std::setprecision(-1)
                      << std::resetiosflags(std::ios::fixed);

          std::cout << std::endl;
        }
      }

      get_probes(&stats_.ttable_probes, &stats_.ttable_hits);
    }

    if (verbose)
    {
      double speed(0);

      if (clock() > start_c)
        speed = static_cast<double>(pop_.size()*stats_.gen) * CLOCKS_PER_SEC
                / (clock()-start_c);

      std::string unit("");
      if (speed >= 10)
        unit = "n/s";
      else if (speed >= 1)
      {
        speed *= 3600;
        unit = "n/h";
      }
      else  // speed < 1
      {
        speed *= 3600*24;
        unit = "n/d";
      }

      std::cout << unsigned(speed) << unit << std::string(10, ' ')
                << std::endl << std::string(40, '-') << std::endl;
    }

    return stats_;
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool evolution::check() const
  {
    return pop_.check() && eva_;
  }

  ///
  /// Default constructor just call the summary::clear method.
  ///
  summary::summary()
  {
    clear();
  }

  ///
  /// Resets summary informations.
  ///
  void summary::clear()
  {
    ttable_probes = 0;
    ttable_hits   = 0;
    mutations     = 0;
    crossovers    = 0;
    gen           = 0;
    testset       = 0;
    last_imp      = 0;

    best.reset();

    az.clear();
  }
}  // namespace vita
