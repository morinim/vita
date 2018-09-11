/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LAMBDA_F_H)
#define      VITA_LAMBDA_F_H

#include <type_traits>

#include "kernel/src/dataframe.h"
#include "kernel/src/interpreter.h"
#include "kernel/src/model_metric.h"
#include "kernel/team.h"
#include "utility/discretization.h"
#include "utility/matrix.h"

#include "kernel/detail/lambda_f.h"

namespace vita
{
///
/// Transforms individuals into lambda functions.
///
/// \tparam T type of individual
///
/// This class transforms individuals into lambda functions which can be
/// used to calculate the answers for symbolic regression / classification
/// problems.
///
/// \note
/// Depending on the task (regression, classification...) lambda_f and
/// interpreter outputs can be similar or distinct.
/// E.g. for **regression problems** lambda_f and interpreter are identical:
/// they calculate the same number.
/// lambda_f always calculates a meaningful value for the end-user (the
/// class of the example, an approximation...) while interpreter can output
/// a value that is just a building block for lambda_f (e.g. classification
/// tasks with discriminant functions).
/// The typical use chain is: evaluator uses lambda_f, lambda_f uses
/// interpreter.
///
/// \note
/// Another interesting function of lambda_f is that it extends the
/// functionalities of interpreter to teams.
///
template<class T>
class lambda_f
{
public:
  virtual ~lambda_f() = default;

  virtual any operator()(const dataframe::example &) const = 0;

  virtual std::string name(const any &) const = 0;

  virtual double measure(const model_metric<T> &, const dataframe &) const = 0;

  virtual bool debug() const = 0;

  // Serialization
  virtual bool load(std::istream &, const problem &) { return false; }
  virtual bool save(std::ostream &) const { return false; }
};

// ***********************************************************************
// * Symbolic regression                                                 *
// ***********************************************************************

/// Often, working with regression models, we aren't interested in
/// implementation details (i.e. class basic_reg_lambda_f) but only in the
/// general class of the model (e.g. the measurement of some metric in class
/// model_metric).
template<class T> class reg_lambda_f : public lambda_f<T> {};

///
/// \brief Transforms individual to a lambda function for regression
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only.
///           Sometimes we need an autonomous lambda function that stores
///           everything it needs inside (slow but will survive the death of
///           the the program it's constructed on); sometimes we need speed,
///           not persistence
///
template<class T, bool S>
class basic_reg_lambda_f : public reg_lambda_f<T>,
                           protected detail::reg_lambda_f_storage<T, S>
{
public:
  explicit basic_reg_lambda_f(const T &);

  any operator()(const dataframe::example &) const override;

  std::string name(const any &) const override;

  double measure(const model_metric<T> &, const dataframe &) const override;

  bool debug() const override;

  // Serialization
  bool load(std::istream &, const problem &) override;
  bool save(std::ostream &) const override;

private:
  any eval(const dataframe::example &, std::false_type) const;
  any eval(const dataframe::example &, std::true_type) const;
};

// ***********************************************************************
// * Classification                                                      *
// ***********************************************************************

/// Often, working with classification models, we aren't interested in
/// implementation details (i.e. class basic_class_lambda_f) but only in the
/// general class of the model (e.g. the measurement of some metric in
/// class model_metric).
template<class T> class class_lambda_f : public lambda_f<T>
{
public:
  virtual std::pair<class_t, double> tag(const dataframe::example &) const = 0;
};

///
/// \brief The basic interface of a classification lambda class
///
/// \tparam T type of individual
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class:
/// * extends the interface of class lambda_f to handle typical requirements
///   for classification tasks;
/// * factorizes out some code from specific classification schemes;
/// * optionally stores class names.
///
template<class T, bool N>
class basic_class_lambda_f : public class_lambda_f<T>,
                             protected detail::class_names<N>
{
public:
  explicit basic_class_lambda_f(const dataframe &);

  any operator()(const dataframe::example &) const override;

  std::string name(const any &) const final;

  double measure(const model_metric<T> &, const dataframe &) const override;
};

///
/// \brief Lambda class for Slotted Dynamic Class Boundary Determination
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals to lambda functions which can be used
/// for classification tasks.
///
/// See vita::dyn_slot_evaluator for further details.
///
template<class T, bool S, bool N>
class basic_dyn_slot_lambda_f : public basic_class_lambda_f<T, N>
{
public:
  basic_dyn_slot_lambda_f(const T &, dataframe &, unsigned);

  std::pair<class_t, double> tag(const dataframe::example &) const override;

  bool debug() const override;

  double training_accuracy() const;

  // Serialization
  bool load(std::istream &, const problem &) override;
  bool save(std::ostream &) const override;

private:
  // Private support methods
  bool load_(std::istream &, const problem &, std::true_type);
  bool load_(std::istream &, const problem &, std::false_type);

  void fill_matrix(dataframe &, unsigned);
  std::size_t slot(const dataframe::example &) const;

  // Private data members

  /// Mainly used by the slot private method.
  basic_reg_lambda_f<T, S> lambda_;

  /// The main matrix of the dynamic slot algorithm.
  /// slot_matrix[slot][class] = "number of training examples of class
  /// 'class' mapped to slot 'slot'".
  matrix<unsigned> slot_matrix_;

