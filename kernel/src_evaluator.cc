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
#include "kernel/data.h"
#include "kernel/individual.h"

namespace vita
{
  ///
  /// \param[in] d
  /// \param[in] v
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
      (*var_)[i]->val = boost::any_cast<double>(d.input[i]);
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
  /// \param[in] val the numeric value that should be mapped in the [0,1]
  ///                interval.
  ///
  inline
  double dyn_slot_evaluator::normalize_01(double val)
  {
    return 0.5 + std::atan(val)/3.1415926535;
  }

  ///
  /// \param[in] ind
  /// \param[out] slots
  ///
  void dyn_slot_evaluator::fill_slots(
    const individual &ind,
    std::vector <std::vector<unsigned>> *slots,
    std::vector<unsigned> *slot_label
    )
  {
    const unsigned n_slots(slots->size());
    for (unsigned i(0); i < n_slots; ++i)
    {
      (*slots)[i].resize(dat_->classes());

      for (unsigned j(0); j < (*slots)[i].size(); ++j)
        (*slots)[i][j] = 0;
    }

    assert(ind.check());
    interpreter agent(ind);

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slots[slot][class]).
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      load_vars(*t);

      const boost::any res(agent());

      unsigned slot;
      if (res.empty())
        slot = slots->size()-1;
      else
      {
        const double val(boost::any_cast<double>(res));
        slot = static_cast<unsigned>(normalize_01(val) * n_slots);
        if (slot >= slots->size())
          slot = slots->size()-1;
      }

      ++((*slots)[slot][t->label()]);
    }

    const unsigned unknown(dat_->classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (unsigned i(0); i < n_slots; ++i)
    {
      unsigned best_class(0);

      for (unsigned j(1); j < (*slots)[i].size(); ++j)
        if ((*slots)[i][j] >= (*slots)[i][best_class])
          best_class = j;

      (*slot_label)[i] = (*slots)[i][best_class] ? best_class : unknown;
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
    std::vector<std::vector<unsigned>> slots(n_slots);
    std::vector<unsigned> slot_label(n_slots);
    fill_slots(ind, &slots, &slot_label);

    fitness_t err(0.0);
    for (unsigned i(0); i < n_slots; ++i)
      for (unsigned j(0); j < slots[i].size(); ++j)
        if (j != slot_label[i])
          err += slots[i][j];

    return fitness_t(-err);
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the success rate (between 0.0 and 1.0).
  ///
  double dyn_slot_evaluator::success_rate(const individual &ind)
  {
    unsigned count(0);
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
      ++count;

    const unsigned ok(count + operator()(ind));

    return count ? double(ok) / double(count) : 0.0;
  }

  void gaussian_evaluator::gaussian_distribution(
    const individual &ind,
    std::vector< distribution<double> > *gauss)
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

        d += 200.0*std::log(delta) - radius;
      }

    return d;
  }

  ///
  /// \param[in] ind program used for class recognition.
  /// \return the success rate (between 0.0 and 1.0).
  ///
  double gaussian_evaluator::success_rate(const individual &ind)
  {
    assert(dat_->classes() >= 2);
    std::vector< distribution<double> > gauss(dat_->classes());
    gaussian_distribution(ind, &gauss);

    for (unsigned i(0); i < gauss.size(); ++i)
      std::cout << "Class " << i << " mean: " << gauss[i].mean << " variance: "
                << gauss[i].variance << std::endl;

    unsigned ok(0), count(0);
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++count, ++t)
      if (class_label(ind, *t, gauss) == t->label())
        ++ok;

    return count ? double(ok) / double(count) : 0.0;
  }

  ///
  /// \param[in] ind
  /// \param[in] val input value whose class we are interested in.
  /// \param[in] gauss parameters of the gaussian distributions.
  /// \return the class of \a val.
  ///
  unsigned gaussian_evaluator::class_label(
    const individual &ind,
    const data::value_type &val,
    const std::vector< distribution<double> > &gauss)
  {
    load_vars(val);

    const boost::any res( (interpreter(ind))() );
    const double x(res.empty() ? 0.0 : boost::any_cast<double>(res));
    //const double pi2(2*std::acos(-1.0));

    assert(dat_->classes() == gauss.size());

    //double max_probability(0.0);
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
