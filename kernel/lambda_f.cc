/**
 *
 *  \file lambda_f.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "lambda_f.h"
#include "src_interpreter.h"

namespace vita
{
  ///
  /// \param[in] e input example for the lambda function.
  /// \return the output value associated with \a e.
  ///
  any lambda_f::operator()(const data::example &e) const
  {
    return any(src_interpreter(ind_).run(e));
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] d the training set.
  /// \param[in] x_slot number of slots for each class of the training set.
  ///
  /// Sets up the data structures needed by the 'dyn_slot' algorithm.
  ///
  dyn_slot_engine::dyn_slot_engine(const individual &ind, data &d,
                                   size_t x_slot)
    : slot_matrix(d.classes() * x_slot, d.classes()),
      slot_class(d.classes() * x_slot), dataset_size(0)
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);
    assert(x_slot);

    const size_t n_slots(d.classes() * x_slot);
    assert(n_slots == slot_matrix.rows());
    assert(slot_matrix.cols() == d.classes());

    // Here starts the slot-filling task.
    slot_matrix.fill(0);

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional matrix is built (slot_matrix_(slot, class)).
    for (const data::example &example : d)
    {
      ++dataset_size;

      const size_t where(slot(ind, example));

      ++slot_matrix(where, example.label());
    }

    const size_t unknown(d.classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (size_t i(0); i < n_slots; ++i)
    {
      size_t best_class(0);

      for (size_t j(1); j < slot_matrix.cols(); ++j)
        if (slot_matrix(i, j) >= slot_matrix(i, best_class))
          best_class = j;

      slot_class[i] = slot_matrix(i, best_class) ? best_class : unknown;
    }

    // Unknown slots can be a problem with new examples (not contained in the
    // training set). We arbitrary assign to them the class of a neighbour
    // slot (if available).
    // Another interesting strategy would be to assign unknown slots to the
    // largest class.
    for (size_t i(0); i < n_slots; ++i)
      if (slot_class[i] == unknown)
      {
        if (i && slot_class[i - 1] != unknown)
          slot_class[i] = slot_class[i - 1];
        else if (i + 1 < n_slots && slot_class[i + 1] != unknown)
          slot_class[i] = slot_class[i + 1];
        else
          slot_class[i] = 0;
      }
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] e input data for \a ind.
  /// \return the slot the example \a e falls into.
  ///
  size_t dyn_slot_engine::slot(const individual &ind,
                               const data::example &e) const
  {
    assert(ind.check());

    src_interpreter agent(ind);
    const any res(agent.run(e));

    const size_t ns(slot_matrix.rows());
    const size_t last_slot(ns - 1);

    if (res.empty())
      return last_slot;

    const double val(interpreter::to_double(res));
    const size_t where(static_cast<size_t>(normalize_01(val) * ns));

    return (where >= ns) ? last_slot : where;
  }

  ///
  /// \param[in] x the numeric value (a real number in the [-inf;+inf] range)
  ///              that should be mapped in the [0,1] interval.
  /// \return a double in the [0,1] range.
  ///
  /// This is a sigmoid function (it is a bounded real function, "S" shaped,
  /// with positive derivative everywhere).
  ///
  /// \see
  /// <http://en.wikipedia.org/wiki/Sigmoid_function>
  ///
  inline
  double dyn_slot_engine::normalize_01(double x)
  {
    // return (1.0 + x / (1 + std::fabs(x))) / 2.0;  // Algebraic function.

    return 0.5 + std::atan(x) / 3.1415926535;        // Arctangent.

    // return 0.5 + std::tanh(x);                    // Hyperbolic tangent.

    // return 1.0 / (1.0 + std::exp(-x));            // Logistic function.
  }

  ///
  /// \param[in] ind individual "to be transformed" into a lambda function.
  /// \param[in] d the training set.
  /// \param[in] x_slot number of slots for each class of the training set.
  ///
  dyn_slot_lambda_f::dyn_slot_lambda_f(const individual &ind, data &d,
                                       size_t x_slot)
    : lambda_f(ind), class_name_(d.classes() * x_slot)
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);
    assert(x_slot);

    // Use the training set for lambdification.
    const data::dataset_t backup(d.dataset());
    d.dataset(data::training);
    engine_ = dyn_slot_engine(ind, d, x_slot);
    d.dataset(backup);

    for (size_t i(0); i < d.classes(); ++i)
      class_name_[i] = d.class_name(i);
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the name of the class that includes \a instance.
  ///
  any dyn_slot_lambda_f::operator()(const data::example &instance) const
  {
    const size_t where(engine_.slot(ind_, instance));

    return any(class_name_[engine_.slot_class[where]]);
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] d the training set.
  ///
  /// Sets up the data structures needed by the gaussian algorithm.
  ///
  gaussian_engine::gaussian_engine(const individual &ind, data &d)
    : gauss_dist(d.classes())
  {
    assert(ind.check());
    assert(d.classes() > 1);

    src_interpreter agent(ind);

    // For a set of training data, we assume that the behaviour of a program
    // classifier is modelled using multiple Gaussian distributions, each of
    // which corresponds to a particular class. The distribution of a class is
    // determined by evaluating the program on the examples of the class in
    // the training set. This is done by taking the mean and standard deviation
    // of the program outputs for those training examples for that class.
    for (const data::example &example : d)
    {
      const any res(agent.run(example));

      double val(res.empty() ? 0.0 : interpreter::to_double(res));
      const double cut(10000000);
      if (val > cut)
        val = cut;
      else if (val < -cut)
        val = -cut;

      gauss_dist[example.label()].add(val);
    }
  }

  ///
  /// \param[in] ind program used for classification.
  /// \param[in] example input value whose class we are interested in.
  /// \param[out] val confidence level: how sure you can be that \a example
  ///             is properly classified. The value is in [0;1] range.
  /// \param[out] sum the sum of all the confidence levels for \a example
  ///                 (confidence of \a example in class 1 + confidence of
  ///                 \a example in class 2 + ... + confidence \a example in
  ///                 class n).
  /// \return the class of \a instance.
  ///
  size_t gaussian_engine::class_label(const individual &ind,
                                      const data::example &example,
                                      double *val, double *sum) const
  {
    const any res(src_interpreter(ind).run(example));
    const double x(res.empty() ? 0.0 : interpreter::to_double(res));

    double val_(0.0), val_sum_(0.0);
    size_t probable_class(0);

    for (size_t i(0); i < gauss_dist.size(); ++i)
    {
      const double distance(std::fabs(x - gauss_dist[i].mean));
      const double variance(gauss_dist[i].variance);

      double p(0.0);
      if (variance == 0.0)     // These are borderline cases
        if (distance == 0.0)   // These are borderline cases
          p = 1.0;
        else
          p = 0.0;
      else                     // This is the standard case
        p = std::exp(-0.5 * distance * distance / variance);

      if (p > val_)
      {
        val_ = p;
        probable_class = i;
      }

      val_sum_ += p;
    }

    if (val)
      *val = val_;
    if (sum)
      *sum = val_sum_;

    return probable_class;
  }

  ///
  /// \param[in] ind individual "to be transformed" into a lambda function.
  /// \param[in] d the training set.
  ///
  gaussian_lambda_f::gaussian_lambda_f(const individual &ind, data &d)
    : lambda_f(ind), class_name_(d.classes())
  {
    assert(ind.check());
    assert(d.check());
    assert(d.classes() > 1);

    // Use the training set for lambdification.
    const data::dataset_t backup(d.dataset());
    d.dataset(data::training);
    engine_ = gaussian_engine(ind, d);
    d.dataset(backup);

    for (size_t i(0); i < d.classes(); ++i)
      class_name_[i] = d.class_name(i);
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the name of the class that includes \a instance.
  ///
  any gaussian_lambda_f::operator()(const data::example &instance) const
  {
    return any(class_name_[engine_.class_label(ind_, instance)]);
  }
}  // namespace vita
