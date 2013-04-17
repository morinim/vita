/**
 *
 *  \file evolution.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "evolution.h"
#include "environment.h"
#include "random.h"
#include "timer.h"

namespace vita
{
  ///
  /// \param[in] env environment (mostly used for population initialization).
  /// \param[in] eva evaluator used during the evolution.
  /// \param[in] sc function used to identify a stop condition (i.e. it's
  ///               most improbable that evolution will discover better
  ///               solutions).
  /// \param[in] sd the "shake data" function. It is used to alter the training
  ///               set so that evolution would take place in a dynamic
  ///               environment.
  ///
  evolution::evolution(const environment &env, evaluator *eva,
                       std::function<bool (const summary &)> sc,
                       std::function<void (unsigned)> sd)
    : selection(std::make_shared<tournament_selection>(this)),
      operation(std::make_shared<standard_op>(this, &stats_)),
      replacement(std::make_shared<kill_tournament>(this)), pop_(env),
      eva_(eva), stop_condition_(sc), shake_data_(sd)
  {
    assert(eva);

    // When we have a stop_condition function we use it; otherwise the stop
    // criterion is the number of generations.
    if (!stop_condition_)
      stop_condition_ = [this](const summary &s) -> bool
                        {
                          return *pop_.env().g_since_start > 0 &&
                                 s.gen > *pop_.env().g_since_start;
                        };

    assert(debug(true));
  }

  ///
  /// \param[in] elapsed_milli time, in milliseconds, elapsed from the start
  ///                          of evolution.
  /// \return speed of execution (cycles / s).
  ///
  double evolution::get_speed(double elapsed_milli) const
  {
    double speed(0.0);
    if (stats_.gen && elapsed_milli > 0)
      speed = 1000.0 * (pop_.individuals() * stats_.gen) / elapsed_milli;

    return speed;
  }

  ///
  /// \return statistical informations about the elements of the population.
  ///
  analyzer evolution::get_stats() const
  {
    analyzer az;
    for (size_t l(0); l < pop_.layers(); ++l)
      for (size_t i(0); i < pop_.individuals(l); ++i)
      {
        const coord c(l, i);
        az.add(pop_[c], fitness(pop_[c]), l);
      }

    return az;
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
  ///   data_1 [space] data_2 [space] ... [space] data_n
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
      const std::string f_dynamic(pop_.env().stat_dir + "/" +
                                  environment::dyn_filename);
      std::ofstream dynamic(f_dynamic.c_str(), std::ios_base::app);
      if (dynamic.good())
      {
        if (last_run != run_count)
          dynamic << std::endl << std::endl;

        dynamic << run_count
                << ' ' << stats_.gen;

        if (stats_.best)
          dynamic << ' ' << stats_.best->fitness[0];
        else
          dynamic << " ?";

        dynamic << ' ' << stats_.az.fit_dist().mean[0]
                << ' ' << stats_.az.fit_dist().standard_deviation()[0]
                << ' ' << stats_.az.fit_dist().entropy()
                << ' ' << stats_.az.fit_dist().min[0]
                << ' ' << unsigned(stats_.az.length_dist().mean)
                << ' ' << stats_.az.length_dist().standard_deviation()
                << ' ' << unsigned(stats_.az.length_dist().max)
                << ' ' << stats_.mutations
                << ' ' << stats_.crossovers
                << ' ' << stats_.az.functions(0)
                << ' ' << stats_.az.terminals(0)
                << ' ' << stats_.az.functions(1)
                << ' ' << stats_.az.terminals(1);

        for (unsigned active(0); active <= 1; ++active)
          for (const auto &symb_stat : stats_.az)
            dynamic << ' ' << (symb_stat.first)->display()
                    << ' ' << symb_stat.second.counter[active];

        dynamic << " \"";
        if (stats_.best)
          stats_.best->ind.in_line(dynamic);
        dynamic << '"' << std::endl;
      }
    }

    if (pop_.env().stat_population)
    {
      const std::string f_pop(pop_.env().stat_dir + "/" +
                              environment::pop_filename);
      std::ofstream pop(f_pop.c_str(), std::ios_base::app);
      if (pop.good())
      {
        if (last_run != run_count)
          pop << std::endl << std::endl;

        for (const auto &f : stats_.az.fit_dist().freq)
        {
          // f.first: value, f.second: frequency
          pop << run_count << ' ' << stats_.gen << ' ' << f.first << ' '
              << std::setprecision(std::numeric_limits<fitness_t>::digits10 + 2)
              << f.second << std::endl;
        }
      }
    }

    if (pop_.env().stat_ages)
    {
      const std::string f_ages(pop_.env().stat_dir + "/" + "ages");
      std::ofstream ages(f_ages.c_str(), std::ios_base::app);
      if (ages.good())
      {
        if (last_run != run_count)
          ages << std::endl << std::endl;

        for (size_t l(0); l < pop_.layers(); ++l)
        {
          const auto &age_dist(stats_.az.age_dist(l));

          ages << run_count << ' ' << stats_.gen << ' ' << l << ' '
               << pop_.max_age(l) << ' ' << age_dist.mean << ' '
               << age_dist.standard_deviation() << ' ' << age_dist.min
               << ' ' << age_dist.max << std::endl;
        }
      }
    }

    if (last_run != run_count)
      last_run = run_count;
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
  /// \param[in] k current generation.
  /// \param[in] run_count total number of runs planned.
  /// \param[in] resume if \c true print the end-of-generation report.
  ///
  /// Print evolution informations (if environment::verbosity > 0).
  ///
  void evolution::print_progress(unsigned k, unsigned run_count,
                                 bool resume) const
  {
    if (env().verbosity >= 1)
    {
      const unsigned perc(100 * k / pop_.individuals());
      if (resume)
        std::cout << "Run " << run_count << '.' << std::setw(6)
                  << stats_.gen << " (" << std::setw(3)
                  << perc << "%): fitness " << stats_.best->fitness
                  << std::endl;
      else
        std::cout << "Crunching " << run_count << '.' << stats_.gen << " ("
                  << std::setw(3) << perc << "%)\r" << std::flush;
    }
  }

  ///
  /// \param[in] run_count run number (used for print and log).
  ///
  /// The genetic programming loop:
  /// * select the individual(s) to participate (default algorithm: tournament
  ///   selection) in the genetic operation;
  /// * perform genetic operation creating a new offspring individual;
  /// * place the offspring into the original population (steady state)
  ///   replacing a bad individual.
  /// This whole process repeats until the termination criteria is satisfied.
  /// With any luck, it will produce an individual that solves the problem at
  /// hand.
  ///
  const summary &evolution::run(unsigned run_count)
  {
    stats_.clear();
    stats_.best = {pop_[{0, 0}], fitness(pop_[{0, 0}])};

    eva_->clear();

    timer measure;

    for (stats_.gen = 0; !stop_condition_(stats_); ++stats_.gen)
    {
      if (shake_data_)
      {
        shake_data_(stats_.gen);

        // If we 'shake' the data, the statistics picked so far have to be
        // cleared (the best individual and its fitness refer to an old
        // training set).
        stats_.best = {pop_[{0, 0}], fitness(pop_[{0, 0}])};
      }

      stats_.az = get_stats();
      log(run_count);

      for (unsigned k(0); k < pop_.individuals(); ++k)
      {
        if (k % std::max<size_t>(pop_.individuals() / 100, 2))
          print_progress(k, run_count, false);

        // --------- SELECTION ---------
        std::vector<coord> parents(selection->run());

        // --------- CROSSOVER / MUTATION ---------
        std::vector<individual> off(operation->run(parents));

        // --------- REPLACEMENT --------
        const fitness_t before(stats_.best->fitness);
        replacement->run(parents, off, &stats_);

        if (stats_.best->fitness != before)
          print_progress(k, run_count, true);
      }

      stats_.speed = get_speed(measure.elapsed());

      pop_.inc_age();
      if (env().layers > 1 && stats_.gen && stats_.gen % env().age_gap == 0)
        pop_.reset_layer();
    }

    if (env().verbosity >= 2)
    {
      double speed(stats_.speed);
      std::string unit;

      if (speed >= 1.0)
        unit = "cycles/s";
      else if (speed >= 0.1)
      {
        speed *= 3600.0;
        unit = "cycles/h";
      }
      else  // speed < 0.1
      {
        speed *= 3600.0 * 24.0;
        unit = "cycles/day";
      }

      std::cout << k_s_info << ' ' << static_cast<unsigned>(speed) << unit
                << std::string(10, ' ') << std::endl;
    }

    return stats_;
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return true if object passes the internal consistency check.
  ///
  bool evolution::debug(bool verbose) const
  {
    if (!pop_.debug(verbose))
      return false;

    if (!eva_)
    {
      if (verbose)
        std::cerr << k_s_debug << " Empty evaluator pointer." << std::endl;

      return false;
    }

    if (!stop_condition_)
    {
      if (verbose)
        std::cerr << k_s_debug << " Empty stop_condition pointer."
                  << std::endl;

      return false;
    }

    return true;
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
    az.clear();

    best = boost::none;

    speed         = 0.0;
    mutations     = 0;
    crossovers    = 0;
    gen           = 0;
    last_imp      = 0;
  }

  ///
  /// \param[in] in input stream.
  /// \param[in] e an environment (needed to build the best individual).
  /// \return \c true if hash_t loaded correctly.
  ///
  /// \note
  /// If the load operation isn't successful the current hash_t isn't changed.
  ///
  bool summary::load(std::istream &in, const environment &e)
  {
    unsigned known_best(false);
    if (!(in >> known_best))
      return false;

    summary tmp_summary;
    if (known_best)
    {
      individual tmp_ind(e, false);
      if (!tmp_ind.load(in))
        return false;

      fitness_t tmp_fitness;
      if (!tmp_fitness.load(in))
        return false;

      tmp_summary.best = {tmp_ind, tmp_fitness};
    }
    else
      tmp_summary.best = boost::none;

    if (!(in >> tmp_summary.speed >> tmp_summary.mutations
          >> tmp_summary.crossovers >> tmp_summary.gen
          >> tmp_summary.last_imp))
      return false;

    *this = tmp_summary;
    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if summary was saved correctly.
  ///
  bool summary::save(std::ostream &out) const
  {
    // analyzer az doesn't need to be saved: it'll be recalculated at the
    // beginning of evolution.

    if (best)
    {
      out << '1' << std::endl;
      best->ind.save(out);
      best->fitness.save(out);
    }
    else
      out << '0' << std::endl;

    out << speed << ' ' << mutations << ' ' << crossovers << ' ' << gen << ' '
        << last_imp << std::endl;

    return out.good();
  }
}  // namespace vita
