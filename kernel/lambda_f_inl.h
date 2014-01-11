/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(LAMBDA_F_INL_H)
#define      LAMBDA_F_INL_H

///
/// \param[in] prg the program (individual / team) to be lambdified.
///
template<class T>
lambda_f<T>::lambda_f(const T &prg) : prg_(prg)
{
  assert(prg_.debug());
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
/// \param[in] ind individual to be lambdified.
///
template<class T>
reg_lambda_f<T>::reg_lambda_f(const T &ind)
  : lambda_f<T>(ind), int_(this->prg_)
{
  assert(int_.debug());
}

///
/// \param[in] t team to be lambdified.
///
template<class T>
reg_lambda_f<team<T>>::reg_lambda_f(const team<T> &t) : lambda_f<team<T>>(t)
{
  const auto size(t.size());

  int_.reserve(size);
  for (const auto &i : t)
    int_.emplace_back(i);

  assert(int_.debug());
}

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with \a e.
///
template<class T>
any reg_lambda_f<T>::operator()(const data::example &e) const
{
  return int_.run(e.input);
}

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with \a e.
///
template<class T>
any reg_lambda_f<team<T>>::operator()(const data::example &e) const
{
  number avg(0), count(0);

  // Calculate the running average.
  for (auto &i : int_)
  {
    const auto res(i.run(e.input));

    if (!res.empty())
      avg += (to<number>(res) - avg) / ++count;
  }

  return count > 0 ? any(avg) : any();
}

///
/// \param[in] prg program (individual/team) used for classification.
/// \param[in] d the training set.
///
template<class T>
class_lambda_f<T>::class_lambda_f(const T &prg, const data &d)
  : lambda_f<T>(prg), class_name_(d.classes())
{
  assert(prg.debug());

  const auto classes(d.classes());
  assert(d.classes() > 1);

  for (auto i(decltype(classes){0}); i < classes; ++i)
    class_name_[i] = d.class_name(i);
}

///
/// \param[in] instance data to be classified.
/// \return the label of the class that includes \a instance (wrapped in class
///         any).
///
template<class T>
any class_lambda_f<T>::operator()(const data::example &e) const
{
  return any(tag(e));
}

///
/// \param[in] a id of a class.
/// \return the name f class \a a.
///
template<class T>
std::string class_lambda_f<T>::name(const any &a) const
{
  return class_name_[any_cast<class_tag_t>(a)];
}

///
/// \param[in] x the numeric value (a real number in the [-inf;+inf] range)
///              that should be mapped in the [0,1] interval.
/// \return a number in the [0,1] range.
///
/// This is a sigmoid function (it is a bounded real function, "S" shaped,
/// with positive derivative everywhere).
///
/// \see
/// <http://en.wikipedia.org/wiki/Sigmoid_function>
///
template<class T>
number dyn_slot_lambda_f<T>::normalize_01(number x)
{
  // return (1.0 + x / (1 + std::fabs(x))) / 2.0;  // Algebraic function

  return 0.5 + std::atan(x) / 3.1415926535;        // Arctangent

  // return 0.5 + std::tanh(x);                    // Hyperbolic tangent

  // return 1.0 / (1.0 + std::exp(-x));            // Logistic function
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
template<class T>
dyn_slot_lambda_f<T>::dyn_slot_lambda_f(const T &ind, data &d, unsigned x_slot)
  : class_lambda_f<T>(ind, d), slot_matrix_(d.classes() * x_slot, d.classes()),
    slot_class_(d.classes() * x_slot), dataset_size_(0)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);
  assert(x_slot);

  // Use the training set for lambdification.
  const auto backup(d.dataset());
  d.dataset(data::training);
  fill_matrix(d, x_slot);
  d.dataset(backup);
}

///
/// \param[in] t team "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
template<class T>
dyn_slot_lambda_f<team<T>>::dyn_slot_lambda_f(const team<T> &t, data &d,
                                              unsigned x_slot)
  : class_lambda_f<team<T>>(t, d)
{
  team_.reserve(t.size());
  for (const auto &ind : t)
    team_.emplace_back(ind, d, x_slot);
}

///
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
/// Sets up the data structures needed by the 'dynamic slot' algorithm.
///
template<class T>
void dyn_slot_lambda_f<T>::fill_matrix(data &d, unsigned x_slot)
{
  assert(d.debug());
  assert(d.classes() > 1);
  assert(x_slot);

  const auto n_slots(d.classes() * x_slot);
  assert(n_slots == slot_matrix_.rows());
  assert(slot_matrix_.cols() == d.classes());

  // Here starts the slot-filling task.
  slot_matrix_.fill(0);

  // In the first step this method evaluates the program to obtain an output
  // value for each training example. Based on the program output a
  // bi-dimensional matrix is built (slot_matrix_(slot, class)).
  for (const auto &example : d)
  {
    ++dataset_size_;

    ++slot_matrix_(slot(example), example.tag());
  }

  const auto unknown(d.classes());

  // In the second step the method dynamically determine which class each
  // slot belongs to by simply taking the class with the largest value at the
  // slot...
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
  {
    const auto cols(slot_matrix_.cols());
    auto best_class(decltype(cols){0});  // Initially assuming class 0 as best

    // ...then looking for a better class among the remaining ones.
    for (auto j(decltype(cols){1}); j < cols; ++j)
      if (slot_matrix_(i, j) >= slot_matrix_(i, best_class))
        best_class = j;

    slot_class_[i] = slot_matrix_(i, best_class) ? best_class : unknown;
  }

  // Unknown slots can be a problem with new examples (not contained in the
  // training set). We arbitrary assign them to the class of a neighbour
  // slot (if available).
  // Another interesting strategy would be to assign unknown slots to the
  // largest class.
  for (auto i(decltype(n_slots){0}); i < n_slots; ++i)
    if (slot_class_[i] == unknown)
    {
      if (i && slot_class_[i - 1] != unknown)
        slot_class_[i] = slot_class_[i - 1];
      else if (i + 1 < n_slots && slot_class_[i + 1] != unknown)
        slot_class_[i] = slot_class_[i + 1];
      else
        slot_class_[i] = 0;
    }
}

