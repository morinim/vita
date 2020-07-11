/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVOLUTION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_EVOLUTION_TCC)
#define      VITA_EVOLUTION_TCC

namespace term
{
///
/// \return `true` when the user presses the '.' key
///
inline bool user_stop()
{
  const bool stop(keypressed('.'));

  if (stop)
  {
    vitaINFO << "Stopping evolution...";
  }

  return stop;
}

///
/// Resets the term and restores the default signal handlers.
///
inline void reset()
{
  std::signal(SIGABRT, SIG_DFL);
  std::signal(SIGINT, SIG_DFL);
  std::signal(SIGTERM, SIG_DFL);

  term_raw_mode(false);
}

///
/// If the program receives a SIGABRT / SIGINT / SIGTERM, it must handle
/// the signal and reset the terminal to the initial state.
///
inline void signal_handler(int signum)
{
  term::reset();

  std::raise(signum);
}

///
/// Sets the term in raw mode and handles the interrupt signals.
///
inline void set()
{
  // Install our signal handler.
  std::signal(SIGABRT, term::signal_handler);
  std::signal(SIGINT, term::signal_handler);
  std::signal(SIGTERM, term::signal_handler);

  term_raw_mode(true);
}
}  // namespace term

///
/// \param[in] p   the current problem
/// \param[in] eva evaluator used during the evolution
///
template<class T, template<class> class ES>
evolution<T, ES>::evolution(const problem &p, evaluator<T> &eva)
  : pop_(p), eva_(eva), es_(pop_, eva_, &stats_), after_generation_callback_()
{
  Ensures(is_valid());
}

///
/// Sets a callback function called at the end of every generation.
///
/// \param[in] f callback function
/// \return      a reference to `*this` object (fluent interface)
///
template<class T, template<class> class ES>
evolution<T, ES> &evolution<T, ES>::after_generation(
  after_generation_callback_t f)
{
  after_generation_callback_ = f;
  return *this;
}

///
/// \param[in] s an up to date evolution summary
/// \return      `true` when evolution should be interrupted
///
template<class T, template<class> class ES>
bool evolution<T, ES>::stop_condition(const summary<T> &s) const
{
  const auto generations(pop_.get_problem().env.generations);
  Expects(generations);

  // Check the number of generations.
  if (s.gen > generations)
    return true;

  if (term::user_stop())
    return true;

  // Check strategy specific stop conditions.
  return es_.stop_condition();
}

///
/// \return statistical information about the population
///
template<class T, template<class> class ES>
analyzer<T> evolution<T, ES>::get_stats() const
{
  analyzer<T> az;

  for (auto it(pop_.begin()), end(pop_.end()); it != end; ++it)
    az.add(*it, eva_(*it), it.layer());

  return az;
}

///
/// Saves working / statistical informations in a log file.
///
/// \param[in] run_count run number
///
/// Data are written in a CSV-like fashion and are partitioned in blocks
/// separated by two blank lines:
///
///     [BLOCK_1]\n\n
///     [BLOCK_2]\n\n
///     ...
///     [BLOCK_x]
///
/// where each block is a set of line like this:
///
///     data_1 [space] data_2 [space] ... [space] data_n
///
/// We use this format, instead of XML, because statistics are produced
/// incrementally and so it's simple and fast to append new data to a
/// CSV-like file. Note also that it's simple to extract and plot data with
/// GNU Plot.
///
template<class T, template<class> class ES>
void evolution<T, ES>::log_evolution(unsigned run_count) const
{
  static unsigned last_run(0);

  const auto &env(pop_.get_problem().env);

  const auto fullpath([env](const std::filesystem::path &f)
                      {
                        return env.stat.dir / f;
                      });

  if (!env.stat.dynamic_file.empty())
  {
    std::ofstream f_dyn(fullpath(env.stat.dynamic_file), std::ios_base::app);
    if (f_dyn.good())
    {
      if (last_run != run_count)
        f_dyn << "\n\n";

      f_dyn << run_count << ' ' << stats_.gen;

      if (stats_.best.solution.empty())
        f_dyn << " ?";
      else
        f_dyn << ' ' << stats_.best.score.fitness[0];

      f_dyn << ' ' << stats_.az.fit_dist().mean()[0]
            << ' ' << stats_.az.fit_dist().standard_deviation()[0]
            << ' ' << stats_.az.fit_dist().entropy()
            << ' ' << stats_.az.fit_dist().min()[0]
            << ' ' << static_cast<unsigned>(stats_.az.length_dist().mean())
            << ' ' << stats_.az.length_dist().standard_deviation()
            << ' ' << static_cast<unsigned>(stats_.az.length_dist().max())
            << ' ' << stats_.mutations
            << ' ' << stats_.crossovers
            << ' ' << stats_.az.functions(0)
            << ' ' << stats_.az.terminals(0)
            << ' ' << stats_.az.functions(1)
            << ' ' << stats_.az.terminals(1);

      for (unsigned active(0); active <= 1; ++active)
        for (const auto &symb_stat : stats_.az)
          f_dyn << ' ' << symb_stat.first->name()
                << ' ' << symb_stat.second.counter[active];

      f_dyn << " \"";
      if (!stats_.best.solution.empty())
        f_dyn << out::in_line << stats_.best.solution;
      f_dyn << "\"\n";
    }
  }

  if (!env.stat.population_file.empty())
  {
    std::ofstream f_pop(fullpath(env.stat.population_file),
                        std::ios_base::app);
    if (f_pop.good())
    {
      if (last_run != run_count)
        f_pop << "\n\n";

      for (const auto &f : stats_.az.fit_dist().seen())
        // f.first: value, f.second: frequency
        f_pop << run_count << ' ' << stats_.gen << ' '
              << std::fixed << std::scientific
              << std::setprecision(
                   std::numeric_limits<fitness_t::value_type>::digits10 + 2)
              << f.first[0] << ' ' << f.second << '\n';
    }
  }

  es_.log_strategy(last_run, run_count);

  if (last_run != run_count)
    last_run = run_count;
}

