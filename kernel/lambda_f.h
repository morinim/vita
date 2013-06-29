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
#include "kernel/individual.h"
#include "kernel/matrix.h"

namespace vita
{
  ///
  /// This class transforms vita individuals to lambda functions which can
  /// be used to calculate the answers for symbolic regression problems.
  ///
  /// For classification problems there are other classes derived from
  /// \c lambda_f.
  ///
  class lambda_f
  {
  public:
    explicit lambda_f(const individual &ind) : ind_(ind)
    { assert(ind.debug()); }

    virtual any operator()(const data::example &) const;

    virtual std::string name(const any &) const { return std::string(); }

  protected:
    individual ind_;
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
  class dyn_slot_engine
  {
    friend class dyn_slot_lambda_f;
    friend class dyn_slot_evaluator;

    dyn_slot_engine() {}
    dyn_slot_engine(const individual &, data &, unsigned);

    unsigned slot(const individual &, const data::example &) const;

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
  class class_lambda_f : public lambda_f
  {
  public :
    class_lambda_f(const individual &, const data &);

    virtual std::string name(const any &a) const final
    {
      return class_name_[any_cast<unsigned>(a)];
    }

  protected:
    /// class_name_[i] = "name of the i-th class of the classification task".
    std::vector<std::string> class_name_;
  };

  ///
  /// This class transforms vita individuals to lambda functions which can
  /// be used for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  class dyn_slot_lambda_f : public class_lambda_f
  {
  public:
    dyn_slot_lambda_f(const individual &, data &, unsigned);

    virtual any operator()(const data::example &) const override;

  private:
    dyn_slot_engine engine_;
  };

  ///
  /// This class encapsulates the engine of the Gaussian classification
  /// algorithm (see vita::gaussian_evaluator for further details).
  ///
  /// Methods and properties of gaussian_engine can only be accessed from
  /// vita::gaussian_lambda_f and gaussian_evaluator (they have a HAS-A
  /// 'friendly' relationship with gaussian_engine).
  ///
  class gaussian_engine
  {
    friend class gaussian_lambda_f;
    friend class gaussian_evaluator;

    gaussian_engine() {}
    gaussian_engine(const individual &, data &);

    unsigned class_label(const individual &, const data::example &,
                         double * = 0, double * = 0) const;

    /// gauss_dist[i] = "the gaussian distribution of the i-th class if the
    /// classification problem".
    std::vector<distribution<double>> gauss_dist;
  };

  ///
  /// This class transforms vita individuals to lambda functions which can
  /// be used for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  class gaussian_lambda_f : public class_lambda_f
  {
  public:
    gaussian_lambda_f(const individual &, data &);

    virtual any operator()(const data::example &) const override;

  private:
    gaussian_engine engine_;
  };

  ///
  /// This class transforms vita individuals to lambda functions which can
  /// be used for sinlge-class classification tasks.
  ///
  class binary_lambda_f : public class_lambda_f
  {
  public:
    binary_lambda_f(const individual &, data &);

    virtual any operator()(const data::example &) const override;
  };
}  // namespace vita

#endif  // LAMBDA_F_H
