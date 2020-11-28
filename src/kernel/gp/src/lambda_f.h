/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LAMBDA_F_H)
#define      VITA_LAMBDA_F_H

#include <type_traits>

#include "kernel/exceptions.h"
#include "kernel/gp/src/dataframe.h"
#include "kernel/gp/src/interpreter.h"
#include "kernel/gp/src/model_metric.h"
#include "kernel/gp/team.h"
#include "utility/discretization.h"

#include "kernel/gp/detail/lambda_f.h"

namespace vita
{

///
/// The basic interface of a lambda function.
///
/// \tparam T type of individual
///
/// A lambda function is used to calculate the answers for our problem. It's
/// the model we've computed.
///
/// \note
/// The output of basic_lambda_f and interpreter can be similar or distinct,
/// depending on the task (regression, classification...)
/// E.g. for **regression problems** basic_lambda_f and interpreter are
/// identical: they calculate the same number.
/// basic_lambda_f always calculates a meaningful value for the end-user (the
/// class of an example, an approximation...) while interpreter can output
/// a value that is just a building block for basic_lambda_f (e.g.
/// classification tasks with discriminant functions).
/// The typical use chain is:
/// evaluator --[uses]--> basic_lambda_f --[uses]--> interpreter.
///
class basic_lambda_f
{
public:
  virtual ~basic_lambda_f() = default;

  virtual value_t operator()(const dataframe::example &) const = 0;

  virtual bool is_valid() const = 0;
};

// Forward declarations.
namespace serialize
{

namespace lambda
{

template<class T = i_mep> std::unique_ptr<basic_src_lambda_f> load(
  std::istream &, const symbol_set &);

}  // namespace lambda

bool save(std::ostream &, const basic_src_lambda_f *);
bool save(std::ostream &, const basic_src_lambda_f &);
bool save(std::ostream &, const std::unique_ptr<basic_src_lambda_f> &);

}  // namespace serialize

///
/// Contains a class ID / confidence level pair.
///
struct classification_result
{
  class_t   label;   /// class ID
  double sureness;   /// confidence level
};

///
/// Extends basic_lambda_f interface adding some useful methods for symbolic
/// regression / classification and serialization.
///
/// \note
/// Another interesting function of basic_src_lambda_f is that it extends the
/// functionalities of interpreter to teams.
///
class basic_src_lambda_f : public basic_lambda_f
{
public:
  virtual double measure(const model_metric &, const dataframe &) const = 0;
  virtual std::string name(const value_t &) const = 0;
  virtual classification_result tag(const dataframe::example &) const = 0;

private:
  // *** Serialization ***
  virtual std::string serialize_id() const = 0;
  virtual bool save(std::ostream &) const = 0;

  template<class T> friend std::unique_ptr<basic_src_lambda_f>
  serialize::lambda::load(std::istream &, const symbol_set &);
  friend bool serialize::save(std::ostream &, const basic_src_lambda_f *);
};

// ***********************************************************************
// * Symbolic regression                                                 *
// ***********************************************************************

/// The model_metric class choose the appropriate method considering this type.
/// \see core_class_lambda_f
class core_reg_lambda_f : public basic_src_lambda_f {};

///
/// Lambda function specialized for regression tasks.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only.
///           Sometimes we need an autonomous lambda function that stores
///           everything it needs inside (it will survive the death of the
///           individual it's constructed on). Sometimes we prefer space
///           efficiency (typically inside an evaluator)
///
template<class T, bool S>
class basic_reg_lambda_f : public core_reg_lambda_f,
                           protected detail::reg_lambda_f_storage<T, S>
{
public:
  explicit basic_reg_lambda_f(const T &);
  basic_reg_lambda_f(std::istream &, const symbol_set &);

  value_t operator()(const dataframe::example &) const final;

  std::string name(const value_t &) const final;

  double measure(const model_metric &, const dataframe &) const final;

  bool is_valid() const final;

  // *** Serialization ***
  static const std::string SERIALIZE_ID;
  bool save(std::ostream &) const final;

private:
  // Not useful for regression tasks and moved to private section.
  classification_result tag(const dataframe::example &) const final;

  std::string serialize_id() const final { return SERIALIZE_ID; }

  value_t eval(const dataframe::example &, std::false_type) const;
  value_t eval(const dataframe::example &, std::true_type) const;
};

// ***********************************************************************
// * Classification                                                      *
// ***********************************************************************

///
/// For classification problems there are two major possibilities to combine
/// the outputs of multiple predictors: either the raw output values or the
/// classification decisions can be aggregated (in the latter case the team
/// members act as full pre-classificators themselves). We decided for the
/// latter and combined classification decisions (thanks to the confidence
/// parameter we don't have a reduction in the information content that each
/// individual can contribute to the common team decision).
///
enum class team_composition
{
  mv,  // majority voting
  wta, // winner takes all

  standard = wta
};

/// The model_metric class choose the appropriate method considering this type.
/// \see core_reg_lambda_f
class core_class_lambda_f : public basic_src_lambda_f {};

///
/// The basic interface of a classification lambda class.
///
/// \tparam T type of individual
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class:
/// - extends the interface of class lambda_f to handle typical requirements
///   for classification tasks;
/// - factorizes out some code from specific classification schemes;
/// - optionally stores class names.
///
template<bool N>
class basic_class_lambda_f : public core_class_lambda_f,
                             protected detail::class_names<N>
{
public:
  explicit basic_class_lambda_f(const dataframe &);

  value_t operator()(const dataframe::example &) const final;

  std::string name(const value_t &) const final;

  double measure(const model_metric &, const dataframe &) const final;

protected:
  basic_class_lambda_f() = default;
};

///
/// Lambda class for Slotted Dynamic Class Boundary Determination.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals intto lambda functions which can be used
/// for classification tasks.
///
/// \see vita::dyn_slot_evaluator for further details.
///
template<class T, bool S, bool N>
class basic_dyn_slot_lambda_f : public basic_class_lambda_f<N>
{
public:
  basic_dyn_slot_lambda_f(const T &, dataframe &, unsigned);
  basic_dyn_slot_lambda_f(std::istream &, const symbol_set &);

