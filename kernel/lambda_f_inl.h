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

#if !defined(VITA_LAMBDA_F_INL_H)
#define      VITA_LAMBDA_F_INL_H

///
/// \param[in] prg the program (individual/team) to be lambdified.
///
template<class T, bool S>
basic_reg_lambda_f<T, S>::basic_reg_lambda_f(const T &prg)
  : detail::core_reg_lambda_f<T, S>(prg)
{
  assert(debug());
}

///
/// \param[in] e input example for the lambda function.
/// \return the output value associated with \a e.
///
template<class T, bool S>
any basic_reg_lambda_f<T, S>::operator()(const data::example &e) const
{
  // We use tag dispatching by instance (i.e. to delegate to an implementation
  // function that receives standard arguments plus a dummy argument based on a
  // compile-time condition). Usually this is much easier to debug and get
  // right that the std::enable_if solution.
  // Moreover this is almost guaranteed to be optimized away by a decent
  // compiler.
  return eval(e, is_team<T>());
}

template<class T, bool S>
any basic_reg_lambda_f<T, S>::eval(const data::example &e,
                                   std::false_type) const
{
  return this->int_.run(e.input);
}

template<class T, bool S>
any basic_reg_lambda_f<T, S>::eval(const data::example &e,
                                   std::true_type) const
{
  number avg(0), count(0);

  // Calculate the running average.
  for (const auto &core : this->team_)
  {
    const auto res(core.int_.run(e.input));

    if (!res.empty())
      avg += (to<number>(res) - avg) / ++count;
  }

  return count > 0.0 ? any(avg) : any();
}

