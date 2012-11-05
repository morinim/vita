/**
 *
 *  \file src_evaluator.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "src_evaluator.h"
#include "individual.h"
#include "interpreter.h"

namespace vita
{
  ///
  /// \param[in] d pointer to data that the evaluator will use.
  /// \param[in] v vector of input variables of the dataset.
  ///
  src_evaluator::src_evaluator(data *d, std::vector<variable_ptr> *v)
    : dat_(d), var_(v)
  {
    assert(d);
    assert(v);
  }

  ///
  /// \param[in] d values to be stored in the input variables.
  ///
  void src_evaluator::load_vars(const data::example &d)
  {
    assert(d.input.size() == var_->size());

    for (unsigned i(0); i < var_->size(); ++i)
      (*var_)[i]->val = d.input[i];
  }

  ///
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0) and the accuracy
  ///         (percentage).
  ///
  score_t sum_of_errors_evaluator::operator()(const individual &ind)
  {
    assert(!dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    interpreter agent(ind);

    double err(0.0);
    int illegals(0);
    unsigned ok(0), total_nr(0);

    for (data::iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      err += error(agent, t, &illegals, &ok);

      ++total_nr;
    }

    assert(total_nr);

    // Note that we take the average error: this way fast() and operator()
    // outputs can be compared.
    return score_t(fitness_t(-err / total_nr),
                   static_cast<double>(ok) / static_cast<double>(total_nr));
  }

  ///
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0) and the accuracy
  ///         (percentage).
  ///
  /// This function is similar to operator()() but will skip 3 out of 4
  /// training instances, so it's faster ;-)
  ///
  score_t sum_of_errors_evaluator::fast(const individual &ind)
  {
    assert(!dat_->classes());
    assert(dat_->cbegin() != dat_->cend());

    interpreter agent(ind);

    double err(0.0);
    int illegals(0);
    unsigned ok(0), total_nr(0);
    unsigned counter(0);

    for (data::iterator t(dat_->begin()); t != dat_->end(); ++t)
      if (dat_->size() <= 20 || (counter++ % 5) == 0)
      {
        err += error(agent, t, &illegals, &ok);

        ++total_nr;
      }

    assert(total_nr);

    // Note that we take the average error: this way fast() and operator()
    // outputs can be compared.
    return score_t(fitness_t(-err / total_nr),
                   static_cast<double>(ok) / static_cast<double>(total_nr));
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \param[in,out] illegals number of illegals values found evaluating the
  ///                         current individual so far.
  /// \param[in,out] ok corret answers of the current individual so far.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t.
  ///
  double sae_evaluator::error(interpreter &agent, data::iterator t,
                              int *const illegals, unsigned *const ok)
  {
    load_vars(*t);

    const boost::any res(agent());
    double err;
    if (res.empty())
      err = std::pow(100.0, ++(*illegals));
    else
      err = std::fabs(interpreter::to_double(res) -
                      data::cast<double>(t->output));

    if (err > 0.1)
      ++t->difficulty;
    else
      ++(*ok);

    return err;
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \param[in,out] illegals number of illegals values found evaluating the
  ///                         current individual so far.
  /// \param[in,out] ok corret answers of the current individual so far.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t.
  ///
  double sse_evaluator::error(interpreter &agent, data::iterator t,
                              int *const illegals, unsigned *const ok)
  {
    load_vars(*t);

    const boost::any res(agent());
    double err;
    if (res.empty())
      err = std::pow(100.0, ++(*illegals));
    else
    {
      err = interpreter::to_double(res) - data::cast<double>(t->output);
      err *= err;
    }

    if (err > 0.1)
      ++t->difficulty;
    else
      ++(*ok);

    return err;
  }

  ///
  /// \param[in] agent interpreter used for the evaluation of the current
  ///                  individual. Note that this isn't a constant reference
  ///                  because the internal state of agent changes during
  ///                  evaluation; anyway this is an input-only parameter.
  /// \param[in] t the current training case.
  /// \param[in,out]
  /// \param[in,out] ok corret answers of the current individual so far.
  /// \return a measurement of the error of the current individual on the
  ///         training case \a t.
  ///
  double count_evaluator::error(interpreter &agent, data::iterator t,
                                int *const, unsigned *const ok)
  {
    load_vars(*t);

    const boost::any res(agent());

    const bool err(res.empty() ||
                   std::fabs(interpreter::to_double(res) -
                             data::cast<double>(t->output)) >=
                   float_epsilon);

    if (err)
      ++t->difficulty;
    else
      ++(*ok);

    return err ? 1.0 : 0.0;
  }

  ///
  /// \param[in] d training data.
  /// \param[in] v vector of input variables.
  /// \param[in] x_slot basic parameter for the Slotted Dynamic Class Boundary
  ///                   Determination algorithm.
  ///
  dyn_slot_evaluator::dyn_slot_evaluator(data *d, std::vector<variable_ptr> *v,
                                         unsigned x_slot)
    : src_evaluator(d, v), x_slot_(x_slot)
  {
    assert(d);
    assert(v);
    assert(x_slot);
  }

  ///
  /// \param[in] val the numeric value that should be mapped in the [0,1]
  ///                interval.
  ///
  inline
  double dyn_slot_evaluator::normalize_01(double val)
  {
    return 0.5 + std::atan(val) / 3.1415926535;
    // return 1.0 / (1 + std::exp(-val));
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] t input data for \a ind.
  /// \return the slot the input instance falls into.
  ///
  unsigned dyn_slot_evaluator::slot(const individual &ind,
                                    data::const_iterator t)
  {
    assert(ind.check());

    load_vars(*t);

    interpreter agent(ind);
    const boost::any res(agent());

    const unsigned n_slots(dat_->classes() * x_slot_);
    const unsigned last_slot(n_slots - 1);

    if (res.empty())
      return last_slot;

    const double val(interpreter::to_double(res));
    const unsigned where(static_cast<unsigned>(normalize_01(val) * n_slots));

    return (where >= n_slots) ? last_slot : where;
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[out] slot_matrix the main matrix of the dynamic slot algorithm.
  /// \param[out] slot_class slot_class[i] = "label of the predominant class"
  ///                        for the i-th slot.
  ///
  void dyn_slot_evaluator::fill_slots(const individual &ind,
                                      std::vector<uvect> *slot_matrix,
                                      uvect *slot_class,
                                      unsigned *dataset_size)
  {
    const data::dataset_t backup(dat_->dataset());
    dat_->dataset(data::training);

    assert(ind.check());
    assert(slot_matrix);
    assert(slot_class);

    std::vector<uvect> &sm(*slot_matrix);  // just for convenience
    const unsigned n_slots(dat_->classes() * x_slot_);
    assert(sm.size() == n_slots);

    for (unsigned i(0); i < n_slots; ++i)
    {
      assert(sm[i].size() == dat_->classes());

      for (unsigned j(0); j < sm[i].size(); ++j)
        sm[i][j] = 0;
    }

    assert(ind.check());

    interpreter agent(ind);

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slots[slot][class]).
    *dataset_size = 0;
    for (data::const_iterator t(dat_->cbegin()); t != dat_->cend(); ++t)
    {
      ++(*dataset_size);

      const unsigned where(slot(ind, t));

      ++sm[where][t->label()];
    }

    const unsigned unknown(dat_->classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (unsigned i(0); i < n_slots; ++i)
    {
      unsigned best_class(0);

      for (unsigned j(1); j < sm[i].size(); ++j)
        if (sm[i][j] >= sm[i][best_class])
          best_class = j;

      (*slot_class)[i] = sm[i][best_class] ? best_class : unknown;
    }

    dat_->dataset(backup);
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the fitness (greater is better, max is 0) and the accuracy
  ///         (percentage).
  ///
  /// Slotted Dynamic Class Boundary Determination
  ///
  score_t dyn_slot_evaluator::operator()(const individual &ind)
  {
    assert(ind.check());
    assert(dat_->classes() >= 2);

    const unsigned n_slots(dat_->classes() * x_slot_);
    std::vector<uvect> slot_matrix(n_slots, uvect(dat_->classes()));
    uvect slot_class(n_slots);
    unsigned count(0);
    fill_slots(ind, &slot_matrix, &slot_class, &count);

    assert(count);

    fitness_t err(0.0);
    for (unsigned i(0); i < n_slots; ++i)
      for (unsigned j(0); j < slot_matrix[i].size(); ++j)
        if (j != slot_class[i])
          err += slot_matrix[i][j];

    return score_t(fitness_t(-err),
                   static_cast<double>(count-err) / static_cast<double>(count));
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] eva evaluator for \a ind.
  ///
  dyn_slot_classifier::dyn_slot_classifier(const individual &ind,
                                           dyn_slot_evaluator *eva)
    : classifier(ind), eva_(eva)
  {
    assert(ind.check());
    assert(eva);

    const unsigned n_slots(eva_->dat_->classes() * eva_->x_slot_);
    typedef std::vector<unsigned> uvect;

    std::vector<uvect> slot_matrix(n_slots, uvect(eva_->dat_->classes()));
    uvect slot_class(n_slots);
    unsigned dummy(0);
    eva_->fill_slots(ind, &slot_matrix, &slot_class, &dummy);

    for (unsigned i(0); i < slot_class.size(); ++i)
      slot_class_.push_back(eva_->dat_->class_name(slot_class[i]));
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the class that include the \a instance.
  ///
  std::string dyn_slot_classifier::operator()(
    const data::example &instance) const
  {
    eva_->load_vars(instance);

    const boost::any res( (interpreter(ind_))() );

    const unsigned n_slots(slot_class_.size());
    assert(n_slots == eva_->dat_->classes() * eva_->x_slot_);
    unsigned where(n_slots - 1);

    if (!res.empty())
    {
      const double val(interpreter::to_double(res));

      where = static_cast<unsigned>(dyn_slot_evaluator::normalize_01(val) *
                                    n_slots);
      if (where >= n_slots)
        where = n_slots - 1;
    }

    return slot_class_[where];
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \return a vector containing the distribution parameters for each class
  ///         of training set.
  ///
  std::vector<distribution<double>> gaussian_evaluator::gaussian_distribution(
    const individual &ind)
  {
    const data::dataset_t backup(dat_->dataset());
    dat_->dataset(data::training);

    assert(dat_->classes() > 1);
    std::vector<distribution<double>> gauss(dat_->classes());

    assert(ind.check());
    interpreter agent(ind);

    // For a set of training data, we assume that the behaviour of a program
    // classifier is modelled using multiple Gaussian distributions, each of
    // which corresponds to a particular class. The distribution of a class is
    // determined by evaluating the program on the examples of the class in
    // the training set. This is done by taking the mean and standard deviation
    // of the program outputs for those training examples for that class.
    for (data::const_iterator t(dat_->cbegin()); t != dat_->cend(); ++t)
    {
      load_vars(*t);

      const boost::any res(agent());

      double val(res.empty() ? 0.0 : interpreter::to_double(res));
      const double cut(10000000);
      if (val > cut)
        val = cut;
      else if (val < -cut)
        val = -cut;

      gauss[t->label()].add(val);
    }

    dat_->dataset(backup);
    return gauss;
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// For details about this algorithm see:
  /// * "Using Gaussian Distribution to Construct Fitnesss Functions in Genetic
  ///   Programming for Multiclass Object Classification" - Mengjie Zhang, Will
  ///   Smart (december 2005).
  ///
  score_t gaussian_evaluator::operator()(const individual &ind)
  {
    std::vector<distribution<double>> gauss(gaussian_distribution(ind));

    /*
    fitness_t d(0.0);
    for (unsigned i(0); i < gauss.size(); ++i)
      for (unsigned j(i+1); j < gauss.size(); ++j)
      {
        const double mean_i(gauss[i].mean);
        const double mean_j(gauss[j].mean);
        const double stddev_i(gauss[i].standard_deviation());
        const double stddev_j(gauss[j].standard_deviation());

        const double delta(std::fabs(mean_j - mean_i));
        const double radius(stddev_j + stddev_i);

        d += 200.0 * std::log(delta) - radius;
      }

    unsigned ok(0), count(0);
    for (auto t(dat_->cbegin()); t != dat_->cend(); ++count, ++t)
      if (class_label(ind, *t, gauss) == t->label())
        ++ok;

    assert(count);

    d = double(ok) - double(count);
    return score_t(d, static_cast<double>(ok) / static_cast<double>(count));
    */

    assert(dat_->classes() > 1);

    fitness_t d(0.0);
    unsigned ok(0), count(0);
    for (auto t(dat_->cbegin()); t != dat_->cend(); ++count, ++t)
    {
      double max_val, sum;
      const unsigned probable_class(class_label(ind, *t, gauss, &max_val,
                                                &sum));

      if (probable_class == t->label())
      {
        ++ok;

        // Note:
        // * (sum - max_val) is the sum of the errors;
        // * (max_val - sum) is the opposite (we want a standardized fitness);
        // * (max_val - sum) / (dat_->classes() - 1) is the opposite of the
        //   average error;
        // * (max_val - 1.0) is the uncertainty about the right class;
        // * 0.001 is a scaling factor.
        d += (max_val - sum) / (dat_->classes() - 1) + 0.001 * (max_val - 1.0);
      }
      else
      {
        // Note:
        // * the maximum single class error is -1.0;
        // * the maximum average class error is -1.0 / dat_->classes();
        // So -1.0 is like to say that we have a complete failure.
        d -= 1.0;
      }
    }

    return score_t(d, static_cast<double>(ok) / static_cast<double>(count));
  }

  ///
  /// \param[in] ind program used for classification.
  /// \param[in] example input value whose class we are interested in.
  /// \param[in] gauss parameters of the gaussian distributions.
  /// \return the class of \a instance.
  ///
  unsigned gaussian_evaluator::class_label(
    const individual &ind,
    const data::example &example,
    const std::vector<distribution<double>> &gauss,
    double *prob, double *prob_sum)
  {
    load_vars(example);

    const boost::any res((interpreter(ind))());
    const double x(res.empty() ? 0.0 : interpreter::to_double(res));

    assert(dat_->classes() == gauss.size());

    *prob = *prob_sum = 0.0;
    unsigned probable_class(0);

    for (unsigned i(0); i < dat_->classes(); ++i)
    {
      const double distance(std::abs(x - gauss[i].mean));
      const double variance(gauss[i].variance);

      double p(0.0);
      if (variance == 0.0)
        if (distance == 0.0)
          p = 1.0;
        else
          p = 0.0;
      else
        p = std::exp(-0.5 * distance * distance / variance);

      if (p > *prob)
      {
        *prob = p;
        probable_class = i;
      }

      *prob_sum += p;
    }

    return probable_class;
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] eva evaluator for \a ind.
  ///
  gaussian_classifier::gaussian_classifier(const individual &ind,
                                           gaussian_evaluator *eva)
    : classifier(ind), eva_(eva)
  {
    assert(ind.check());
    assert(eva);

    gauss_ = eva_->gaussian_distribution(ind);
  }

  ///
  ///
  /// \param[in] example data to be classified.
  /// \return the class that include the \a instance.
  ///
  std::string gaussian_classifier::operator()(
    const data::example &example) const
  {
    double dummy;
    return eva_->dat_->class_name(eva_->class_label(ind_, example, gauss_,
                                                    &dummy, &dummy));
  }

  /*
  ///
  /// \param[in] ind an individual.
  /// \param[in] label
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t problem::binary_fitness(const vita::individual &ind,
                                    unsigned label) const
  {
    assert(dat_.classes() == 2);

    interpreter agent(ind);

    fitness_t err(0.0);
    int illegals(0);

    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars_.size(); ++i)
        vars_[i]->val = t->input[i];

      const boost::any res(agent());

      if (res.empty())
        err += std::pow(100.0,++illegals);
      else
      {
        const T val(boost::any_cast<T>(res));
        if ((t->label() == label && val < 0.0) ||
            (t->label() != label && val >= 0.0))
          err += std::fabs(val);
      }
    }

    return -err;
  }
  */
}  // namespace vita
