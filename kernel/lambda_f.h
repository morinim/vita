/**
 *
 *  \file lambda_f.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(LAMBDA_F_H)
#define      LAMBDA_F_H

#include "kernel/data.h"
#include "kernel/matrix.h"
#include "kernel/src/interpreter.h"

namespace vita
{
  template<class T> class dyn_slot_lambda_f;
  template<class T> class dyn_slot_evaluator;
  template<class T> class gaussian_lambda_f;
  template<class T> class gaussian_evaluator;

  ///
  /// This class transforms individuals to lambda functions which can be used
  /// to calculate the answers for symbolic regression problems.
  ///
  /// \note
  /// For classification problems there are other classes derived from
  /// \c lambda_f.
  ///
  template<class T>
  class lambda_f
  {
  public:
    explicit lambda_f(const T &ind) : ind_(ind)
    { assert(ind.debug()); }

    virtual any operator()(const data::example &) const;

    virtual std::string name(const any &) const { return std::string(); }

  protected:
    T ind_;
  };

  ///
  /// This class encapsulates the engine of the Slotted Dynamic Class
  /// Boundary Determination algorithm (see vita::dyn_slot_evaluator for
  /// further details about the algorithm).
  ///
  /// Methods and properties of dyn_slot_engine can only be accessed from
  /// vita::dyn_slot_lambda_f and dyn_slot_evaluator (they have a HAS-A
  /// 'friendly' relationship with dyn_slot_engine).
  ///
  template<class T>
  class dyn_slot_engine
  {
    friend class dyn_slot_lambda_f<T>;
    friend class dyn_slot_evaluator<T>;

    dyn_slot_engine() {}
    dyn_slot_engine(const T &, data &, unsigned);

    unsigned slot(const T &, const data::example &) const;

    /// The main matrix of the dynamic slot algorithm.
    /// slot_matrix[slot][class] = "number of training examples of class
    /// 'class' mapped to slot 'slot'".
    matrix<unsigned> slot_matrix;

    /// slot_class[i] = "label of the predominant class" for the i-th slot.
    std::vector<unsigned> slot_class;

    /// Size of the dataset used to construct \a slot_matrix.
    unsigned dataset_size;

    static double normalize_01(double);
  };

  ///
  /// This class is used to factorize out some code from the lambda functions
  /// used for classification tasks.
  ///
  template<class T>
  class class_lambda_f : public lambda_f<T>
  {
  public:
    class_lambda_f(const T &, const data &);

    virtual std::string name(const any &a) const final
    {
      return class_name_[any_cast<unsigned>(a)];
    }

  protected:
    /// class_name_[i] = "name of the i-th class of the classification task".
    std::vector<std::string> class_name_;
  };

  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T>
  class dyn_slot_lambda_f : public class_lambda_f<T>
  {
  public:
    dyn_slot_lambda_f(const T &, data &, unsigned);

    virtual any operator()(const data::example &) const override;

  private:
    dyn_slot_engine<T> engine_;
  };

  ///
  /// This class encapsulates the engine of the Gaussian classification
  /// algorithm (see vita::gaussian_evaluator for further details).
  ///
  /// Methods and properties of gaussian_engine can only be accessed from
  /// vita::gaussian_lambda_f and gaussian_evaluator (they have a HAS-A
  /// 'friendly' relationship with gaussian_engine).
  ///
  template<class T>
  class gaussian_engine
  {
    friend class gaussian_lambda_f<T>;
    friend class gaussian_evaluator<T>;

    gaussian_engine() {}
    gaussian_engine(const T &, data &);

    unsigned class_label(const T &, const data::example &,
                         double * = nullptr, double * = nullptr) const;

    /// gauss_dist[i] = "the gaussian distribution of the i-th class if the
    /// classification problem".
    std::vector<distribution<double>> gauss_dist;
  };

  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  template<class T>
  class gaussian_lambda_f : public class_lambda_f<T>
  {
  public:
    gaussian_lambda_f(const T &, data &);

    virtual any operator()(const data::example &) const override;

  private:
    gaussian_engine<T> engine_;
  };

  ///
  /// This class transforms individuals to lambda functions which can be used
  /// for single-class classification tasks.
  ///
  template<class T>
  class binary_lambda_f : public class_lambda_f<T>
  {
  public:
    binary_lambda_f(const T &, data &);

    virtual any operator()(const data::example &) const override;
  };

#include "kernel/lambda_f_inl.h"
}  // namespace vita

#endif  // LAMBDA_F_H