  /// slot_class[i] = "label of the predominant class" for the i-th slot.
  std::vector<std::size_t> slot_class_;

  /// Size of the dataset used to construct `slot_matrix`.
  std::size_t dataset_size_;
};

///
/// \brief Lambda class for the Gaussian Distribution Classification
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals to lambda functions which can be used
/// for classification tasks.
///
/// See vita::gaussian_evaluator for further details.
///
template<class T, bool S, bool N>
class basic_gaussian_lambda_f : public basic_class_lambda_f<T, N>
{
public:
  basic_gaussian_lambda_f(const T &, dataframe &);

  std::pair<class_t, double> tag(const dataframe::example &) const override;

  bool debug() const override;

  // Serialization
  bool load(std::istream &, const problem &) override;
  bool save(std::ostream &) const override;

private:
  // Private support methods
  void fill_vector(dataframe &);
  bool load_(std::istream &, const problem &, std::true_type);
  bool load_(std::istream &, const problem &, std::false_type);

  // Private data members
  basic_reg_lambda_f<T, S> lambda_;

  // gauss_dist[i] = "the gaussian distribution of the i-th class if the
  // classification problem".
  std::vector<distribution<number>> gauss_dist_;
};

///
/// \brief Lambda class for Binary Classification
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals to lambda functions which can be used
/// for single-class classification tasks.
///
template<class T, bool S, bool N>
class basic_binary_lambda_f : public basic_class_lambda_f<T, N>
{
public:
  basic_binary_lambda_f(const T &, dataframe &);

  std::pair<class_t, double> tag(const dataframe::example &) const override;

  bool debug() const override;

  // Serialization
  bool load(std::istream &, const problem &) override;
  bool save(std::ostream &) const override;

private:
  // Private support methods
  bool load_(std::istream &, const problem &, std::true_type);
  bool load_(std::istream &, const problem &, std::false_type);

  // Private data members
  basic_reg_lambda_f<T, S> lambda_;
};

// ***********************************************************************
// * Extension to support teams                                          *
// ***********************************************************************

///
/// For classification problems there exist two major possibilities to
/// combine the outputs of multiple predictors: either the raw output values
/// or the classification decisions can be aggregated (in the latter case
/// the team members act as full pre-classificators themselves). We decided
/// for the latter and combined classification decisions (thanks to the
/// confidence parameter we don't have a reduction in the information
/// content that each individual can contribute to the common team decision).
///
enum class team_composition
{
  mv,  // majority voting
  wta, // winner takes all

  standard = wta
};

///
/// \brief An helper class for extending classification schemes to teams
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only.
///           Sometimes we need an autonomous lambda function that stores
///           everything it needs inside (slow but will survive the death of
///           the the program it's constructed on); sometimes we need speed,
///           not persistence
/// \tparam N stores the name of the classes vs doesn't store the names
/// \tparam L the basic classificator that must be extended
/// \tparam C composition method for team's member responses
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C = team_composition::standard>
class team_class_lambda_f : public basic_class_lambda_f<team<T>, N>
{
public:
  template<class... Args> team_class_lambda_f(const team<T> &, dataframe &,
                                              Args &&...);

  std::pair<class_t, double> tag(const dataframe::example &) const override;

  bool debug() const override;

  // Serialization
  bool load(std::istream &, const problem &) override;
  bool save(std::ostream &) const override;

protected:
  // The components of the team never store the names of the classes. If we
  // need the names, the master class will memorize them.
  std::vector<L<T, S, false>> team_;

  unsigned classes_;
};

///
/// \brief Slotted Dynamic Class Boundary Determination specialization for
///        teams
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
template<class T, bool S, bool N>
class basic_dyn_slot_lambda_f<team<T>, S, N>
  : public team_class_lambda_f<T, S, N, basic_dyn_slot_lambda_f>
{
public:
  using basic_dyn_slot_lambda_f::team_class_lambda_f::team_class_lambda_f;
};

///
/// \brief Gaussian Distribution Classification specialization for teams
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
template<class T, bool S, bool N>
class basic_gaussian_lambda_f<team<T>, S, N>
  : public team_class_lambda_f<T, S, N, basic_gaussian_lambda_f>
{
public:
  using basic_gaussian_lambda_f::team_class_lambda_f::team_class_lambda_f;
};

///
/// \brief Binary Classification specialization for teams
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals to lambda functions which can be used
/// for single-class classification tasks.
///
template<class T, bool S, bool N>
class basic_binary_lambda_f<team<T>, S, N>
  : public team_class_lambda_f<T, S, N, basic_binary_lambda_f>
{
public:
  using basic_binary_lambda_f::team_class_lambda_f::team_class_lambda_f;
};

// ***********************************************************************
// *  Template aliases to simplify the syntax and help the end user      *
// ***********************************************************************
template<class T>
using dyn_slot_lambda_f = basic_dyn_slot_lambda_f<T, true, true>;

template<class T>
using gaussian_lambda_f = basic_gaussian_lambda_f<T, true, true>;

template<class T>
using binary_lambda_f = basic_binary_lambda_f<T, true, true>;

#include "kernel/lambda_f.tcc"
}  // namespace vita

#endif  // Include guard
