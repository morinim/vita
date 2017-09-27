/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
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

#include "tinyxml2/tinyxml2.h"

#include "kernel/model_measurements.h"
#include "kernel/log.h"
#include "kernel/symbol_set.h"
#include "kernel/trilean.h"

namespace vita
{

///
/// Initialization type for environment class.
///
/// - `initialization::skip` lets the parameters in an auto-tune state. They'll
///   be auto tuned before the start of the search;
/// - `initialization::standard` sets the parameters to "quite common"
///   (secure?) values.
///
enum class initialization : bool {skip, standard};

namespace out
{
enum print_format_t {list_f,  // default value
                     dump_f, graphviz_f, in_line_f, tree_f,
                     language_f,
                     c_language_f = language_f + symbol::c_format,
                     cpp_language_f = language_f + symbol::cpp_format,
                     mql_language_f = language_f + symbol::mql_format,
                     python_language_f = language_f + symbol::python_format};
}

///
/// Context object aggregating multiple related parameters into one structure.
///
/// A context object (as defined in the Context Object Pattern) aggregates
/// multiple related parameters into one structure, so there needs to be only
/// one common parameter passed amongst functions and classes.
///
/// Included values are used as parameters for object initializations and
/// runtime execution control.
///
class environment
{
public:
  // Constructor and support functions
  explicit environment(initialization = initialization::skip);

  void xml(tinyxml2::XMLDocument *) const;

  // Serialization
  //bool load(std::istream &) {}
  //bool save(std::ostream &) const;

  bool debug(bool) const;

  /// The number of genes (maximum length of an evolved program in the
  /// population).
  ///
  /// Code length have to be chosen before population is created and cannot be
  /// changed afterwards.
  ///
  /// \note A length of 0 means undefined (auto-tune).
  unsigned code_length = 0;

  /// The number of symbols in the patch section (a section of the genome that
  /// contains terminals only).
  ///
  /// \note A length of 0 means undefined (auto-tune).
  unsigned patch_length = 0;

  /// Number of layers for the population.
  ///
  /// \warning
  /// When the evolution strategy is vita::basic_std_es, setting `layers > 1`
  /// is like running `n` evolutions "in parallel" (the sub-populations of each
  /// layer don't interact). A value greater than one is usually choosen for
  /// vita::basic_alps_es or with other strategies that allow migrants.
  ///
  /// \note A value of 0 means undefined (auto-tune).
  unsigned layers = 0;

  /// Number of individuals in a layer of the population.
  ///
  /// \note A value of 0 means undefined (auto-tune).
  unsigned individuals = 0;

  /// Minimum number of individuals in a layer of the population.
  ///
  /// Some evolution strategies dynamically change the number of individuals of
  /// the population. This parameter avoids to drop below a predefined limit
  ///
  /// \note A value of 0 means undefined (auto-tune)
  unsigned min_individuals = 0;

  /// An elitist algorithm is one that ALWAYS retains in the population the
  /// best individual found so far. With higher elitism the population will
  /// converge quicker but losing diversity.
  trilean elitism = trilean::unknown;

  /// Mutation rate probability.
  ///
  /// Mutation is one of the principal "search operators" used to transform
  /// programs in the Genetic Programming algorithm. It causes random
  /// changes in individuals.
  ///
  /// \warning
  /// `p_cross + p_mutation != 1.0`: `p_mutation` is the probability to mutate
  /// a gene; it's not the probability to choose the mutation operator (which
  /// depends depends on the recombination algorithm).
  ///
  /// \note A negative value means undefined (auto-tune).
  ///
  /// \see
  /// - individual::mutation;
  /// - operation_strategy::run.
  double p_mutation = -1.0;

  /// Crossover probability.
  ///
  /// \note A negative value means means undefined (auto-tune).
  ///
  /// \see operation_strategy::run.
  double p_cross = -1.0;

  /// This parameter controls the brood recombination/selection level (`0` to
  /// turn it off).
  ///
  /// In nature it's common for organisms to produce many offspring and then
  /// neglect, abort, resorb, eat some of them or allow them to eat each
  /// other. There are many reasons for this behavior ("Kozlowski & Stearns,
  /// 1989"). The phenomenon is known variously as soft selection, brood
  /// selection, spontaneous abortion. The "bottom line" of this behaviour in
  /// nature is the reduction of parental resource investment in offspring who
  /// are potentially less fit than others.
  ///
  /// \see
  /// "Greedy recombination and genetic search on the space of computer
  /// programs" (Walter Alden Tackett - 1995).
  ///
  /// \note
  /// - `0` means undefined (auto-tune);
  //  - `1` is the standard recombination (perform 1 crossover);
  //  - larger values enable the brood recombination method (more than one
  //    crossover).
  unsigned brood_recombination = 0;