  classification_result tag(const dataframe::example &) const final;

  bool is_valid() const final;

  double training_accuracy() const;

  // *** Serialization ***
  static const std::string SERIALIZE_ID;
  bool save(std::ostream &) const final;

private:
  // *** Private support methods ***
  void fill_matrix(dataframe &, unsigned);
  std::size_t slot(const dataframe::example &) const;

  std::string serialize_id() const final { return SERIALIZE_ID; }

  // *** Private data members ***
  /// Mainly used by the slot private method.
  basic_reg_lambda_f<T, S> lambda_;

  /// The main matrix of the dynamic slot algorithm.
  /// slot_matrix[slot][class] = "number of training examples of class
  /// 'class' mapped to slot 'slot'".
  matrix<unsigned> slot_matrix_;

  /// slot_class[i] = "label of the predominant class" for the i-th slot.
  std::vector<class_t> slot_class_;

  /// Size of the dataset used to construct `slot_matrix`.
  std::size_t dataset_size_;
};

///
/// Lambda class for the Gaussian Distribution Classification.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals into lambda functions which can be used
/// for classification tasks.
///
/// See vita::gaussian_evaluator for further details.
///
template<class T, bool S, bool N>
class basic_gaussian_lambda_f : public basic_class_lambda_f<N>
{
public:
  basic_gaussian_lambda_f(const T &, dataframe &);
  basic_gaussian_lambda_f(std::istream &, const symbol_set &);

  classification_result tag(const dataframe::example &) const final;

  bool is_valid() const final;

  // *** Serialization ***
  static const std::string SERIALIZE_ID;
  bool save(std::ostream &) const final;

private:
  // *** Private support methods ***
  void fill_vector(dataframe &);
  bool load_(std::istream &, const symbol_set &, std::true_type);
  bool load_(std::istream &, const symbol_set &, std::false_type);

  std::string serialize_id() const final { return SERIALIZE_ID; }

  // *** Private data members ***
  basic_reg_lambda_f<T, S> lambda_;

  // gauss_dist[i] = "the gaussian distribution of the i-th class if the
  // classification problem".
  std::vector<distribution<number>> gauss_dist_;
};

///
/// Lambda class for Binary Classification.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
/// This class transforms individuals into lambda functions which can be used
/// for single-class classification tasks.
///
template<class T, bool S, bool N>
class basic_binary_lambda_f : public basic_class_lambda_f<N>
{
public:
  basic_binary_lambda_f(const T &, dataframe &);
  basic_binary_lambda_f(std::istream &, const symbol_set &);

  classification_result tag(const dataframe::example &) const final;

  bool is_valid() const final;

  // *** Serialization ***
  static const std::string SERIALIZE_ID;
  bool save(std::ostream &) const final;

private:
  std::string serialize_id() const final { return SERIALIZE_ID; }

  basic_reg_lambda_f<T, S> lambda_;
};

// ***********************************************************************
// * Extensions to support teams                                          *
// ***********************************************************************

///
/// An helper class for extending classification schemes to teams.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
/// \tparam L the basic classificator that must be extended
/// \tparam C composition method for team's member responses
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C = team_composition::standard>
class team_class_lambda_f : public basic_class_lambda_f<N>
{
public:
  template<class... Args> team_class_lambda_f(const team<T> &, dataframe &,
                                              Args &&...);
  team_class_lambda_f(std::istream &, const symbol_set &);

  classification_result tag(const dataframe::example &) const final;

  bool is_valid() const final;

  static const std::string SERIALIZE_ID;

private:
  bool save(std::ostream &) const final;
  std::string serialize_id() const final;

  // The components of the team never store the names of the classes. If we
  // need the names, the master class will memorize them.
  std::vector<L<T, S, false>> team_;

  class_t classes_;
};

///
/// Slotted Dynamic Class Boundary Determination specialization for teams.
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
/// Gaussian Distribution Classification specialization for teams.
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
/// Binary Classification specialization for teams.
///
/// \tparam T type of individual
/// \tparam S stores the individual inside vs keep a reference only
/// \tparam N stores the name of the classes vs doesn't store the names
///
template<class T, bool S, bool N>
class basic_binary_lambda_f<team<T>, S, N>
  : public team_class_lambda_f<T, S, N, basic_binary_lambda_f>
{
public:
  using basic_binary_lambda_f::team_class_lambda_f::team_class_lambda_f;
};

// ***********************************************************************
// *  Template aliases to simplify the syntax and help the user          *
// ***********************************************************************
template<class T>
using reg_lambda_f = basic_reg_lambda_f<T, true>;

template<class T>
using dyn_slot_lambda_f = basic_dyn_slot_lambda_f<T, true, true>;

template<class T>
using gaussian_lambda_f = basic_gaussian_lambda_f<T, true, true>;

template<class T>
using binary_lambda_f = basic_binary_lambda_f<T, true, true>;

#include "kernel/gp/src/lambda_f.tcc"
}  // namespace vita

#endif  // include guard