///
/// \param[in] e input data for \a ind.
/// \return the slot example \a e falls into.
///
template<class T>
unsigned dyn_slot_lambda_f<T>::slot(const data::example &e) const
{
  src_interpreter<T> agent(this->prg_);
  const any res(agent.run(e.input));

  const auto ns(slot_matrix_.rows());
  const auto last_slot(ns - 1);
  if (res.empty())
    return last_slot;

  const auto val(to<number>(res));
  const auto where(static_cast<decltype(ns)>(normalize_01(val) * ns));

  return (where >= ns) ? last_slot : where;
}

///
/// \return the accuracy of the lambda function on the training set.
///
template<class T>
double dyn_slot_lambda_f<T>::training_accuracy() const
{
  double ok(0.0);

  const auto slots(slot_matrix_.rows());

  for (auto i(decltype(slots){0}); i < slots; ++i)
    ok += slot_matrix_(i, slot_class_[i]);

  assert(dataset_size_ >= ok);

  return static_cast<double>(ok) / dataset_size_;
}

///
/// \param[in] instance data to be classified.
/// \return the label of the class that includes \a instance.
///
template<class T>
class_tag_t dyn_slot_lambda_f<T>::tag(const data::example &instance) const
{
  return slot_class_[slot(instance)];
}

///
/// \param[in] instance data to be classified.
/// \return the label of the class that includes \a instance.
///
/// Specialized method for teams: this is a simple majority voting scheme.
///
template<class T>
class_tag_t dyn_slot_lambda_f<team<T>>::tag(
  const data::example &instance) const
{
  const auto classes(team_[0].slot_matrix_.cols());

  std::vector<unsigned> votes(classes);

  for (const auto &lambda : team_)
  {
    const auto where(lambda.slot(instance));
    ++votes[lambda.slot_class_[where]];
  }

  auto max(decltype(classes){0});
  for (auto i(max + 1); i < classes; ++i)
    if (votes[i] > votes[max])
      max = i;

  return max;
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T>
gaussian_lambda_f<T>::gaussian_lambda_f(const T &ind, data &d)
  : class_lambda_f<T>(ind, d), gauss_dist_(d.classes())
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);

  // Use the training set for lambdification.
  const data::dataset_t backup(d.dataset());
  d.dataset(data::training);
  fill_vector(d);
  d.dataset(backup);
}

///
/// \param[in] d the training set.
///
/// Sets up the data structures needed by the gaussian algorithm.
///
template<class T>
void gaussian_lambda_f<T>::fill_vector(data &d)
{
  assert(d.classes() > 1);

  src_interpreter<T> agent(this->prg_);

  // For a set of training data, we assume that the behaviour of a program
  // classifier is modelled using multiple Gaussian distributions, each of
  // which corresponds to a particular class. The distribution of a class is
  // determined by evaluating the program on the examples of the class in
  // the training set. This is done by taking the mean and standard deviation
  // of the program outputs for those training examples for that class.
  for (const auto &example : d)
  {
    const any res(agent.run(example.input));

    number val(res.empty() ? 0.0 : to<number>(res));
    const number cut(10000000.0);
    if (val > cut)
      val = cut;
    else if (val < -cut)
      val = -cut;

    gauss_dist_[example.tag()].add(val);
  }
}

///
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
class_tag_t gaussian_lambda_f<T>::tag(const data::example &example,
                                      number *val, number *sum) const
{
  const any res(src_interpreter<T>(this->prg_).run(example.input));
  const number x(res.empty() ? 0.0 : to<number>(res));

  number val_(0.0), val_sum_(0.0);
  unsigned probable_class(0);

  const auto size(gauss_dist_.size());
  for (auto i(decltype(size){0}); i < size; ++i)
  {
    const number distance(std::fabs(x - gauss_dist_[i].mean));
    const number variance(gauss_dist_[i].variance);

    number p(0.0);
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
/// \param[in] instance data to be classified.
/// \return the tag of the class that includes \a instance.
///
template<class T>
class_tag_t gaussian_lambda_f<T>::tag(const data::example &instance) const
{
  return tag(instance, nullptr, nullptr);
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
/// \return the class label associated with \a e.
///
template<class T>
class_tag_t binary_lambda_f<T>::tag(const data::example &e) const
{
  const any res(src_interpreter<T>(this->prg_).run(e.input));
  const number val(res.empty() ? -1.0 : to<number>(res));

  return val > 0.0 ? 1u : 0u;
}

#endif  // LAMBDA_F_INL_H
