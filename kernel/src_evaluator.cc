/**
 *
 *  \file src_evaluator.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kernel/src_evaluator.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"

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
  void src_evaluator::load_vars(const data::value_type &d)
  {
    assert(d.input.size() == var_->size());

    for (unsigned i(0); i < var_->size(); ++i)
      (*var_)[i]->val = d.input[i];
  }

  ///
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0).
  ///
  fitness_t abs_evaluator::operator()(const individual &ind)
  {
    assert(!dat_->classes());

    interpreter agent(ind);

    double err(0.0);
    int illegals(0);
    // double fit(0.0);

    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      load_vars(*t);

      const boost::any res(agent());

      if (res.empty())
        err += std::pow(100.0, ++illegals);
      else
        err += std::fabs(boost::any_cast<double>(res) -
                         boost::any_cast<double>(t->output));
    }

    return fitness_t(-err);
    // return fitness_t(1000.0*std::exp(-err));
    // return fitness_t(fit);
  }

  ///
  /// \param[in] ind program used for fitness evaluation.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// This function is similar to operator()() but will skip 3 out of 4
  /// training instances, so it's faster ;-) but...
  /// \attention output value of this method and of the operator()() method
  /// cannot be compared (I know, it's a pity).
  ///
  fitness_t abs_evaluator::fast(const individual &ind)
  {
    assert(!dat_->classes());

    interpreter agent(ind);

    double err(0.0);
    int illegals(0);
    unsigned counter(0);

    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
      if (dat_->size() <= 20 || (counter++ % 5) == 0)
      {
        load_vars(*t);

        const boost::any res(agent());

        if (res.empty())
          err += std::pow(100.0, ++illegals);
        else
          err += std::fabs(boost::any_cast<double>(res) -
                           boost::any_cast<double>(t->output));
      }

    return fitness_t(-err);
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

    const unsigned n_slots(eva_->dat_->classes() * 10);
    std::vector <std::vector<unsigned>> slot_matrix(n_slots);
    std::vector<unsigned> slot_class(n_slots);
    eva_->fill_slots(ind, &slot_matrix, &slot_class);

    for (unsigned i(0); i < slot_class.size(); ++i)
      slot_class_.push_back(eva_->dat_->class_name(slot_class[i]));
  }

  ///
  /// \param[in] instance data to be classified.
  /// \return the class that include the \a instance.
  ///
  std::string dyn_slot_classifier::operator()(
    const data::value_type &instance) const
  {
    eva_->load_vars(instance);

    const boost::any res( (interpreter(ind_))() );

    const unsigned n_slots(slot_class_.size());
    unsigned where(n_slots - 1);

    if (!res.empty())
    {
      const double val(boost::any_cast<double>(res));

      where = static_cast<unsigned>(dyn_slot_evaluator::normalize_01(val) *
                                    n_slots);
      if (where >= n_slots)
        where = n_slots - 1;
    }

    return slot_class_[where];
  }

  ///
  /// \param[in] val the numeric value that should be mapped in the [0,1]
  ///                interval.
  ///
  inline
  double dyn_slot_evaluator::normalize_01(double val)
  {
    return 0.5 + std::atan(val)/3.1415926535;
    // return 1.0 / (1 + std::exp(-val));
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[in] t input data for \a ind.
  /// \param[in] n_slots number of slots used for Slotted Dynamic Class
  ///                    Boundary Determination.
  /// \return in which slot the input data must be put.
  ///
  unsigned dyn_slot_evaluator::slot(const individual &ind,
                                    data::const_iterator t,
                                    unsigned n_slots)
  {
    assert(ind.check());
    assert(n_slots);

    load_vars(*t);

    interpreter agent(ind);
    const boost::any res(agent());

    if (res.empty())
      return n_slots - 1;


    const double val(boost::any_cast<double>(res));
    const unsigned where(static_cast<unsigned>(normalize_01(val) * n_slots));

    return (where >= n_slots) ? n_slots - 1 : where;
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[out] slot_matrix the main matrix of the dynamic slot algorithm.
  /// \param[out] slot_class slot_class[i] = "label of the predominant class"
  ///                        for the i-th slot.
  ///
  void dyn_slot_evaluator::fill_slots(
    const individual &ind,
    std::vector <std::vector<unsigned>> *slot_matrix,
    std::vector<unsigned> *slot_class
    )
  {
    assert(ind.check());
    assert(slot_matrix);
    assert(slot_class);

    const unsigned n_slots(slot_matrix->size());
    for (unsigned i(0); i < n_slots; ++i)
    {
      (*slot_matrix)[i].resize(dat_->classes());

      for (unsigned j(0); j < (*slot_matrix)[i].size(); ++j)
        (*slot_matrix)[i][j] = 0;
    }

    assert(ind.check());

    interpreter agent(ind);

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slots[slot][class]).
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      const unsigned where(slot(ind, t, n_slots));

      ++((*slot_matrix)[where][t->label()]);
    }

    const unsigned unknown(dat_->classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (unsigned i(0); i < n_slots; ++i)
    {
      unsigned best_class(0);

      for (unsigned j(1); j < (*slot_matrix)[i].size(); ++j)
        if ((*slot_matrix)[i][j] >= (*slot_matrix)[i][best_class])
          best_class = j;

      (*slot_class)[i] = (*slot_matrix)[i][best_class] ? best_class : unknown;
    }
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// Slotted Dynamic Class Boundary Determination
  ///
  fitness_t dyn_slot_evaluator::operator()(const individual &ind)
  {
    assert(ind.check());
    assert(dat_->classes() >= 2);

    const unsigned n_slots(dat_->classes()*10);

    std::vector<std::vector<unsigned>> slot_matrix(n_slots);
    std::vector<unsigned> slot_class(n_slots);
    fill_slots(ind, &slot_matrix, &slot_class);

    fitness_t err(0.0);
    for (unsigned i(0); i < n_slots; ++i)
      for (unsigned j(0); j < slot_matrix[i].size(); ++j)
        if (j != slot_class[i])
          err += slot_matrix[i][j];

    return fitness_t(-err);
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the accuracy (between 0.0 and 1.0 if there are available data,
  ///         -1.0 is the dataset is empty).
  ///
  double dyn_slot_evaluator::accuracy(const individual &ind)
  {
    unsigned count(0);
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
      ++count;

    const unsigned ok(count + operator()(ind));

    return count ? double(ok) / double(count) : -1.0;
  }

  ///
  /// \param[in] ind individual used for classification.
  /// \param[out] gauss
  ///
  void gaussian_evaluator::gaussian_distribution(
    const individual &ind,
    std::vector<distribution<double>> *gauss)
  {
    assert(dat_->classes() == gauss->size());

    assert(ind.check());
    interpreter agent(ind);

    // For a set of training data, we assume that the behaviour of a program
    // classifier is modelled using multiple Gaussian distributions, each of
    // which corresponds to a particular class. The distribution of a class is
    // determined by evaluating the program on the examples of the class in
    // the training set. This is done by taking the mean and standard deviation
    // of the program outputs for those training examples for that class.
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      load_vars(*t);

      const boost::any res(agent());

      const double cut(1000);
      double val(res.empty() ? 0.0 : boost::any_cast<double>(res));
      if (val > cut)
        val = cut;
      else if (val < -cut)
        val = -cut;

      (*gauss)[t->label()].add(val);
    }
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
  fitness_t gaussian_evaluator::operator()(const individual &ind)
  {
    assert(dat_->classes() >= 2);
    std::vector< distribution<double> > gauss(dat_->classes());
    gaussian_distribution(ind, &gauss);

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

    return d;
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the accuracy (between 0.0 and 1.0 if there are available data,
  ///         -1.0 is the dataset is empty).
  ///
  double gaussian_evaluator::accuracy(const individual &ind)
  {
    assert(dat_->classes() >= 2);
    std::vector<distribution<double>> gauss(dat_->classes());
    gaussian_distribution(ind, &gauss);

    unsigned ok(0), count(0);
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++count, ++t)
      if (class_label(ind, *t, gauss) == t->label())
        ++ok;

    return count ? double(ok) / double(count) : -1.0;
  }

  ///
  /// \param[in] ind program used for classification.
  /// \param[in] instance input value whose class we are interested in.
  /// \param[in] gauss parameters of the gaussian distributions.
  /// \return the class of \a instance.
  ///
  unsigned gaussian_evaluator::class_label(
    const individual &ind,
    const data::value_type &instance,
    const std::vector<distribution<double>> &gauss)
  {
    load_vars(instance);

    const boost::any res( (interpreter(ind))() );
    const double x(res.empty() ? 0.0 : boost::any_cast<double>(res));

    assert(dat_->classes() == gauss.size());

    double min_distance(std::fabs(x - gauss[0].mean));
    unsigned probable_class(0);
    for (unsigned i(0); i < dat_->classes(); ++i)
    {
      const double m(gauss[i].mean);
      const double distance(std::fabs(x - m));
      if (distance < min_distance)
      {
        min_distance = distance;
        probable_class = i;
      }
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

    eva_->gaussian_distribution(ind, &gauss_);
  }

  ///
  ///
  /// \param[in] instance data to be classified.
  /// \return the class that include the \a instance.
  ///
  std::string gaussian_classifier::operator()(
    const data::value_type &instance) const
  {
    return eva_->dat_->class_name(eva_->class_label(ind_, instance, gauss_));
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

  /*
  ///
  /// \param[in] ind an individual.
  /// \return the fitness of individual \a ind (greater is better, max is 0).
  ///
  template<class T>
  fitness_t
  problem::count_fitness(const vita::individual &ind) const
  {
    assert(!dat.classes());

    interpreter agent(ind);

    unsigned ok(0);

    for (data::const_iterator t(dat.begin()); t != dat.end(); ++t)
    {
      for (unsigned i(0); i < vars_.size(); ++i)
        vars[i]->val = t->input[i];

      const boost::any res(agent());

      if (!res.empty() && boost::any_cast<T>(res) == t->output)
        ++ok;
    }

    return fitness_t(ok);
  }
  */
}  // namespace vita