  /// Size of the tournament to choose the parents from.
  ///
  /// Tournament sizes tend to be small relative to the population size. The
  /// ratio of tournament size to population size can be used as a measure of
  /// selective pressure.
  ///
  /// \note
  /// - A tournament size of 1 would be equivalent to selecting individuals
  ///   at random.
  /// - A length of 0 means undefined (auto-tune).
  unsigned tournament_size = 0;

  /// Switches Dynamic Subset Selection on/off.
  ///
  /// \see search::dss()
  trilean dss = trilean::unknown;

  /// This is used for the trivial geography scheme (Spector, Klein 2005).
  /// The population is viewed as having a 1-dimensional spatial structure -
  /// actually a circle, as we consider the first and last locations to be
  /// adiacent. The production of an individual from location `i` is permitted
  /// to involve only parents from `i`'s local neightborhood, where the
  /// neightborhood is defined as all individuals within distance
  /// `mate_zone/2` of `i`.
  ///
  /// \note
  /// - `0` means auto-tune.
  /// - `std::numeric_limits<unsigned>::max()` disables the scheme.
  unsigned mate_zone = 0;

  /// Maximun number of generations allowed before terminate a run.
  ///
  /// \note A value of 0 means undefined (auto-tune).
  unsigned generations = 0;

  /// Used by some evolution strategies to stop a run when there aren't
  /// improvements within g_without_improvement generations.
  ///
  /// \note A value of 0 means disabled (this parameter isn't auto-tuned).
  unsigned g_without_improvement = 0;

  /// How much data should be reserved for the validation set?
  /// `validation_percentage` is the fraction of the original data that goes
  /// into the validation set.
  ///
  /// \note The special value 100 means auto-tune.
  unsigned validation_percentage = 100;

  /// Should we use Adaptive Representation through Learning?
  trilean arl = trilean::unknown;

  /// \f$2^cache_size\f$ is the number of elements of the cache.
  unsigned cache_size = 16;

  struct misc_parameters
  {
    /// Filename used for persistance. An empty name is used to skip
    /// serialization.
    std::string serialization_name = "";
  } misc;

  struct statistics
  {
    /// Where shuld we save statistics / status files?
    std::string dir = "";

    /// Should we save a list of active ADF?
    bool arl             = false;
    std::string arl_name = "arl";

    /// Should we save a dynamic execution status file?
    bool dynamic         =     false;
    std::string dyn_name = "dynamic";

    /// Should we save dynamic statistics about layers status?
    bool layers          =    false;
    std::string lys_name = "layers";

    /// Should we save a dynamic population status file?
    /// \warning It can be very slow!
    bool population      =        false;
    std::string pop_name = "population";

    /// Should we save a summary of the run?
    bool summary         =     false;
    std::string sum_name = "summary";

    std::string tst_name = "test";

    out::print_format_t ind_format = out::list_f;
  } stat;

  /// Used to identify successfully learned (matched, classified, resolved...)
  /// examples.
  ///
  /// By default only fitness is considered.
  model_measurements threshold = model_measurements();

  ///
  /// Parameters for the Age-Layered Population Structure (ALPS) paradigm.
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
    /// an `age_gap` parameter. In addition, this allows individuals in the
    /// first age-layer some time to be optimized before them, or their
    /// offspring, are pushed to the next age layer.
    /// For instance, with 6 age layers, a linear aging-scheme and an age gap
    /// of 20, the maximum ages for the layers are: 20, 40, 60, 80, 100, 120.
    ///
    /// Also, the `age_gap` parameter sets the frequency of how often the first
    /// layer is restarted.
    ///
    /// \note A value of 0 means undefined (auto-tune).
    unsigned age_gap = 20;

    /// We already have a parent (individual) from a layer, which is the
    /// probability that the second parent will be extracted from the same
    /// layer? (with ALPS it could be taken from the previous layer).
    ///
    /// \note A negative value means undefined (auto-tune).
    double p_same_layer = 0.75;
  } alps;

  struct de_parameters
  {
    /// Weighting factor range (aka differential factor range).
    ///
    /// It has been found recently that selecting the weight from the
    /// interval [0.5, 1.0] randomly for each generation or for each
    /// difference vector, a technique called dither, improves convergence
    /// behaviour significantly, especially for noisy objective functions.
    double weight[2] = {0.5, 1.0};
  } de;

  struct team_parameters
  {
    /// 0 is auto-tune;
    /// > 1 means team mode.
    unsigned individuals = 3;
  } team;
};  // class environment

}  // namespace vita

#endif  // include guard
