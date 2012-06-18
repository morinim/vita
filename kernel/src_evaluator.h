/**
 *
 *  \file src_evaluator.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_EVALUATOR_H)
#define      SRC_EVALUATOR_H

#include <vector>

#include "kernel/classifier.h"
#include "kernel/data.h"
#include "kernel/evaluator.h"
#include "kernel/primitive/factory.h"

namespace vita
{
  class individual;

  class src_evaluator : public evaluator
  {
  public:
    src_evaluator(data *, std::vector<variable_ptr> *);

    void load_vars(const data::value_type &);

  protected:
    data                      *dat_;
    std::vector<variable_ptr> *var_;
  };

  ///
  /// This class models the evaluators that will drive the evolution towards
  /// the minimum sum of some sort of error.
  /// \see \ref sse_evaluator and \ref sae_evaluator.
  ///
  class sum_of_errors_evaluator : public src_evaluator
  {
  public:
    sum_of_errors_evaluator(data *d, std::vector<variable_ptr> *v)
      : src_evaluator(d, v) {}

    score_t operator()(const individual &);
    score_t fast(const individual &);

  private:
    virtual double error(interpreter &, data::iterator, int *const,
                         unsigned *const) = 0;
  };

  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// absolute errors (\f$\sum_{i=1}^n abs(target_i - actual_i)\f$).
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  /// It is interesting to note that the sum of absolute errors is also
  /// minimized in the least absolute deviations (LAD) approach to regression.
  /// LAD is a robust estimation technique in that it is less sensitive to the
  /// presence of outliers than OLS (Ordinary Least Squares), but is less
  /// efficient than OLS when no outliers are present). It is equivalent to
  /// maximum likelihood estimation under a Laplace distribution model for
  /// \f$epsilon\f$.
  /// \see \ref sse_evaluator.
  ///
  class sae_evaluator : public sum_of_errors_evaluator
  {
  public:
    sae_evaluator(data *d, std::vector<variable_ptr> *v)
      : sum_of_errors_evaluator(d, v) {}

  private:
    double error(interpreter &, data::iterator, int *const, unsigned *const);
  };

  ///
  /// This evaluator will drive the evolution towards the minimum sum of
  /// squared errors (\f$\sum_{i=1}^n (target_i - actual_i)^2\f$).
  /// There is also a penality for illegal values (it is a function of the
  /// number of illegal values).
  /// \see \ref sae_evaluator.
  ///
  class sse_evaluator : public sum_of_errors_evaluator
  {
  public:
    sse_evaluator(data *d, std::vector<variable_ptr> *v)
      : sum_of_errors_evaluator(d, v) {}

  private:
    double error(interpreter &, data::iterator, int *const, unsigned *const);
  };

  ///
  /// This evaluator will drive the evolution towards the maximum sum of
  /// matches (\f$\sum_{i=1}^n target_i == actual_i\f$).
  /// All incorrect answers receive the same fitness penality.
  ///
  class count_evaluator : public sum_of_errors_evaluator
  {
  public:
    count_evaluator(data *d, std::vector<variable_ptr> *v)
      : sum_of_errors_evaluator(d, v) {}

  private:
    double error(interpreter &, data::iterator, int *const, unsigned *const);
  };

  ///
  /// Slotted Dynamic Class Boundary Determination
  /// (http://www.mcs.vuw.ac.nz/comp/Publications/CS-TR-04-2.abs.html).
  ///
  class dyn_slot_evaluator : public src_evaluator
  {
  public:
    dyn_slot_evaluator(data *, std::vector<variable_ptr> *, unsigned = 10);

    score_t operator()(const individual &);

    friend class dyn_slot_classifier;

  private:
    static double normalize_01(double);

    unsigned slot(const individual &, data::const_iterator);

    typedef std::vector<unsigned> uvect;
    void fill_slots(const individual &, std::vector<uvect> *, uvect *,
                    unsigned *);

    // How many slots for each class of the problem?
    unsigned x_slot_;
  };

  class dyn_slot_classifier : public classifier
  {
  public:
    dyn_slot_classifier(const individual &, dyn_slot_evaluator *);

    std::string operator()(const data::value_type &) const;

  private:
    dyn_slot_evaluator             *eva_;
    std::vector<std::string> slot_class_;
  };

  class gaussian_evaluator : public src_evaluator
  {
  public:
    gaussian_evaluator(data *d, std::vector<variable_ptr> *v)
      : src_evaluator(d, v)
    {
      assert(d);
      assert(v);
    }

    score_t operator()(const individual &);

    friend class gaussian_classifier;

  private:
    void gaussian_distribution(const individual &,
                               std::vector<distribution<double>> *);

    unsigned class_label(const individual &, const data::value_type &,
                         const std::vector<distribution<double>> &);
  };

  class gaussian_classifier : public classifier
  {
  public:
    gaussian_classifier(const individual &, gaussian_evaluator *);

    std::string operator()(const data::value_type &) const;

  private:
    gaussian_evaluator                 *eva_;
    std::vector<distribution<double>> gauss_;
  };
}  // namespace vita

#endif  // SRC_EVALUATOR_H
