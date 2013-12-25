/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LAMBDA_F_INL_H)
#define      LAMBDA_F_INL_H

///
/// \param[in] ind the individual to be lambdified.
///
template<class T>
lambda_f<T>::lambda_f(const T &ind) : ind_(ind)
{
  assert(ind_.debug());
}

///
/// \return An empty string.
///
/// Specialized classes can map values to names by this method.
///
template<class T>
std::string lambda_f<T>::name(const any &) const
{
  return std::string();
}

///
/// \param[in] ind the individual to be lambdified.
///
template<class T>
reg_lambda_f<T>::reg_lambda_f(const T &ind)
  : lambda_f<T>(ind), int_(this->ind_)
{
  assert(int_.debug());
}    

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with \a e.
///
template<class T>
any reg_lambda_f<T>::operator()(const data::example &e) const
{
  return any(int_.run(e.input));
}

///
/// \param[in] ind individual used for classification.
/// \param[in] d the training set.
///
template<class T>
class_lambda_f<T>::class_lambda_f(const T &ind, const data &d)
  : lambda_f<T>(ind), class_name_(d.classes())
{
  assert(ind.debug());

  const auto classes(d.classes());
  assert(d.classes() > 1);

  for (auto i(decltype(classes){0}); i < classes; ++i)
    class_name_[i] = d.class_name(i);
}

///
/// \param[in] ind individual used for classification.
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
/// Sets up the data structures needed by the 'dyn_slot' algorithm.
///
template<class T>
dyn_slot_engine<T>::dyn_slot_engine(const T &ind, data &d, unsigned x_slot)
  : slot_matrix(d.classes() * x_slot, d.classes()),
    slot_class(d.classes() * x_slot), dataset_size(0)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);
  assert(x_slot);

  const auto n_slots(d.classes() * x_slot);
  assert(n_slots == slot_matrix.rows());
  assert(slot_matrix.cols() == d.classes());

  // Here starts the slot-filling task.
  slot_matrix.fill(0);

  // In the first step this method evaluates the program to obtain an output
  // value for each training example. Based on the program output a
  // bi-dimensional matrix is built (slot_matrix_(slot, class)).
  for (const auto &example : d)
  {
    ++dataset_size;

    const auto where(slot(ind, example));  // Insertion slot

    ++slot_matrix(where, example.label());
  }

  const auto unknown(d.classes());

  // In the second step the method dynamically determine to which class each
  // slot belongs by simply taking the class with the largest value at the
  // slot...
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
  {
    const auto cols(slot_matrix.cols());
    auto best_class(decltype(cols){0});

    for (auto j(decltype(cols){1}); j < cols; ++j)
      if (slot_matrix(i, j) >= slot_matrix(i, best_class))
        best_class = j;

    slot_class[i] = slot_matrix(i, best_class) ? best_class : unknown;
  }

  // Unknown slots can be a problem with new examples (not contained in the
  // training set). We arbitrary assign them to the class of a neighbour
  // slot (if available).
  // Another interesting strategy would be to assign unknown slots to the
  // largest class.
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
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
template<class T>
unsigned dyn_slot_engine<T>::slot(const T &ind, const data::example &e) const
{
  assert(ind.debug());

  src_interpreter<T> agent(ind);
  const any res(agent.run(e.input));

  const auto ns(slot_matrix.rows());
  const auto last_slot(ns - 1);

  if (res.empty())
    return last_slot;

  const double val(to<double>(res));
  const auto where(static_cast<decltype(ns)>(normalize_01(val) * ns));

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
template<class T>
double dyn_slot_engine<T>::normalize_01(double x)
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
template<class T>
dyn_slot_lambda_f<T>::dyn_slot_lambda_f(const T &ind, data &d, unsigned x_slot)
  : class_lambda_f<T>(ind, d)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);
  assert(x_slot);

  // Use the training set for lambdification.
  const data::dataset_t backup(d.dataset());
  d.dataset(data::training);
  engine_ = dyn_slot_engine<T>(ind, d, x_slot);
  d.dataset(backup);
}

///
/// \param[in] instance data to be classified.
/// \return the name of the class that includes \a instance.
///
template<class T>
any dyn_slot_lambda_f<T>::operator()(const data::example &instance) const
{
  const auto where(engine_.slot(this->ind_, instance));

  return any(engine_.slot_class[where]);
}

///
/// \param[in] ind individual used for classification.
/// \param[in] d the training set.
///
/// Sets up the data structures needed by the gaussian algorithm.
///
template<class T>
gaussian_engine<T>::gaussian_engine(const T &ind, data &d)
  : gauss_dist(d.classes())
{
  assert(ind.debug());
  assert(d.classes() > 1);

  src_interpreter<T> agent(ind);

  // For a set of training data, we assume that the behaviour of a program
  // classifier is modelled using multiple Gaussian distributions, each of
  // which corresponds to a particular class. The distribution of a class is
  // determined by evaluating the program on the examples of the class in
  // the training set. This is done by taking the mean and standard deviation
  // of the program outputs for those training examples for that class.
  for (const data::example &example : d)
  {
    const any res(agent.run(example.input));

    double val(res.empty() ? 0.0 : to<double>(res));
    const double cut(10000000.0);
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
/// \return the class of \a instance (numerical id).
///
template<class T>
unsigned gaussian_engine<T>::class_label(const T &ind,
                                         const data::example &example,
                                         double *val, double *sum) const
{
  const any res(src_interpreter<T>(ind).run(example.input));
  const double x(res.empty() ? 0.0 : to<double>(res));

  double val_(0.0), val_sum_(0.0);
  unsigned probable_class(0);

  const auto size(gauss_dist.size());
  for (auto i(decltype(size){0}); i < size; ++i)
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
template<class T>
gaussian_lambda_f<T>::gaussian_lambda_f(const T &ind, data &d)
  : class_lambda_f<T>(ind, d)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);

  // Use the training set for lambdification.
  const data::dataset_t backup(d.dataset());
  d.dataset(data::training);
  engine_ = gaussian_engine<T>(ind, d);
  d.dataset(backup);
}

///
/// \param[in] instance data to be classified.
/// \return the name of the class that includes \a instance.
///
template<class T>
any gaussian_lambda_f<T>::operator()(const data::example &instance) const
{
  return any(engine_.class_label(this->ind_, instance));
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T>
binary_lambda_f<T>::binary_lambda_f(const T &ind, data &d)
  : class_lambda_f<T>(ind, d)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() == 2);
}

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with \a e.
///
template<class T>
any binary_lambda_f<T>::operator()(const data::example &e) const
{
  const any res(src_interpreter<T>(this->ind_).run(e.input));
  const double val(res.empty() ? -1.0 : to<double>(res));

  return any(val > 0.0 ? 1u : 0u);
}

#endif  // LAMBDA_F_INL_H
