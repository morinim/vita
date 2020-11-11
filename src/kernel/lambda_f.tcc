/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LAMBDA_F_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_LAMBDA_F_TCC)
#define      VITA_LAMBDA_F_TCC

template<class T, bool S>
const std::string basic_reg_lambda_f<T, S>::SERIALIZE_ID(
  is_team<T>() ? "TEAM_REG_LAMBDA_F" : "REG_LAMBDA_F");

template<class T, bool S, bool N>
const std::string basic_dyn_slot_lambda_f<T, S, N>::SERIALIZE_ID(
  "DYN_SLOT_LAMBDA_F");

template<class T, bool S, bool N>
const std::string basic_gaussian_lambda_f<T, S, N>::SERIALIZE_ID(
  "GAUSSIAN_LAMBDA_F");

template<class T, bool S, bool N>
const std::string basic_binary_lambda_f<T, S, N>::SERIALIZE_ID(
  "BINARY_LAMBDA_F");

template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
const std::string team_class_lambda_f<T, S, N, L, C>::SERIALIZE_ID(
  "TEAM_" + L<T, S, N>::SERIALIZE_ID);

///
/// \param[in] prg the program (individual/team) to be lambdified
///
template<class T, bool S>
basic_reg_lambda_f<T, S>::basic_reg_lambda_f(const T &prg)
  : detail::reg_lambda_f_storage<T, S>(prg)
{
  Ensures(is_valid());
}

///
/// \param[in] in input stream
/// \param[in] ss active symbol set
///
template<class T, bool S>
basic_reg_lambda_f<T, S>::basic_reg_lambda_f(std::istream &in,
                                             const symbol_set &ss)
  : detail::reg_lambda_f_storage<T, S>(in, ss)
{
  static_assert(S, "reg_lambda_f requires storage for de-serialization");

  Ensures(is_valid());
}

///
/// \param[in] e input example for the lambda function
/// \return      the output value associated with `e`
///
template<class T, bool S>
value_t basic_reg_lambda_f<T, S>::operator()(const dataframe::example &e) const
{
  // Here using *tag dispatching by instance*: main function delegates to an
  // implementation function that receives standard arguments plus a dummy
  // argument based on a compile-time condition.
  // Usually this is much easier to debug and get right that the
  // `std::enable_if` solution.
  // Moreover this is almost guaranteed to be optimized away by a decent
  // compiler.
  return eval(e, is_team<T>());
}

template<class T, bool S>
value_t basic_reg_lambda_f<T, S>::eval(const dataframe::example &e,
                                       std::false_type) const
{
  return this->run(e.input);
}

template<class T, bool S>
value_t basic_reg_lambda_f<T, S>::eval(const dataframe::example &e,
                                       std::true_type) const
{
  D_DOUBLE avg(0), count(0);

  // Calculate the running average.
  for (const auto &core : this->team_)
  {
    const auto res(core.run(e.input));

    if (has_value(res))
      avg += (lexical_cast<D_DOUBLE>(res) - avg) / ++count;
  }

  if (count > 0.0)
    return avg;

  return {};
}

///
/// \return a *failed* status
///
/// \warning This function is useful only for classification tasks.
///
template<class T, bool S>
classification_result basic_reg_lambda_f<T, S>::tag(
  const dataframe::example &) const
{
  return {0, 0};
}

///
/// \param[in] a value produced by basic_lambda_f::operator()
/// \return      the string version of `a`
///
template<class T, bool S>
std::string basic_reg_lambda_f<T, S>::name(const value_t &a) const
{
  return lexical_cast<std::string>(a);
}

