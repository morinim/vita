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

#if !defined(VITA_ENVIRONMENT_H)
#define      VITA_ENVIRONMENT_H

#include <cmath>
#include <string>

#include <boost/logic/tribool.hpp>
#include <boost/property_tree/ptree.hpp>

#include "kernel/fitness.h"

namespace vita
{
  ///
  /// This is a container for Vita's parameters. Included values are used
  /// as parameters for object initializations and runtime execution control.
  ///
  /// \note
  /// Because using \c boost::optional<bool> can lead to subtle errors due to
  /// the implicit \c bool conversion, we prefer to use \c boost::tribool.
  ///
  class environment
  {
  public:  // Constructor and support functions.
    explicit environment(bool = false);

    void log(boost::property_tree::ptree *const,
             const std::string & = "") const;

    bool debug(bool, bool) const;

  public:  // Data members.
    /// Verbosity level:
    /// * 0 only fatal errors;
    /// * 1 search results and errors;
    /// * 2 search results, informations, warnings and errors;
    /// * >= 3 everything.
    unsigned verbosity = 2;

    /// The number of genes (maximum length of an evolved program in the
    /// population).
    /// Code length have to be chosen before population is created and cannot
    /// be changed afterwards.
    /// \note
    /// A length of 0 means undefined (auto-tune).
    unsigned code_length = 0;

    /// The number of symbols in the patch section (a section of the genome
    /// that contains terminals only).
    /// \note
    /// A length of 0 means undefined (auto-tune).
    unsigned patch_length = 0;

    /// Number of layers for the population.
    /// \warning
    /// When the evolution strategy is vita::basic_std_es, using a \a n-layer
    /// population is like running \a n evolutions "in parallel" (the
    /// sub-populations of each layer don't interact).
    /// A value greater than one is usually choosen for vita::basic_alps_es or
    /// with other strategies that allow migrants.
    /// \note
    /// A value of 0 means undefined (auto-tune).
    unsigned layers = 0;

    /// Number of individuals in a layer of the population.
    ///
    /// \note
    /// A value of 0 means undefined (auto-tune).
    unsigned individuals = 0;

    /// An elitist algorithm is one that ALWAYS retains in the population the
    /// best individual found so far. With higher elitism the population will
    /// converge quicker but losing diversity.
    boost::tribool elitism = boost::indeterminate;

    /// \brief Mutation probability.
    ///
    /// Mutation is one of the principal "search operators" used to transform
    /// programs in the Genetic Programming algorithm. It causes random
    /// changes in individuals.
    ///
    /// \warning
    /// > p_cross + p_mutation != 1.0
    /// \a p_mutation is the probability to mutate a gene; it is not the
    /// probability to choose the the mutation operator (the latter is
    /// 1.0 - p_cross).
    ///
    /// \note
    /// A negative value means means undefined (auto-tune).
    /// \see
    /// * individual::mutation;
    /// * operation_strategy::run.
    double p_mutation;

    /// \brief Crossover probability.
    ///
    /// \see operation_strategy::run.
    boost::optional<double> p_cross;

    /// This parameter controls the brood recombination/selection level (0 to
    /// turn it off).
    ///
    /// In nature it is common for organisms to produce many offspring and then
    /// neglect, abort, resorb, eat some of them or allow them to eat each
    /// other. There are many reasons for this behavior ("Kozlowski & Stearns,
    /// 1989"). This phenomenon is known variously as soft selection, brood
    /// selection, spontaneous abortion. The "bottom line" of this behaviour in
    /// nature is the reduction of parental resource investment in offspring who
    /// are potentially less fit than others.
    /// \see
    /// "Greedy recombination and genetic search on the space of computer
    /// programs" (Walter Alden Tackett - 1995).
    boost::optional<unsigned> brood_recombination;

    /// Size of the tournament to choose the parents from.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be
    /// equivalent to selecting individuals at random.
    /// \note
    /// A length of 0 means undefined (auto-tune).
    unsigned tournament_size = 0;

