/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LAMBDA_F_H)
#define      LAMBDA_F_H

#include <type_traits>

#include <boost/lexical_cast.hpp>

#include "kernel/data.h"
#include "kernel/matrix.h"
#include "kernel/src/interpreter.h"

namespace vita
{
  ///
  /// \brief Transforms individuals into lambda functions
  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals into lambda functions which can be
  /// used to calculate the answers for symbolic regression /
  /// classification problems.
  ///
  /// \note
  /// Depending on the task (regression, classification...) lambda_f and
  /// interpreter outputs can be similar or distinct.
  /// E.g. for *regression problems* lambda_f and interpreter are identical:
  /// they calculate the same number.
  /// lambda_f always calculates a meaningful value for the end-user (the
  /// class of the example, an approximation...) while interpreter can output
  /// a value that is just a building block for lambda_f (e.g. classification
  /// tasks with discriminant functions).
  /// The typical use chain is: evaluator uses lambda_f, lambda_f uses
  /// interpreter.
  ///
  /// Another interesting function of lambda_f is that it extends the
  /// functionalities of interpreter to teams.
  ///
  template<class T>
  class lambda_f
  {
  public:
    virtual any operator()(const data::example &) const = 0;

    virtual std::string name(const any &) const = 0;

    virtual bool debug() const = 0;
  };

  ///
  /// \brief Transforms individual to a lambda function for regression
  ///
  /// \tparam T type of individual.
  /// \tparam S stores the individual inside vs keep a reference only.
  ///           Sometimes we need an autonomous lambda function that stores
  ///           everything it needs inside (slow but will survive the death of
  ///           the the program it's constructed on); sometimes we need speed,
  ///           not persistence.
  ///
  template<class T, bool S>
  class basic_reg_lambda_f : public lambda_f<T>
  {
  public:
    basic_reg_lambda_f(const T &);

    virtual any operator()(const data::example &) const override final;

    virtual std::string name(const any &) const override final;

    virtual bool debug() const override;

  protected:
    typename std::conditional<S, const T, const T &>::type ind_;
    mutable src_interpreter<T>                             int_;
  };

  ///
  /// \brief Regression lambda function specialization for teams
  ///
  template<class T, bool S>
  class basic_reg_lambda_f<team<T>, S> : public lambda_f<team<T>>
  {
  public:
    basic_reg_lambda_f(const team<T> &);

    virtual any operator()(const data::example &) const override final;

    virtual std::string name(const any &) const override final;

    virtual bool debug() const override;

  protected:
    std::vector<basic_reg_lambda_f<T, S>> team_;
  };

  template<class T> using reg_lambda_f = basic_reg_lambda_f<T, true>;

  ///
  /// \tparam T type of individual.
  ///
  /// This class is used to factorize out some code from the lambda functions
  /// used for classification tasks.
  ///
  template<class T>
  class class_lambda_f : public lambda_f<T>
  {
  public:
    class_lambda_f(const data &);

    virtual class_tag_t tag(const data::example &) const = 0;
    virtual any operator()(const data::example &) const override;

    virtual std::string name(const any &) const override final;

  protected:
    /// class_name_[i] = "name of the i-th class of the classification task".
    std::vector<std::string> class_name_;
  };

  ///
  /// \tparam T type of individual.
  /// \tparam S stores the individual inside vs keep a reference only.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T, bool S>
  class basic_dyn_slot_lambda_f : public class_lambda_f<T>
  {
  public:
    basic_dyn_slot_lambda_f(const T &, data &, unsigned);

    virtual class_tag_t tag(const data::example &) const override;

    virtual bool debug() const override;

    double training_accuracy() const;

  private:  // Private support methods
    static number normalize_01(number);

    void fill_matrix(data &, unsigned);
    unsigned slot(const data::example &) const;

  private:  // Private data members
    /// Mainly used by the slot private method.
    const basic_reg_lambda_f<T, S> lambda_;

    /// The main matrix of the dynamic slot algorithm.
    /// slot_matrix[slot][class] = "number of training examples of class
    /// 'class' mapped to slot 'slot'".
    matrix<unsigned> slot_matrix_;

    /// slot_class[i] = "label of the predominant class" for the i-th slot.
    std::vector<unsigned> slot_class_;

    /// Size of the dataset used to construct \a slot_matrix.
    std::uintmax_t dataset_size_;
  };

  ///
  /// \brief Slotted Dynamic Class Boundary Determination specialization for
  ///        teams
  ///
  template<class T, bool S>
  class basic_dyn_slot_lambda_f<team<T>, S> : public class_lambda_f<team<T>>
  {
  public:
    basic_dyn_slot_lambda_f(const team<T> &, data &, unsigned);

    virtual class_tag_t tag(const data::example &) const override;

    virtual bool debug() const override;

  private:
    std::vector<basic_dyn_slot_lambda_f<T, S>> team_;

    const unsigned classes_;
  };

  template<class T> using dyn_slot_lambda_f = basic_dyn_slot_lambda_f<T, true>;

  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T, bool S>
  class gaussian_lambda_f : public class_lambda_f<T>
  {
  public:
    gaussian_lambda_f(const T &, data &);

    virtual class_tag_t tag(const data::example &) const override;

    class_tag_t tag(const data::example &, number *, number *) const;

    virtual bool debug() const override;

  private:  // Private support methods
    void fill_vector(data &);

  private:  // Private data members
    const basic_reg_lambda_f<T, S> lambda_;

    /// gauss_dist[i] = "the gaussian distribution of the i-th class if the
    /// classification problem".
    std::vector<distribution<number>> gauss_dist_;
  };

  ///
  /// \tparam T type of individual.
  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for single-class classification tasks.
  ///
  template<class T, bool S>
  class binary_lambda_f : public class_lambda_f<T>
  {
  public:
    binary_lambda_f(const T &, data &);

    virtual class_tag_t tag(const data::example &) const override;

    virtual bool debug() const override;

  private:
    const basic_reg_lambda_f<T, S> lambda_;
  };

#include "kernel/lambda_f_inl.h"
}  // namespace vita

#endif  // LAMBDA_F_H
