/**
 *
 *  \file evolution_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(EVOLUTION_INL_H)
#define      EVOLUTION_INL_H

///
/// \return access to the population being evolved.
///
template<class T>
population<T> &evolution<T>::population()
{
  return pop_;
}

///
/// \return constant reference to the population being evolved.
///
template<class T>
const population<T> &evolution<T>::population() const
{
  return pop_;
}

///
/// \param[in] i individual we are looking for.
/// \return how many times we have looked for \a i.
///
template<class T>
unsigned evolution<T>::seen(const T &i) const
{
  return eva_->seen(i);
}

namespace term
{
  ///
  /// \return \c true when the user press the '.' key.
  ///
  inline bool user_stop()
  {
    const bool stop(kbhit() && std::cin.get() == '.');

    if (stop)
      std::cout << k_s_info << " Stopping evolution..." << std::endl;

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
}

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
template<class T>
evolution<T>::evolution(const environment &env, evaluator *eva,
                        std::function<bool (const summary<T> &)> sc,
                        std::function<void (unsigned)> sd)
  : selection(std::make_shared<tournament_selection<T>>(this)),
    operation(std::make_shared<standard_op<T>>(this, &stats_)),
    replacement(std::make_shared<kill_tournament<T>>(this)), pop_(env),
    eva_(eva), external_stop_condition_(sc), shake_data_(sd)
{
  assert(eva);

  assert(debug(true));
}

///
/// \param[in] s an up to date evolution summary.
/// \return \c true when evolution should be interrupted.
///
template<class T>
bool evolution<T>::stop_condition(const summary<T> &s) const
{
  assert(pop_.env().g_since_start);

  // Check the number of generations.
  if (*pop_.env().g_since_start > 0 && s.gen > *pop_.env().g_since_start)
    return true;

  if (term::user_stop())
    return true;

  // When we have an external_stop_condition_ function we use it
  if (external_stop_condition_)
    return external_stop_condition_(s);

  return false;
}

///
/// \param[in] elapsed_milli time, in milliseconds, elapsed from the start
///                          of evolution.
/// \return speed of execution (cycles / s).
///
template<class T>
double evolution<T>::get_speed(double elapsed_milli) const
{
  double speed(0.0);
  if (stats_.gen && elapsed_milli > 0)
    speed = 1000.0 * (pop_.individuals() * stats_.gen) / elapsed_milli;

  return speed;
}

///
/// \return statistical informations about the elements of the population.
///
template<class T>
analyzer evolution<T>::get_stats() const
{
  analyzer az;

  for (const auto &i : pop_)
    az.add(i, fitness(i));

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
template<class T>
void evolution<T>::log(unsigned run_count) const
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

      dynamic << run_count << ' ' << stats_.gen;

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
        pop << run_count << ' ' << stats_.gen << ' '
            << std::fixed << std::scientific
            << std::setprecision(
                 std::numeric_limits<fitness_t::base_t>::digits10 + 2)
            << f.first[0] << ' ' << f.second << std::endl;
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
template<class T>
fitness_t evolution<T>::fitness(const T &ind) const
{
  return (*eva_)(ind);
}

///
/// \param[in] ind individual whose fitness we are interested in.
/// \return the fitness of \a ind.
///
template<class T>
fitness_t evolution<T>::fast_fitness(const T &ind) const
{
  return eva_->fast(ind);
}

///
/// \param[in] k current generation.
/// \param[in] run_count total number of runs planned.
/// \param[in] status if \c true print a run/generation/fitness status line.
///
/// Print evolution informations (if environment::verbosity > 0).
///
template<class T>
void evolution<T>::print_progress(unsigned k, unsigned run_count,
                                 bool status) const
{
  if (env().verbosity >= 1)
  {
    const unsigned perc(100 * k / pop_.individuals());
    if (status)
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
template<class T>
const summary<T> &evolution<T>::run(unsigned run_count)
{
  stats_.clear();
  stats_.best = {pop_[0], fitness(pop_[0])};

  timer measure;

  bool ext_int(false);
  term::set();

  for (stats_.gen = 0; !stop_condition(stats_) && !ext_int;  ++stats_.gen)
  {
#if defined(CLONE_SCALING)
    eva_->clear(evaluator::stats);
#endif

    if (shake_data_ && stats_.gen % 4 == 0)
    {
      shake_data_(stats_.gen);

      // If we 'shake' the data, the statistics picked so far have to be
      // cleared (the best individual and its fitness refer to an old
      // training set).
      assert(stats_.best);
      stats_.best->fitness = fitness(stats_.best->ind);
      print_progress(0, run_count, true);
    }

    stats_.az = get_stats();
    log(run_count);

    for (unsigned k(0); k < pop_.individuals() && !ext_int; ++k)
    {
      if (k % std::max<size_t>(pop_.individuals() / 100, 2))
      {
        print_progress(k, run_count, false);

        ext_int = term::user_stop();
      }

      // --------- SELECTION ---------
      std::vector<size_t> parents(selection->run());

      // --------- CROSSOVER / MUTATION ---------
      std::vector<T> off(operation->run(parents));

      // --------- REPLACEMENT --------
      const fitness_t before(stats_.best->fitness);
      replacement->run(parents, off, &stats_);

      if (stats_.best->fitness != before)
        print_progress(k, run_count, true);
    }

    stats_.speed = get_speed(measure.elapsed());

    pop_.inc_age();
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

  term::reset();
  return stats_;
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return true if object passes the internal consistency check.
///
template<class T>
bool evolution<T>::debug(bool verbose) const
{
  if (!pop_.debug(verbose))
    return false;

  if (!eva_)
  {
    if (verbose)
      std::cerr << k_s_debug << " Empty evaluator pointer." << std::endl;

    return false;
  }

  return true;
}

///
/// Default constructor just call the summary::clear method.
///
template<class T>
summary<T>::summary()
{
  clear();
}

///
/// Resets summary informations.
///
template<class T>
void summary<T>::clear()
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
template<class T>
bool summary<T>::load(std::istream &in, const environment &e)
{
  unsigned known_best(false);
  if (!(in >> known_best))
    return false;

  summary tmp_summary;
  if (known_best)
  {
    T tmp_ind(e, false);
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
template<class T>
bool summary<T>::save(std::ostream &out) const
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
#endif  // EVOLUTION_INL_H

