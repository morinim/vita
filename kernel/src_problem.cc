/**
 *
 *  \file problem.cc
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

#include "kernel/src_problem.h"

namespace vita
{
  class src_evaluator : public evaluator
  {
  public:
    src_evaluator(data *d, std::vector<vita::sr::variable *> *v)
      : dat_(d), var_(v) {}

  protected:
    data *dat_;
    std::vector<vita::sr::variable *> *var_;
  };

  class abs_evaluator : public src_evaluator
  {
  public:
    abs_evaluator(data *d, std::vector<vita::sr::variable *> *v)
      : src_evaluator(d, v) {}

    fitness_t operator()(const individual &);
  };

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
      for (unsigned i(0); i < var_->size(); ++i)
        (*var_)[i]->val = boost::any_cast<double>(t->input[i]);

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

  class dyn_slot_evaluator : public src_evaluator
  {
  public:
    dyn_slot_evaluator(data *d, std::vector<vita::sr::variable *> *v)
      : src_evaluator(d, v)
    {
      assert(d);
      assert(v);
    }

    fitness_t operator()(const individual &);

  private:
    static double normalize_01(double);
  };

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
  /// \param[in] ind program used for class recognition.
  /// \return the fitness (greater is better, max is 0).
  ///
  /// Slotted Dynamic Class Boundary Determination
  ///
  fitness_t dyn_slot_evaluator::operator()(const individual &ind)
  {
    assert(dat_->classes() >= 2);

    const unsigned n_slots(dat_->classes()*10);
    std::vector< std::vector<unsigned> > slots(n_slots);
    for (unsigned i(0); i < slots.size(); ++i)
    {
      slots[i].resize(dat_->classes());

      for (unsigned j(0); j < slots[i].size(); ++j)
        slots[i][j] = 0;
    }

    interpreter agent(ind);

    // In the first step this method evaluates the program to obtain an output
    // value for each training example. Based on the program output value a
    // a bidimentional array is built (slots[slot][class]).
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      for (unsigned i(0); i < var_->size(); ++i)
        (*var_)[i]->val = boost::any_cast<double>(t->input[i]);

      const boost::any res(agent());

      unsigned slot;
      if (res.empty())
        slot = slots.size()-1;
      else
      {
        const double val(boost::any_cast<double>(res));
        slot = static_cast<unsigned>(normalize_01(val) * n_slots);
        if (slot >= slots.size())
          slot = slots.size()-1;
      }

      ++slots[slot][t->label()];
    }

    std::vector<unsigned> slot_label(n_slots);
    const unsigned unknown(dat_->classes());

    // In the second step the method dynamically determine to which class each
    // slot belongs by simply taking the class with the largest value at the
    // slot...
    for (unsigned i(0); i < n_slots; ++i)
    {
      unsigned best_class(0);

      for (unsigned j(1); j < slots[i].size(); ++j)
        if (slots[i][j] >= slots[i][best_class])
          best_class = j;

      slot_label[i] = slots[i][best_class] ? best_class : unknown;
    }

    fitness_t err(0.0);
    for (unsigned i(0); i < n_slots; ++i)
      for (unsigned j(0); j < slots[i].size(); ++j)
        if (j != slot_label[i])
          err += slots[i][j];

    return fitness_t(-err);
  }

  class gaussian_evaluator : public src_evaluator
  {
  public:
    gaussian_evaluator(data *d, std::vector<vita::sr::variable *> *v)
      : src_evaluator(d, v), gauss_(d->classes())
    {
      assert(d);
      assert(v);
      assert(d->classes() >= 2);
    }

    unsigned class_label(const individual &, const data::value_type &);
    double success_rate() const;

    fitness_t operator()(const individual &);

  private:
    std::vector< distribution<double> > gauss_;
  };

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
    interpreter agent(ind);

    // For a set of training data, we assume that the behaviour of a program
    // classifier is modelled using multiple Gaussian distributions, each of
    // which corresponds to a particular class. The distribution of a class is
    // determined by evaluating the program on the examples of the class in
    // the training set. This is done by taking the mean and standard deviation
    // of the program outputs for those training examples for that class.
    for (data::const_iterator t(dat_->begin()); t != dat_->end(); ++t)
    {
      for (unsigned i(0); i < var_->size(); ++i)
        (*var_)[i]->val = boost::any_cast<double>(t->input[i]);

      const boost::any res(agent());
      const double val(res.empty() ? 0.0 : boost::any_cast<double>(res));

      gauss_[t->label()].add(val);
    }

    fitness_t d(0.0);
    for (unsigned i(0); i < gauss_.size(); ++i)
      for (unsigned j(i+1); j < gauss_.size(); ++j)
      {
        const double mean_i(gauss_[i].mean);
        const double mean_j(gauss_[j].mean);
        const double stddev_i(gauss_[i].standard_deviation());
        const double stddev_j(gauss_[j].standard_deviation());

        d += std::fabs(mean_j - mean_i) / (stddev_j + stddev_i);
      }

    return -1.0 / (1.0 + d);
  }

  ///
  /// \param[in] ind
  /// \param[in] val input value whose class we are interested in.
  /// \return the class of \a val.
  ///
  unsigned gaussian_evaluator::class_label(const individual &ind,
                                           const data::value_type &val)
  {
    for (unsigned i(0); i < var_->size(); ++i)
      (*var_)[i]->val = boost::any_cast<double>(val.input[i]);

    const boost::any res( (interpreter(ind))() );
    const double x(res.empty() ? 0.0 : boost::any_cast<double>(res));
    const double pi2(2*std::acos(-1.0));

    assert(dat_->classes() == gauss_->size());

    double max_probability(0.0);
    unsigned probable_class(0);
    for (unsigned i(0); i < dat_->classes(); ++i)
    {
      const double m(gauss_[i].mean);
      const double s(gauss_[i].variance);

      const double p(std::exp((x-m)*(m-x) / (2*s*s)) / std::sqrt(pi2*s*s));

      if (p > max_probability)
      {
        max_probability = p;
        probable_class = i;
      }
    }

    return probable_class;
  }

  ///
  /// New empty instance.
  ///
  src_problem::src_problem()
  {
    clear();

    add_evaluator(new abs_evaluator(&dat_, &vars_));
    add_evaluator(new gaussian_evaluator(&dat_, &vars_));
    //add_evaluator(new dyn_slot_evaluator(&dat_, &vars_));
  }

  ///
  /// Class destructor frees memory allocated by src_problem constructor.
  ///
  src_problem::~src_problem()
  {
    delete_evaluators();
    env.sset.delete_symbols();
  }

  ///
  /// Resets the object.
  ///
  void src_problem::clear()
  {
    problem::clear();
    vars_.clear();
    dat_.clear();
  }

  ///
  /// \param[in] f name of the file containing the learning collection.
  /// \return false if the file cannot be read.
  ///
  bool src_problem::load_data(const std::string &f)
  {
    dat_.clear();

    if (dat_.open(f) > 1)
    {
      // Sets up the variables.
      for (unsigned i(0); i < dat_.variables(); ++i)
      {
        std::ostringstream s;
        s << 'X' << i;
        const std::string str(s.str());

        vita::sr::variable *const x = new vita::sr::variable(str);
        vars_.push_back(x);
        env.insert(x);
      }

      set_evaluator(classes() > 1
        ? 1     // Symbolic regression problem
        : 0);   // Classification problem

      return true;
    }

    return false;
  }

  ///
  /// \param[in] sf name of the file containing the symbols.
  /// \return a space separated string containing the names of the loaded
  ///         symbols.
  ///
  std::string src_problem::load_symbols(const std::string &sf)
  {
    std::string symbols;

    // Set up the symbols (variables have already been prepared).
    if (!sf.empty())  // Default functions.
    {
      std::ifstream from(sf.c_str());
      if (!from)
        return "";

      std::string name;
      while (from >> name)
      {
        symbols += name+" ";

        std::stringstream s;
        s << name;
        double n;
        if (s >> n)
          env.insert(new vita::sr::constant(n));
        else if (name == "number")
          env.insert(new vita::sr::number(-128, 127));
        else if (name == "abs")
          env.insert(new vita::sr::abs());
        else if (name == "add" || name == "+")
          env.insert(new vita::sr::add());
        // else if (name=="and" || name=="&&")
        //   env.insert(new vita::sr::bool_and());
        // else if (name == "or" || name == "||")
        //   env.insert(new vita::sr::bool_not());
        // else if (name == "not" || name == "!")
        //   env.insert(new vita::sr::bool_or());
        else if (name == "div" || name == "/")
          env.insert(new vita::sr::div());
        else if (name == "idiv")
          env.insert(new vita::sr::idiv());
        else if (name == "ife")
          env.insert(new vita::sr::ife());
        else if (name == "ifl")
          env.insert(new vita::sr::ifl());
        else if (name == "ifz")
          env.insert(new vita::sr::ifz());
        else if (name == "ln")
          env.insert(new vita::sr::ln());
        else if (name == "mul" || name == "*")
          env.insert(new vita::sr::mul());
        else if (name == "mod" || name == "%")
          env.insert(new vita::sr::mod());
        else if (name == "sub" || name == "-")
          env.insert(new vita::sr::sub());
      }
    }

    return symbols;
  }

  ///
  /// \return number of classes of the classification problem (1 for a symbolic
  ///         regression problem).
  ///
  unsigned src_problem::classes() const
  {
    return dat_.classes();
  }

  ///
  /// \return dimension of the input vectors (i.e. the number of variable of
  ///         the problem).
  ///
  unsigned src_problem::variables() const
  {
    return dat_.variables();
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool src_problem::check() const
  {
    return problem::check() && dat_.check() && vars_.size() == dat_.variables();
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
