/**
 *
 *  \file lambda_f.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(LAMBDA_F_H)
#define      LAMBDA_F_H

#include "data.h"
#include "individual.h"

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
    { assert(ind.check()); }

    virtual any operator()(const data::example &) const = 0;

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
    dyn_slot_engine(const individual &, data &, size_t);

    size_t slot(const individual &, const data::example &) const;

    typedef std::vector<unsigned> uvect;

    /// The main matrix of the dynamic slot algorithm.
    /// slot_matrix[slot][class] = "number of training examples of class
    /// 'class' mapped to slot 'slot'".
    std::vector<uvect> slot_matrix;

    /// slot_class[i] = "label of the predominant class" for the i-th slot.
    uvect slot_class;

    /// Size of the dataset used to construct \a slot_matrix.
    size_t dataset_size;

    static double normalize_01(double);
  };

  ///
  /// This class transforms vita individuals to lambda functions which can
  /// be used for classification tasks.
  ///
  /// The algorithm used for classification is Slotted Dynamic Class
  /// Boundary Determination (see vita::dyn_slot_evaluator for further details).
  ///
  class dyn_slot_lambda_f : public lambda_f
  {
  public:
    dyn_slot_lambda_f(const individual &, data &, size_t = 10);

    virtual any operator()(const data::example &) const;

  private:
    dyn_slot_engine engine_;

    /// slot_name_[i] = "name of the i-th class".
    std::vector<std::string> slot_name_;
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

    size_t class_label(const individual &, const data::example &, double * = 0,
                       double * = 0) const;

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
  class gaussian_lambda_f : public lambda_f
  {
  public:
    gaussian_lambda_f(const individual &, data &);

    virtual any operator()(const data::example &) const;

  private:
    gaussian_engine engine_;

    /// class_name_[i] = "name of the i-th class of the classification problem".
    std::vector<std::string> class_name_;
  };
}  // namespace vita

#endif  // LAMBDA_F_H
