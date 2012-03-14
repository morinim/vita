/**
 *
 *  \file environment.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
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
    environment();

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
    /// algorithm. Mutation causes random changes in individuals.
    boost::optional<double> p_mutation;

    /// Crossover probability. See \see operation_strategy::operator().
    boost::optional<double> p_cross;

    /// In nature it is common for organisms to produce many offspring and then
    /// neglect, abort, resorb, eat some of them or allow them to eat each
    /// other. There are manu reasons for this behavior ("Kozlowski & Stearns,
    /// 1989"). This phenomenon is known variously as soft selection, brood
    /// selection, spontaneous abortion. The "bottom line" of this behaior in
    /// nature is the reduction of parental resource investment in offspring who
    /// are potentially less fit than others.
    /// See "Greedy recombination and genetic search on the space of computer
    /// programs" (Walter Alden Tackett - 1995) for further details.
    /// This parameter controls the brood recombination/selection level (0 is
    /// not present).
    unsigned brood_recombination;

    /// Switch Dynamic Subset Selection on/off.
    /// \see search::dss()
    bool dss;

    /// Population's size (number of programs/individuals in the population).
    unsigned long individuals;

    /// Size of the tournament to choose the parents.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be
    /// equivalent to selecting individuals at random (within the mate zone).
    unsigned par_tournament;
    /// Size of the tournament to choose replaced individuals.
    /// Tournament sizes tend to be small relative to the population size. The
    /// ratio of tournament size to population size can be used as a measure of
    /// selective pressure. Note that a tournament size of 1 would be
    /// equivalent to selecting individuals at random (within the mate zone).
    unsigned rep_tournament;
    /// Mating zone. 0 for panmictic.
    unsigned mate_zone;

    /// Maximun number of generations allowed before terminate a run (0 means
    /// no limit).
    unsigned g_since_start;
    /// Stop a run when we cannot see improvements within g_without_improvement
    /// generations.
    unsigned g_without_improvement;

    /// Should we use Adaptive Representation through Learning?
    bool arl;

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

    bool check(bool) const;

    static const char arl_filename[];
    static const char dyn_filename[];
    static const char sum_filename[];
  };
}  // namespace vita

#endif  // ENVIRONMENT_H