///
/// Prints evolution information (if `log::reporting_level >= log::lOUTPUT`).
///
/// \param[in] k             current generation
/// \param[in] run_count     total number of runs planned
/// \param[in] status        if `true` prints a summary line
/// \param[in] from_last_msg time elapsed from the last message
///
template<class T, template<class> class ES>
void evolution<T, ES>::print_progress(unsigned k, unsigned run_count,
                                      bool summary, timer *from_last_msg) const
{
  if (log::lOUTPUT >= log::reporting_level)
  {
    const unsigned perc(100 * k / pop_.individuals());
    if (summary)
      std::cout << "Run " << run_count << '.' << std::setw(6)
                << stats_.gen << " (" << std::setw(3)
                << perc << "%): fitness " << stats_.best.score.fitness
                << '\n';
    else
      std::cout << "Crunching " << run_count << '.' << stats_.gen << " ("
                << std::setw(3) << perc << "%)\r";

    std::cout << std::flush;

    from_last_msg->restart();
  }
}

///
/// The evolutionary core loop.
///
/// \param[in] run_count run number (used for printing and logging)
/// \param[in] shake     the "shake data" function. It's used to alter the
///                      training set so that evolution would take place in a
///                      dynamic environment
/// \return              a partial summary of the search (see notes)
///
/// The genetic programming loop:
///
/// * select the individual(s) to participate (default algorithm: tournament
///   selection) in the genetic operation;
/// * perform genetic operation creating a new offspring individual;
/// * place the offspring into the original population (steady state)
///   replacing a bad individual.
///
/// This whole process repeats until the termination criteria is satisfied.
/// With any luck, it will produce an individual that solves the problem at
/// hand.
///
/// \note
/// The return value is a partial summary: the `measurement` section is only
/// partially filled (fitness) since many metrics are expensive to calculate
/// and not significative for all kind of problems (e.g. f1-score for a
/// symbolic regression problem). The src_search class has a simple scheme to
/// request the computation of additional metrics.
///
template<class T, template<class> class ES>
template<class S>
const summary<T> &evolution<T, ES>::run(unsigned run_count, S shake)
{
  stats_.clear();
  stats_.best.solution = pop_[{0, 0}];
  stats_.best.score.fitness = eva_(stats_.best.solution);

  timer measure;
  timer from_last_msg;

  bool stop(false);
  term::set();

  es_.init();  // customizatin point for strategy-specific initialization

  for (stats_.gen = 0; !stop_condition(stats_) && !stop;  ++stats_.gen)
  {
    if (shake(stats_.gen))
    {
      // The `shake` functions clear cached fitness values (they refer to the
      // previous dataset). So we must recalculate the fitness of the best
      // individual found.
      assert(!stats_.best.solution.empty());
      stats_.best.score.fitness = eva_(stats_.best.solution);

      print_progress(0, run_count, true, &from_last_msg);
    }

    stats_.az = get_stats();
    log_evolution(run_count);

    for (unsigned k(0); k < pop_.individuals() && !stop; ++k)
    {
      if (from_last_msg.elapsed() > std::chrono::seconds(2))
      {
        print_progress(k, run_count, false, &from_last_msg);

        stop = term::user_stop();
      }

      // --------- SELECTION ---------
      auto parents(es_.selection.run());

      // --------- CROSSOVER / MUTATION ---------
      auto off(es_.recombination.run(parents));

      // --------- REPLACEMENT --------
      const auto before(stats_.best.score.fitness);
      es_.replacement.run(parents, off, &stats_);

      if (stats_.best.score.fitness != before)
        print_progress(k, run_count, true, &from_last_msg);
    }

    stats_.elapsed = measure.elapsed();

    es_.after_generation();  // hook for strategy-specific bookkeeping
    if (after_generation_callback_)
      after_generation_callback_(pop_, stats_);
  }

  vitaINFO << "Elapsed time: "
           << std::chrono::duration<double>(stats_.elapsed).count()
           << "s" << std::string(10, ' ');

  term::reset();
  return stats_;
}

///
/// A shortcut to call the `run` method without a shake function.
///
template<class T, template<class> class ES>
const summary<T> &evolution<T, ES>::run(unsigned run_count)
{
  return run(run_count, [](unsigned) { return false; });
}

///
/// \return `true` if object passes the internal consistency check
///
template<class T, template<class> class ES>
bool evolution<T, ES>::is_valid() const
{
  return true;
}
#endif  // include guard