    /// Switches Dynamic Subset Selection on/off.
    /// \see search::dss()
    boost::tribool dss = boost::indeterminate;

    /// This is used for the trivial geography scheme (Spector, Klein 2005).
    /// The population is viewed as having a 1-dimensional spatial structure -
    /// actually a circle, as we consider the first and last locations to be
    /// adiacent. The production of an individual from location i is permitted
    /// to involve only parents from i's local neightborhood, where the
    /// neightborhood is defined as all individuals within distance
    /// \c mate_zone/2 of i (0 for panmictic).
    boost::optional<unsigned> mate_zone;

    /// Maximun number of generations allowed before terminate a run.
    /// \note
    /// A value of 0 means undefined (auto-tune).
    unsigned generations = 0;

    /// Stop a run when we cannot see improvements within g_without_improvement
    /// generations.
    boost::optional<unsigned> g_without_improvement;

    /// How much data should be reserved for the validation set?
    /// validation_ratio is the fraction of the original data that goes into
    /// the validation set.
    boost::optional<double> validation_ratio;

    /// Should we use Adaptive Representation through Learning?
    boost::tribool arl = boost::indeterminate;

    /// \f$2^ttable_size\f$ is the number of elements of the transposition
    /// table.
    unsigned ttable_size = 16;

    /// Where shuld we save statistics / status files?
    std::string stat_dir = "";

    /// Should we save a list of active ADF?
    bool stat_arl = false;

    /// Should we save a dynamic execution status file?
    bool stat_dynamic = false;

    /// Should we save dynamic statistics about layers status?
    bool stat_layers = false;

    /// Should we save a dynamic population status file?
    /// \warning It can be quite slow!
    bool stat_population = false;

    /// Should we save a summary of the run?
    bool stat_summary = false;

    /// \a f_threashold is used to identify successfully learned (matched,
    /// classified, resolved...) examples by fitness comparison.
    fitness_t f_threashold;

    /// \a a_threashold is used to identify successfully learned (matched,
    /// classified, resolved...) examples by accuracy comparison.
    /// \note
    /// a negative value means not used (only \a f_threashold is used).
    double a_threashold = -1.0;

    ///
    /// \brief Parameters for the Age-Layered Population Structure (ALPS)
    ///        paradigm.
    ///
    /// ALPS is a meta heuristic for overcoming premature convergence by
    /// running multiple instances of a search algorithm in parallel, with each
    /// instance in its own age layer and having its own population.
    ///
    struct alps_parameters
    {
      /// The maximum ages for age layers is monotonically increasing and
      /// different methods can be used for setting these values. Since there
      /// is generally little need to segregate individuals which are within a
      /// few "generations" of each other, these values are then multiplied by
      /// an \a age_gap parameter. In addition, this allows individuals in the
      /// first age-layer some time to be optimized before them, or their
      /// offspring, are pushed to the next age layer.
      /// For instance, with 6 age layers, a linear aging-scheme and an age gap
      /// of 20, the maximum ages for the layers are: 20, 40, 60, 80, 100, 120.
      ///
      /// Also, the \a age_gap parameter sets the frequency of how often the
      /// first layer is restarted.
      ///
      /// \note
      /// A value of 0 means undefined (auto-tune).
      unsigned age_gap = 20;

      /// We already have a parent (individual) from a layer, which is the
      /// probability that the second parent will be extracted from the same
      /// layer? (with ALPS it could be taken from the previous layer).
      /// \note
      /// A probability of -1.0 means undefined (auto-tune).
      double p_same_layer = 0.75;
    } alps;

    struct team_parameters
    {
      /// 0 is auto-tune;
      /// > 1 means team mode.
      unsigned individuals = 3;
    } team;

    static const char arl_filename[];
    static const char dyn_filename[];
    static const char lys_filename[];
    static const char pop_filename[];
    static const char sum_filename[];
    static const char tst_filename[];
  };
}  // namespace vita

#endif  // Include guard.