///
/// \param[in] a a value produced by lambda_f::operator().
/// \return the string version of \a a.
///
template<class T, bool S>
std::string basic_reg_lambda_f<T, S>::name(const any &a) const
{
  return std::to_string(to<number>(a));
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::debug() const
{
  return detail::core_reg_lambda_f<T, S>::debug();
}

///
/// \param[in] in input stream.
/// \return \c true if the lambda has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::load(std::istream &in)
{
  return detail::core_reg_lambda_f<T, S>::load(in);
}

///
/// \param[out] out output stream.
/// \return \c true if lambda was saved correctly.
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::save(std::ostream &out) const
{
  return detail::core_reg_lambda_f<T, S>::save(out);
}

///
/// \param[in] d the training set.
///
template<class T, bool N>
basic_class_lambda_f<T, N>::basic_class_lambda_f(const data &d)
  : detail::class_names<N>(d)
{
}

///
/// \param[in] e data to be classified.
/// \return the label of the class that includes \a instance (wrapped in class
///         any).
///
template<class T, bool N>
any basic_class_lambda_f<T, N>::operator()(const data::example &e) const
{
  return any(tag(e).first);
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool N>
bool basic_class_lambda_f<T, N>::save(std::ostream &out) const
{
  return detail::class_names<N>::save(out);
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the operation fails the object isn't modified.
///
template<class T, bool N>
bool basic_class_lambda_f<T, N>::load(std::istream &in)
{
  return detail::class_names<N>::load(in);
}

///
/// \param[in] a id of a class.
/// \return the name of class \a a.
///
template<class T, bool N>
std::string basic_class_lambda_f<T, N>::name(const any &a) const
{
  return detail::class_names<N>::string(a);
}

///
/// \param[in] x the numeric value (a real number in the [-inf;+inf] range)
///              that should be mapped in the [0,1] interval.
/// \return a number in the [0,1] range.
///
/// This is a sigmoid function (it is a bounded real function, "S" shaped,
/// with positive derivative everywhere).
/// Among the various uses there is continuous value discretization when we
/// don't know an upper/lower bound for the continuos value.
///
/// \see
/// * <http://en.wikipedia.org/wiki/Sigmoid_function>
/// * <http://en.wikipedia.org/wiki/Generalised_logistic_function>
///
template<class T, bool S, bool N>
number basic_dyn_slot_lambda_f<T, S, N>::normalize_01(number x)
{
  // return (1.0 + x / (1 + std::fabs(x))) / 2.0;  // Algebraic function

  return 0.5 + std::atan(x) / 3.1415926535;        // Arctangent

  // return 0.5 + std::tanh(x) / 2.0;              // Hyperbolic tangent

  // return 1.0 / (1.0 + std::exp(-x));            // Logistic function
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
template<class T, bool S, bool N>
basic_dyn_slot_lambda_f<T, S, N>::basic_dyn_slot_lambda_f(const T &ind,
                                                          data &d,
                                                          unsigned x_slot)
  : basic_class_lambda_f<T, N>(d), lambda_(ind),
    slot_matrix_(d.classes() * x_slot, d.classes()),
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
/// \param[in] d the training set.
/// \param[in] x_slot number of slots for each class of the training set.
///
/// Sets up the data structures needed by the 'dynamic slot' algorithm.
///
template<class T, bool S, bool N>
void basic_dyn_slot_lambda_f<T, S, N>::fill_matrix(data &d, unsigned x_slot)
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
template<class T, bool S, bool N>
unsigned basic_dyn_slot_lambda_f<T, S, N>::slot(const data::example &e) const
{
  const any res(lambda_(e));

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
template<class T, bool S, bool N>
double basic_dyn_slot_lambda_f<T, S, N>::training_accuracy() const
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
/// \return the class of \a instance (numerical id) and the confidence level
///         (in the range [0,1]).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_dyn_slot_lambda_f<T, S, N>::tag(
  const data::example &instance) const
{
  const auto s(slot(instance));
  const auto classes(slot_matrix_.cols());

  unsigned total(0);
  for (auto j(decltype(classes){0}); j < classes; ++j)
    total += slot_matrix_(s, j);

  const auto ok(slot_matrix_(s, slot_class_[s]));

  const double confidence(!total ? 0.5 : static_cast<double>(ok) / total);

  return {slot_class_[s], confidence};
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  if (!slot_matrix_.save(out))
    return false;

  // Don't need to save slot_class_.size() since it's equal to
  // slot_matrix_.rows()
  for (const auto s : slot_class_)
    out << s << std::endl;

  out << dataset_size_ << std::endl;

  if (!basic_class_lambda_f<T, N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load(std::istream &in)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, detail::is_true<S>());
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// This is part of the tag dispatching method used by the
/// basic_dyn_slot_lambda_f::load method.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load_(std::istream &in, std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in))
    return false;

  decltype(slot_matrix_) m;
  if (!m.load(in))
    return false;

  decltype(slot_class_) s(slot_matrix_.rows());
  for (auto &e : s)
    if (!(in >> e))
      return false;

  decltype(dataset_size_) d;
  if (!(in >> d))
    return false;

  if (!basic_class_lambda_f<T, N>::load(in))
    return false;

  lambda_ = l;
  slot_matrix_ = m;
  slot_class_ = s;
  dataset_size_ = d;

  return true;
}

template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::load_(std::istream &, std::false_type)
{
  return false;
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::debug() const
{
  if (slot_matrix_.cols() <= 1)  // Too few classes
    return false;

  if (slot_matrix_.rows() != slot_class_.size())
    return false;

  return lambda_.debug();
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T, bool S, bool N>
basic_gaussian_lambda_f<T, S, N>::basic_gaussian_lambda_f(const T &ind,
                                                          data &d)
  : basic_class_lambda_f<T, N>(d), lambda_(ind), gauss_dist_(d.classes())
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() > 1);

  // Use the training set for lambdification.
  const auto backup(d.dataset());
  d.dataset(data::training);
  fill_vector(d);
  d.dataset(backup);
}

///
/// \param[in] d the training set.
///
/// Sets up the data structures needed by the gaussian algorithm.
///
template<class T, bool S, bool N>
void basic_gaussian_lambda_f<T, S, N>::fill_vector(data &d)
{
  assert(d.classes() > 1);

  // For a set of training data, we assume that the behaviour of a program
  // classifier is modelled using multiple Gaussian distributions, each of
  // which corresponds to a particular class. The distribution of a class is
  // determined by evaluating the program on the examples of the class in
  // the training set. This is done by taking the mean and standard deviation
  // of the program outputs for those training examples for that class.
  for (const auto &example : d)
  {
    const any res(lambda_(example));

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
/// \return the class of \a instance (numerical id) and the confidence level
///         (how sure you can be that \a example is properly classified. The
///         value is in [0;1] range and the sum of all the confidence levels of
///         each class is 1).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_gaussian_lambda_f<T, S, N>::tag(
  const data::example &example) const
{
  const any res(lambda_(example));
  const number x(res.empty() ? 0.0 : to<number>(res));

  number val_(0.0), sum_(0.0);
  class_t probable_class(0);

  const auto classes(static_cast<unsigned>(gauss_dist_.size()));
  for (auto i(decltype(classes){0}); i < classes; ++i)
  {
    const number distance(std::fabs(x - gauss_dist_[i].mean()));
    const number variance(gauss_dist_[i].variance());

    number p(0.0);
    if (issmall(variance))     // These are borderline cases
      if (issmall(distance))   // These are borderline cases
        p = 1.0;
      else
        p = 0.0;
    else                       // This is the standard case
      p = std::exp(-distance * distance / variance);

    if (p > val_)
    {
      val_ = p;
      probable_class = i;
    }

    sum_ += p;
  }

  // Normalized confidence value.
  // Do not change sum_ > 0.0 with
  // - issmall(sum_) => when sum_ is small, val_ is smaller and the division
  //                    works well.
  // - sum_ => it's the same thing but will produce a warning with
  //           -Wfloat-equal
  const double confidence(sum_ > 0.0 ? val_ / sum_ : 0.0);

  return {probable_class, confidence};
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  out << gauss_dist_.size() << std::endl;
  for (const auto g : gauss_dist_)
    if (!g.save(out))
      return false;

  if (!basic_class_lambda_f<T, N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load(std::istream &in)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, detail::is_true<S>());
}

template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load_(std::istream &in, std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in))
    return false;

  typename decltype(gauss_dist_)::size_type n;
  if (!(in >> n))
    return false;

  decltype(gauss_dist_) g(n);
  for (auto &d : g)
    if (!d.load(in))
      return false;

  if (!basic_class_lambda_f<T, N>::load(in))
    return false;

  lambda_ = l;
  gauss_dist_ = g;

  return true;
}

template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::load_(std::istream &, std::false_type)
{
  return false;
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::debug() const
{
  return lambda_.debug();
}

///
/// \param[in] ind individual "to be transformed" into a lambda function.
/// \param[in] d the training set.
///
template<class T, bool S, bool N>
basic_binary_lambda_f<T, S, N>::basic_binary_lambda_f(const T &ind, data &d)
  : basic_class_lambda_f<T, N>(d), lambda_(ind)
{
  assert(ind.debug());
  assert(d.debug());
  assert(d.classes() == 2);
}

///
/// \param[in] e input example for the lambda function.
/// \return the class of \a instance (numerical id) and the confidence level
///         (in the range [0,1]).
///
template<class T, bool S, bool N>
std::pair<class_t, double> basic_binary_lambda_f<T, S, N>::tag(
  const data::example &e) const
{
  const any res(lambda_(e));
  const number val(res.empty() ? 0.0 : to<number>(res));

  return {val > 0.0 ? 1u : 0u, std::fabs(val)};
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::debug() const
{
  return lambda_.debug();
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda on persistent storage.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  if (!basic_class_lambda_f<T, N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load(std::istream &in)
{
  // Tag dispatching to select the appropriate method.
  // Note that there is an implementation of operator= only for S==true.
  // Without tag dispatching the compiler would need a complete implementation
  // (but we haven't a reasonable/general solution for the S==false case).
  return load_(in, detail::is_true<S>());
}

template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load_(std::istream &in, std::true_type)
{
  decltype(lambda_) l(lambda_);
  if (!l.load(in))
    return false;

  if (!basic_class_lambda_f<T, N>::load(in))
    return false;

  lambda_ = l;

  return true;
}

template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::load_(std::istream &, std::false_type)
{
  return false;
}

///
/// \param[in] t team "to be transformed" into a lambda function.
/// \param[in] d the training set.
/// \param[in] args auxiliary parameters for the specific lambda function.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
template<class... Args>
team_class_lambda_f<T, S, N, L, C>::team_class_lambda_f(const team<T> &t,
                                                        data &d,
                                                        Args&&... args)
  : basic_class_lambda_f<team<T>, N>(d), classes_(d.classes())
{
  team_.reserve(t.individuals());
  for (const auto &ind : t)
    team_.emplace_back(ind, d, std::forward<Args>(args)...);
}

///
/// \param[in] instance data to be classified.
/// \return the class of \a instance (numerical id) and the confidence level
///         (in the range [0,1]).
///
/// Specialized method for teams.
///
/// * \c team_composition::mv the class which most of the individuals predict
///   for a given example is selected as team output.
/// * \c team_composition::wta the winner is the individual with the highest
///   confidence in its decision. Specialization may emerge if different
///   members of the team win this contest for different fitness cases (of
///   curse, it is not a feasible alternative to select the member with the
///   best fitness. Then a decision on unknown data is only possible if the
///   right outputs are known in advance and is not made by the team itself).
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
std::pair<class_t, double> team_class_lambda_f<T, S, N, L, C>::tag(
  const data::example &instance) const
{
  if (C == team_composition::wta)
  {
    const auto size(team_.size());
    auto best(team_[0].tag(instance));

    for (auto i(decltype(size){1}); i < size; ++i)
    {
      const auto res(team_[i].tag(instance));

      if (res.second > best.second)
        best = res;
    }

    return best;
  }
  else if (C == team_composition::mv)
  {
    std::vector<unsigned> votes(classes_);

    for (const auto &lambda : team_)
      ++votes[lambda.tag(instance).first];

    class_t max(0);
    for (auto i(max + 1); i < classes_; ++i)
      if (votes[i] > votes[max])
        max = i;

    return {max, static_cast<double>(votes[max]) /
            static_cast<double>(team_.size())};
  }
}

///
/// \param[out] out output stream.
/// \return true on success.
///
/// Saves the lambda team on persistent storage.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::save(std::ostream &out) const
{
  out << classes_ << std::endl;

  out << team_.size() << std::endl;
  for (const auto &i : team_)
    if (!i.save(out))
      return false;

  if (!basic_class_lambda_f<team<T>, N>::save(out))
    return false;

  return out.good();
}

///
/// \param[in] in input stream.
/// \return true on success.
///
/// Loads the lambda team from persistent storage.
///
/// \note
/// If the load operation isn't successful the current lambda isn't modified.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::load(std::istream &in)
{
  decltype(classes_) cl;
  if (!(in >> cl))
    return false;

  typename decltype(team_)::size_type s;
  if (!(in >> s))
    return false;

  decltype(team_) t;
  for (unsigned i(0); i < s; ++i)
  {
    typename decltype(team_)::value_type lambda(team_[0]);
    if (!lambda.load(in))
      return false;
    t.push_back(lambda);
  }

  if (!basic_class_lambda_f<team<T>, N>::load(in))
    return false;

  team_ = t;
  classes_ = cl;

  return true;
}

///
/// \return \c true if the object passes the internal consistency check.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::debug() const
{
  for (const auto &l : team_)
    if (!l.debug())
      return false;

  return classes_ > 1;
}

#endif  // Include guard