///
/// Calls (dynamic dispatch) polymhorphic model_metric `m` on `this`.
///
/// \param[in] m a metric we are evaluating
/// \param[in] d a dataset
/// \return      the value of `this` according to metric `m`
///
template<class T, bool S>
double basic_reg_lambda_f<T, S>::measure(const model_metric &m,
                                         const dataframe &d) const
{
  return m(this, d);
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::is_valid() const
{
  return detail::reg_lambda_f_storage<T, S>::is_valid();
}

///
/// Saves the object on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` if lambda was saved correctly
///
template<class T, bool S>
bool basic_reg_lambda_f<T, S>::save(std::ostream &out) const
{
  return detail::reg_lambda_f_storage<T, S>::save(out);
}

///
/// \param[in] d the training set
///
template<bool N>
basic_class_lambda_f<N>::basic_class_lambda_f(const dataframe &d)
  : detail::class_names<N>(d)
{
}

///
/// \param[in] e example to be classified
/// \return      the label of the class that includes `e`
///
template<bool N>
value_t basic_class_lambda_f<N>::operator()(const dataframe::example &e) const
{
  return static_cast<D_INT>(this->tag(e).label);
}

///
/// Calls (dynamic dispatch) polymhorphic model_metric `m` on `this`.
///
/// \param[in] m a metric we are evaluating
/// \param[in] d a dataset
/// \return      the value of `this` according to metric `m`
///
template<bool N>
double basic_class_lambda_f< N>::measure(const model_metric &m,
                                         const dataframe &d) const
{
  return m(this, d);
}

///
/// \param[in] a id of a class
/// \return      the name of class `a`
///
template<bool N>
std::string basic_class_lambda_f<N>::name(const value_t &a) const
{
  return detail::class_names<N>::string(a);
}

///
/// \param[in] ind    individual "to be transformed" into a lambda function
/// \param[in] d      the training set
/// \param[in] x_slot number of slots for each class of the training set
///
template<class T, bool S, bool N>
basic_dyn_slot_lambda_f<T, S, N>::basic_dyn_slot_lambda_f(const T &ind,
                                                          dataframe &d,
                                                          unsigned x_slot)
  : basic_class_lambda_f<N>(d), lambda_(ind),
    slot_matrix_(d.classes() * x_slot, d.classes()),
    slot_class_(d.classes() * x_slot), dataset_size_(0)
{
  Expects(d.classes() > 1);
  Expects(x_slot);

  fill_matrix(d, x_slot);

  Ensures(is_valid());
}

///
/// Constructs the object reading data from an input stream.
///
/// \param[in] in input stream
/// \param[in] ss active symbol set
///
template<class T, bool S, bool N>
basic_dyn_slot_lambda_f<T, S, N>::basic_dyn_slot_lambda_f(std::istream &in,
                                                          const symbol_set &ss)
  : basic_class_lambda_f<N>(), lambda_(in, ss), slot_matrix_(), slot_class_(),
    dataset_size_()
{
  static_assert(
    S, "dyn_slot_lambda_f requires storage space for de-serialization");

  if (!slot_matrix_.load(in))
    throw exception::data_format(
      "Cannot read dyn_slot_lambda_f matrix component");

  std::generate_n(std::back_inserter(slot_class_), slot_matrix_.rows(),
                  [&in]
                  {
                    std::size_t v;
                    if (!(in >> v))
                      throw exception::data_format(
                        "Cannot read dyn_slot_lambda_f slot_class component");
                    return v;
                  });

  if (!(in >> dataset_size_))
    throw exception::data_format(
      "Cannot read dyn_slot_lambda_f dataset_size component");

  if (!detail::class_names<N>::load(in))
    throw exception::data_format(
      "Cannot read dyn_slot_lambda_f class_names component");

  Ensures(is_valid());
}

///
/// Sets up the data structures needed by the 'dynamic slot' algorithm.
///
/// \param[in] d      the training set
/// \param[in] x_slot number of slots for each class of the training set
///
template<class T, bool S, bool N>
void basic_dyn_slot_lambda_f<T, S, N>::fill_matrix(dataframe &d,
                                                   unsigned x_slot)
{
  Expects(d.classes() > 1);
  Expects(x_slot);

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

    ++slot_matrix_(slot(example), label(example));
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
/// \param[in] e input data
/// \return      the slot example `e` falls into
///
template<class T, bool S, bool N>
std::size_t basic_dyn_slot_lambda_f<T,S,N>::slot(
  const dataframe::example &e) const
{
  const auto res(lambda_(e));

  const auto ns(slot_matrix_.rows());
  const auto last_slot(ns - 1);
  if (!has_value(res))
    return last_slot;

  const auto val(lexical_cast<D_DOUBLE>(res));
  const auto where(discretization(val, last_slot));

  return (where >= ns) ? last_slot : where;
}

///
/// \return the accuracy of the lambda function on the training set
///
template<class T, bool S, bool N>
double basic_dyn_slot_lambda_f<T, S, N>::training_accuracy() const
{
  double ok(0.0);

  const auto slots(slot_matrix_.rows());

  for (auto i(decltype(slots){0}); i < slots; ++i)
    ok += slot_matrix_(i, slot_class_[i]);

  assert(dataset_size_ >= ok);

  return ok / dataset_size_;
}

///
/// \param[in] instance data to be classified
/// \return             the class of `instance` (numerical id) and the
///                     confidence level (in the range `[0,1]`)
///
template<class T, bool S, bool N>
classification_result basic_dyn_slot_lambda_f<T, S, N>::tag(
  const dataframe::example &instance) const
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
/// Saves the lambda on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` on success
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
  for (auto s : slot_class_)
    if (!(out << s << '\n'))
      return false;

  if (!(out << dataset_size_ << '\n'))
    return false;

  return detail::class_names<N>::save(out);
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, bool S, bool N>
bool basic_dyn_slot_lambda_f<T, S, N>::is_valid() const
{
  if (slot_matrix_.cols() <= 1)  // too few classes
    return false;

  if (slot_matrix_.rows() != slot_class_.size())
    return false;

  return true;
}

///
/// \param[in] ind individual "to be transformed" into a lambda function
/// \param[in] d   the training set
///
template<class T, bool S, bool N>
basic_gaussian_lambda_f<T, S, N>::basic_gaussian_lambda_f(const T &ind,
                                                          dataframe &d)
  : basic_class_lambda_f<N>(d), lambda_(ind), gauss_dist_(d.classes())
{
  Expects(d.classes() > 1);

  fill_vector(d);

  Ensures(is_valid());
}

///
/// Constructs the object reading data from an input stream.
///
/// \param[in] in input stream
/// \param[in] ss active symbol set
///
template<class T, bool S, bool N>
basic_gaussian_lambda_f<T, S, N>::basic_gaussian_lambda_f(std::istream &in,
                                                          const symbol_set &ss)
  : basic_class_lambda_f<N>(), lambda_(in, ss), gauss_dist_()
{
  static_assert(
    S, "gaussian_lambda_f requires storage space for de-serialization");

  typename decltype(gauss_dist_)::size_type n;
  if (!(in >> n))
    throw exception::data_format(
      "Cannot read gaussian_lambda_f size component");

  for (decltype(n) i(0); i < n; ++i)
  {
    distribution<number> d;
    if (!d.load(in))
      throw exception::data_format(
        "Cannot read gaussian_lambda_f distribution component");

    gauss_dist_.push_back(d);
  }

  if (!detail::class_names<N>::load(in))
      throw exception::data_format(
        "Cannot read gaussian_lambda_f class_names component");

  Ensures(is_valid());
}

///
/// Sets up the data structures needed by the gaussian algorithm.
///
/// \param[in] d the training set
///
template<class T, bool S, bool N>
void basic_gaussian_lambda_f<T, S, N>::fill_vector(dataframe &d)
{
  Expects(d.classes() > 1);

  // For a set of training data, we assume that the behaviour of a program
  // classifier is modelled using multiple Gaussian distributions, each of
  // which corresponds to a particular class. The distribution of a class is
  // determined by evaluating the program on the examples of the class in
  // the training set. This is done by taking the mean and standard deviation
  // of the program outputs for those training examples for that class.
  for (const auto &example : d)
  {
    const auto res(lambda_(example));

    number val(has_value(res) ? lexical_cast<D_DOUBLE>(res) : 0.0);
    const number cut(10000000.0);
    if (val > cut)
      val = cut;
    else if (val < -cut)
      val = -cut;

    gauss_dist_[label(example)].add(val);
  }
}

///
/// \param[in] example input value whose class we are interested in
/// \return            the class of `example` (numerical id) and the confidence
///                    level (how sure you can be that `example` is properly
///                    classified. The value is in the `[0,1]` interval and the
///                    sum of all the confidence levels of each class equals
///                    `1`)
///
template<class T, bool S, bool N>
classification_result basic_gaussian_lambda_f<T, S, N>::tag(
  const dataframe::example &example) const
{
  const auto res(lambda_(example));
  const number x(has_value(res) ? lexical_cast<D_DOUBLE>(res) : 0.0);

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
/// Saves the lambda on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` on success
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  if (!(out << gauss_dist_.size() << '\n'))
      return false;

  for (const auto &g : gauss_dist_)
    if (!g.save(out))
      return false;

  return detail::class_names<N>::save(out);
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, bool S, bool N>
bool basic_gaussian_lambda_f<T, S, N>::is_valid() const
{
  return true;
}

///
/// Constructs the object reading data from an input stream.
///
/// \param[in] ind individual "to be transformed" into a lambda function
/// \param[in] d   the training set
///
template<class T, bool S, bool N>
basic_binary_lambda_f<T, S, N>::basic_binary_lambda_f(const T &ind,
                                                      dataframe &d)
  : basic_class_lambda_f<N>(d), lambda_(ind)
{
  Expects(d.classes() == 2);

  Ensures(is_valid());
}

///
/// \param[in] in input stream
/// \param[in] ss active symbol set
///
template<class T, bool S, bool N>
basic_binary_lambda_f<T, S, N>::basic_binary_lambda_f(std::istream &in,
                                                      const symbol_set &ss)
  : basic_class_lambda_f<N>(), lambda_(in, ss)
{
  static_assert(
    S, "binary_lambda_f requires storage space for de-serialization");

  if (!detail::class_names<N>::load(in))
      throw exception::data_format(
        "Cannot read binary_lambda_f class_names component");

  Ensures(is_valid());
}

///
/// \param[in] e input example for the lambda function
/// \return      the class of `e` (numerical id) and the confidence level (in
///              the `[0,1]` interval)
///
template<class T, bool S, bool N>
classification_result basic_binary_lambda_f<T, S, N>::tag(
  const dataframe::example &e) const
{
  const auto res(lambda_(e));
  const number val(has_value(res) ? lexical_cast<D_DOUBLE>(res) : 0.0);

  return {val > 0.0 ? 1u : 0u, std::fabs(val)};
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::is_valid() const
{
  return true;
}

///
/// Saves the lambda on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` on success
///
template<class T, bool S, bool N>
bool basic_binary_lambda_f<T, S, N>::save(std::ostream &out) const
{
  if (!lambda_.save(out))
    return false;

  return detail::class_names<N>::save(out);
}

///
/// \param[in] t    team "to be transformed" into a lambda function
/// \param[in] d    the training set
/// \param[in] args auxiliary parameters for the specific lambda function
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
template<class... Args>
team_class_lambda_f<T, S, N, L, C>::team_class_lambda_f(const team<T> &t,
                                                        dataframe &d,
                                                        Args&&... args)
  : basic_class_lambda_f<N>(d), classes_(d.classes())
{
  team_.reserve(t.individuals());
  for (const auto &ind : t)
    team_.emplace_back(ind, d, std::forward<Args>(args)...);
}

///
/// Constructs the object reading data from an input stream.
///
/// \param[in] in input stream
/// \param[in] ss active symbol set
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
team_class_lambda_f<T, S, N, L, C>::team_class_lambda_f(std::istream &in,
                                                        const symbol_set &ss)
  : basic_class_lambda_f<N>(), classes_()
{
  static_assert(
    S, "team_class_lambda_f requires storage space for de-serialization");

  if (!(in >> classes_))
    throw exception::data_format("Cannot read number of classes");

  typename decltype(team_)::size_type s;
  if (!(in >> s))
    throw exception::data_format("Cannot read team size");

  team_.reserve(s);
  for (unsigned i(0); i < s; ++i)
    team_.emplace_back(in, ss);

  if (!detail::class_names<N>::load(in))
    throw exception::data_format("Cannot read class_names");
}

///
/// Specialized method for teams.
///
/// \param[in] instance data to be classified
/// \return             the class of `instance` (numerical id) and the
///                     confidence level (in the `0,1]` interval)
///
/// * `team_composition::mv` the class which most of the individuals predict
///   for a given example is selected as team output.
/// * `team_composition::wta` the winner is the individual with the highest
///   confidence in its decision. Specialization may emerge if different
///   members of the team win this contest for different fitness cases (of
///   curse, it is not a feasible alternative to select the member with the
///   best fitness. Then a decision on unknown data is only possible if the
///   right outputs are known in advance and is not made by the team itself).
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
classification_result team_class_lambda_f<T, S, N, L, C>::tag(
  const dataframe::example &instance) const
{
  if constexpr (C == team_composition::wta)
  {
    const auto size(team_.size());
    auto best(team_[0].tag(instance));

    for (auto i(decltype(size){1}); i < size; ++i)
    {
      const auto res(team_[i].tag(instance));

      if (res.sureness > best.sureness)
        best = res;
    }

    return best;
  }
  else if constexpr (C == team_composition::mv)
  {
    std::vector<unsigned> votes(classes_);

    for (const auto &lambda : team_)
      ++votes[lambda.tag(instance).label];

    class_t max(0);
    for (auto i(max + 1); i < classes_; ++i)
      if (votes[i] > votes[max])
        max = i;

    return {max, static_cast<double>(votes[max]) /
            static_cast<double>(team_.size())};
  }
}

///
/// Saves the lambda team on persistent storage.
///
/// \param[out] out output stream
/// \return         `true` on success
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::save(std::ostream &out) const
{
  if (!(out << classes_ << '\n'))
    return false;

  if (!(out << team_.size() << '\n'))
    return false;

  for (const auto &i : team_)
    if (!i.save(out))
      return false;

  return detail::class_names<N>::save(out);
}

///
/// \return Class ID used for serialization.
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
std::string team_class_lambda_f<T, S, N, L, C>::serialize_id() const
{
  Expects(team_.size());
  return "TEAM_" + L<T, S, N>::SERIALIZE_ID;
}

///
/// \return `true` if the object passes the internal consistency check
///
template<class T, bool S, bool N, template<class, bool, bool> class L,
         team_composition C>
bool team_class_lambda_f<T, S, N, L, C>::is_valid() const
{
  return classes_ > 1;
}

namespace serialize
{

namespace lambda
{

namespace detail
{
using build_func = std::unique_ptr<basic_src_lambda_f> (*)(std::istream &,
                                                           const symbol_set &);

template<class U>
std::unique_ptr<basic_src_lambda_f> build(std::istream &in,
                                          const symbol_set &ss)
{
  return std::make_unique<U>(in, ss);
}

extern std::map<std::string, build_func> factory_;
}

///
/// Allows insertion of user defined classificators.
///
template<class U>
bool insert(const std::string &id)
{
  Expects(!id.empty());
  return detail::factory_.insert({id, detail::build<U>}).second;
}

template<class T>
std::unique_ptr<basic_src_lambda_f> load(std::istream &in,
                                         const symbol_set &ss)
{
  if (detail::factory_.find(reg_lambda_f<T>::SERIALIZE_ID)
      == detail::factory_.end())
  {
    insert<reg_lambda_f<T>>(reg_lambda_f<T>::SERIALIZE_ID);
    insert<dyn_slot_lambda_f<T>>(dyn_slot_lambda_f<T>::SERIALIZE_ID);
    insert<gaussian_lambda_f<T>>(gaussian_lambda_f<T>::SERIALIZE_ID);
    insert<binary_lambda_f<T>>(binary_lambda_f<T>::SERIALIZE_ID);
  }

  std::string id;
  if (!(in >> id))
    return nullptr;

  const auto iter(detail::factory_.find(id));
  if (iter != detail::factory_.end())
    return iter->second(in, ss);

  return nullptr;
}

}  // namespace lambda

}  // namespace serialize

#endif  // include guard
