/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ENVIRONMENT_H)
#define      VITA_ENVIRONMENT_H

#include <cmath>
#include <filesystem>
#include <string>

#include "tinyxml2/tinyxml2.h"

#include "kernel/model_measurements.h"
#include "kernel/log.h"
#include "kernel/range.h"
#include "kernel/symbol_set.h"
#include "kernel/trilean.h"

#include "utility/facultative.h"

namespace vita
{

namespace out
{
/// Rendering format used to print an individual.
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
  explicit environment();

  void xml(tinyxml2::XMLDocument *) const;
  environment &init();

  // Serialization
  //bool load(std::istream &) {}
  //bool save(std::ostream &) const;

  bool is_valid(bool) const;

  struct mep_parameters
  {
    /// The number of genes (maximum length of an evolved program in the
    /// population).
    ///
    /// Code length have to be chosen before population is created and cannot be
    /// changed afterwards.
    ///
    /// \note A length of `0` means undefined (auto-tune).
    std::size_t code_length = 0;

    /// The number of symbols in the patch section (the tail section of the
    /// genome containing only terminals).
    ///
    /// \note A length of `0` means undefined (auto-tune).
    std::size_t patch_length = 0;
  } mep;

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
  /// a gene; it's not the probability of choosing the mutation operator (which
  /// depends depends on the recombination algorithm).
  ///
  /// \note A negative value means undefined (auto-tune).
  ///
  /// \see individual::mutation
  double p_mutation = -1.0;

  /// Crossover probability.
  ///
  /// \note A negative value means means undefined (auto-tune).
  double p_cross = -1.0;

  /// This parameter controls the brood recombination/selection level (`0` to
  /// turn it off).
  ///
  /// In nature it's common for organisms to produce many offspring and then
  /// neglect, abort, resorb, eat some of them or allow them to eat each
  /// other. There are various reasons for this behavior (e.g. progeny choice
  /// hypothesis by "Kozlowski & Stearns, 1989"). The phenomenon is known
  /// variously as soft selection, brood
  /// selection, spontaneous abortion. The "bottom line" of this behaviour in
  /// nature is the reduction of parental resource investment in offspring who
  /// are potentially less fit than others.
  ///
  /// \see
  /// https://github.com/morinim/vita/wiki/bibliography#4 and
  /// https://github.com/morinim/vita/wiki/bibliography#17
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

  /// Enables Dynamic Subset Selection every `dss` generations.
  ///
  /// This parameter is used by the `dss` validation strategy.
  ///
  /// \see
  /// - search::dss()
  /// - https://github.com/morinim/vita/wiki/bibliography#7
  /// - https://github.com/morinim/vita/wiki/validation
  facultative<unsigned> dss = {};

  /// This is used for the trivial geography scheme.
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
  ///
  /// \see https://github.com/morinim/vita/wiki/bibliography#5
  unsigned mate_zone = 0;

  /// Maximun number of generations allowed before terminate a run.
  ///
  /// \note A value of 0 means undefined (auto-tune).
  unsigned generations = 0;

  /// Used by some evolution strategies to stop a run when there aren't
  /// improvements within `max_stuck_time` generations.
  ///
  /// Default status is uninitialized (auto-tune).
  facultative<unsigned, 0> max_stuck_time = {};

  /// How much data should be reserved for the validation set?
  /// `validation_percentage` is the fraction of the original data that goes
  /// into the validation set.
  ///
  /// This parameter is used by the `holdout_validation` strategy.
  ///
  /// \see https://github.com/morinim/vita/wiki/validation
  facultative<unsigned> validation_percentage = {};

  /// `2^cache_size` is the number of elements of the cache.
  unsigned cache_size = 16;

  struct misc_parameters
  {
    /// Filename used for persistance. An empty name is used to skip
    /// serialization.
    std::string serialization_file = "";
  } misc;

  struct statistics
  {
    /// A base common path for the log files.
    /// \note
    /// A single log file can overwrite this path specifying an absolute path.
    std::filesystem::path dir = {};

    /// Name of the log file used to save real-time information.
    /// \note An empty string disable logging.
    std::filesystem::path dynamic_file = {};

    /// Name of the log file used to save layer-specific information.
    /// \note An empty string disable logging.
    std::filesystem::path layers_file = {};

    /// Name of the log file used to save population-specific information.
    /// \note An empty string disable logging.
    /// \warning
    /// Enabling this log with large populations has a big performance impact.
    std::filesystem::path population_file = {};

    /// Name of the log file used to save a summary report.
    /// \note An empty string disable logging.
    std::filesystem::path summary_file = {};

    /// Name of the file used to save test results.
    /// \name An empty string disable savings.
    std::filesystem::path test_file = {};

    /// Default rendering format used to print an individual.
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
    range_t<double> weight = {0.5, 1.0};
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
