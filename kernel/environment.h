/**
 *
 *  \file environment.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(ENVIRONMENT_H)
#define      ENVIRONMENT_H

#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/property_tree/ptree.hpp>

#include <cmath>
#include <string>

#include "kernel/vita.h"
#include "kernel/symbol_set.h"

namespace vita
{
  ///
  /// This is a container for Vita's parameters. Contained values are used
  /// during object initialization and program execution.
  ///
  struct environment
  {
    explicit environment(bool = false);

    void log(boost::property_tree::ptree *const,
             const std::string & = "") const;

    void insert(const symbol_ptr &);

    /// The number of genes (maximum length of an evolved program in the
    /// population).
    /// Code length have to be chosen before population is created and cannot
    /// be changed afterwards.
    boost::optional<unsigned> code_length;

    /// An elitist algorithm is one that ALWAYS retains in the population the
    /// best individual found so far. With higher elitism the population will
    /// converge quicker but losing diversity.
    boost::tribool elitism;

    /// Mutation probability. Mutation is one of the principal "search
    /// operators" used to transform programs in the Genetic Programming
    /// algorithm. It causes random changes in individuals.
    boost::optional<double> p_mutation;

    /// Crossover probability.
    /// \see operation_strategy::operator().
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

    /// Switches Dynamic Subset Selection on/off.
    /// \see search::dss()
    boost::tribool dss;

    /// Population's size (number of programs/individuals in the population).
    boost::optional<unsigned> individuals;

    /// Size of the tournament to choose the parents from.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be
    /// equivalent to selecting individuals at random.
    boost::optional<unsigned> tournament_size;

    /// This is used for the trivial geography scheme (Spector, Klein 2005).
    /// The population is viewed as having a 1-dimensional spatial structure -
    /// actually a circle, as we consider the first and last locations to be
    /// adiacent. The production of an individual from location i is permitted
    /// to involve only parents from i's local neightborhood, where the
    /// neightborhood is defined as all individuals within distance
    /// \c mate_zone/2 of i (0 for panmictic).
    boost::optional<unsigned> mate_zone;

    /// Maximun number of generations allowed before terminate a run.
    boost::optional<unsigned> g_since_start;
    /// Stop a run when we cannot see improvements within g_without_improvement
    /// generations.
    boost::optional<unsigned> g_without_improvement;

    /// Should we use Adaptive Representation through Learning?
    boost::tribool arl;

    /// 2^ttable_size is the number of elements of the transposition table.
    unsigned ttable_size;

    /// Where shuld we save statistics / status files?
    std::string stat_dir;

    /// Save a list of active ADF?
    bool stat_arl;

    /// Should we save a dynamic execution status file?
    bool stat_dynamic;

    /// Should we save a summary of the run?
    bool stat_summary;

    symbol_set sset;

    bool check(bool, bool) const;

    static const char arl_filename[];
    static const char dyn_filename[];
    static const char sum_filename[];
  };
}  // namespace vita

#endif  // ENVIRONMENT_H
