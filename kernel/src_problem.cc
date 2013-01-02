/**
 *
 *  \file src_problem.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "src_problem.h"
#include "lambda_f.h"
#include "src_constant.h"
#include "src_evaluator.h"
#include "src_variable.h"

namespace vita
{
  ///
  /// New empty instance.
  ///
  src_problem::src_problem()
  {
    clear();
  }

  ///
  /// Resets the object.
  ///
  void src_problem::clear()
  {
    p_symre = k_sae_evaluator;
    p_class = k_gaussian_evaluator;

    problem::clear();
    dat_.clear();
  }

  ///
  /// \param[in] id numerical id of the evaluator to be activated
  /// \param[in] msg input parameters for the evaluator constructor.
  ///
  void src_problem::set_evaluator(evaluator_id id, const std::string &msg)
  {
    switch (id)
    {
    case k_count_evaluator:
      problem::set_evaluator(std::make_shared<count_evaluator>(dat_));
      break;

    case k_sae_evaluator:
      problem::set_evaluator(std::make_shared<sae_evaluator>(dat_));
      break;

    case k_sse_evaluator:
      problem::set_evaluator(std::make_shared<sse_evaluator>(dat_));
      break;

    case k_dyn_slot_evaluator:
    {
      const size_t x_slot(msg.empty() ? 10 : boost::lexical_cast<size_t>(msg));
      problem::set_evaluator(std::make_shared<dyn_slot_evaluator>(dat_,
                                                                  x_slot));
      break;
    }

    case k_gaussian_evaluator:
      problem::set_evaluator(std::make_shared<gaussian_evaluator>(dat_));
      break;
    }
  }

  ///
  /// \param[in] ds name of the dataset file (training/validation set).
  /// \param[in] ts name of the test set.
  /// \param[in] symbols name of the file containing the symbols. If it is
  ///                    empty, \c src_problem::setup_default_symbols is called.
  /// \return number of examples (lines) parsed and number of sumbols parsed.
  ///
  /// Loads \a data into the active dataset.
  ///
  std::pair<size_t, size_t> src_problem::load(const std::string &ds,
                                              const std::string &ts,
                                              const std::string &symbols)
  {
    env.sset = vita::symbol_set();
    dat_.clear();

    const size_t n_examples(dat_.open(ds));
    if (n_examples > 0)
      set_evaluator(classification() ? p_class : p_symre);

    if (!ds.empty())
      load_test_set(ts);

    size_t n_symbols(0);
    if (symbols.empty())
      setup_default_symbols();
    else
      n_symbols = load_symbols(symbols);

    return std::pair<size_t, size_t>(n_examples, n_symbols);
  }

  ///
  /// \param[in] ts name of the file containing the test set.
  /// \return number of examples parsed.
  ///
  /// Load the test set.
  ///
  size_t src_problem::load_test_set(const std::string &ts)
  {
    const data::dataset_t backup(dat_.dataset());
    dat_.dataset(data::test);

    const size_t n(dat_.open(ts));

    dat_.dataset(backup);
    return n;
  }

  ///
  /// Inserts into the symbol_set variables and labels for nominal
  /// attributes.
  ///
  void src_problem::setup_terminals_from_data()
  {
    env.sset = vita::symbol_set();

    // Sets up the variables (features).
    for (size_t i(1); i < dat_.columns(); ++i)
    {
      std::string name(dat_.get_column(i).name);
      if (name.empty())
        name = "X" + boost::lexical_cast<std::string>(i);

      const category_t category(dat_.get_column(i).category_id);
      env.insert(std::make_shared<variable>(name, i - 1, category));
    }

    // Sets up the labels for nominal attributes.
    for (category_t c(0); c < dat_.categories(); ++c)
    {
      const data::category &cat(dat_.get_category(c));
      for (auto lp(cat.labels.begin()); lp != cat.labels.end(); ++lp)
      {
        const symbol_ptr label(std::make_shared<constant>(*lp, c));
        env.insert(label);
      }
    }
  }

  ///
  /// Default symbol set. This is useful for simple problems (single category
  /// regression / classification).
  ///
  void src_problem::setup_default_symbols()
  {
    setup_terminals_from_data();

    symbol_factory &factory(symbol_factory::instance());

    for (category_t category(0); category < dat_.categories(); ++category)
      if (compatible({category}, {"numeric"}))
      {
        env.insert(factory.make("1.0", {category}));
        env.insert(factory.make("2.0", {category}));
        env.insert(factory.make("3.0", {category}));
        env.insert(factory.make("4.0", {category}));
        env.insert(factory.make("5.0", {category}));
        env.insert(factory.make("6.0", {category}));
        env.insert(factory.make("7.0", {category}));
        env.insert(factory.make("8.0", {category}));
        env.insert(factory.make("9.0", {category}));
        env.insert(factory.make("FABS", {category}));
        env.insert(factory.make("FADD", {category}));
        env.insert(factory.make("FDIV", {category}));
        env.insert(factory.make("FLN",  {category}));
        env.insert(factory.make("FMUL", {category}));
        env.insert(factory.make("FMOD", {category}));
        env.insert(factory.make("FSUB", {category}));
      }
  }

  ///
  /// \param[in] sf name of the file containing the symbols.
  /// \return number of parsed symbols.
  ///
  /// Data should be loaded before symbols: if we haven't data we don't know,
  /// among other things, how many features the dataset has.
  /// This function is used to change the symbols mantaining the same dataset.
  ///
  size_t src_problem::load_symbols(const std::string &sf)
  {
    setup_terminals_from_data();

    size_t parsed(0);

    cvect categories(dat_.categories());
    for (size_t i(0); i < categories.size(); ++i)
      categories[i] = i;

    // Load the XML file (sf) into the property tree (pt).
    using namespace boost::property_tree;
    ptree pt;
    read_xml(sf, pt);

#if !defined(NDEBUG)
    std::cout << std::endl << std::endl;
    for (size_t i(0); i < dat_.categories(); ++i)
      std::cout << "Category " << i << ": " << dat_.get_category(i).name
                << " (domain " << dat_.get_category(i).domain << ")"
                << std::endl;
    std::cout << std::endl;
#endif

    symbol_factory &factory(symbol_factory::instance());

    for (ptree::value_type s : pt.get_child("symbolset"))
      if (s.first == "symbol")
      {
        const std::string sym_name(s.second.get<std::string>("<xmlattr>.name"));
        const std::string sym_sig(s.second.get<std::string>(
                                    "<xmlattr>.signature", ""));

        if (sym_sig.empty())
        {
          for (ptree::value_type sig : s.second)
            if (sig.first == "signature")
            {
              std::vector<std::string> args;
              for (ptree::value_type arg : sig.second)
                if (arg.first == "arg")
                  args.push_back(arg.second.data());

              // From the list of all the sequences with repetition of
              // args.size() elements (categories)...
              const std::list<cvect> sequences(seq_with_rep(categories,
                                                            args.size()));

              // ...we choose those compatible with the xml signature of the
              // current symbol.
              for (auto i(sequences.begin()); i != sequences.end(); ++i)
                if (compatible(*i, args))
                {
#if !defined(NDEBUG)
                  //const domain_t domain(dat_.get_category(i->back()).domain);
                  std::cout << sym_name << '(';
                  for (size_t j(0); j < i->size(); ++j)
                    std::cout << dat_.get_category((*i)[j]).name
                              << (j+1 == i->size() ? ")" : ", ");
                  std::cout << std::endl;
#endif
                  env.insert(factory.make(sym_name, *i));
                }
            }
        }
        else  // !sym_sig.empty() => one category, uniform symbol initialization
        {
          for (category_t category(0); category < dat_.categories(); ++category)
            if (compatible({category}, {sym_sig}))
            {
              const size_t n_args(factory.args(sym_name));

#if !defined(NDEBUG)
              std::cout << sym_name << '(';
              for (size_t j(0); j < n_args; ++j)
                std::cout << dat_.get_category(category).name
                          << (j + 1 == n_args ? ")" : ", ");
              std::cout << std::endl;
#endif
              env.insert(factory.make(sym_name, cvect(n_args, category)));
            }
        }

        ++parsed;
      }

    return parsed;
  }

  ///
  /// \param[in] instance a vector of categories.
  /// \param[in] pattern a mixed vector of category names and domain names.
  /// \return \c true if \a instance match \a pattern.
  ///
  /// For instance:
  ///
  ///     category_t km_h, name;
  ///     compatible({km_h}, {"km/h"}) == true
  ///     compatible({km_h}, {"numeric"}) == true
  ///     compatible({km_h}, {"string"}) == false
  ///     compatible({km_h}, {"name"}) == false
  ///     compatible({name}, {"string"}) == true
  ///
  bool src_problem::compatible(const cvect &instance,
                               const std::vector<std::string> &pattern) const
  {
    assert(instance.size() == pattern.size());

    for (size_t i(0); i < instance.size(); ++i)
    {
      bool generic(data::from_weka.find(pattern[i]) != data::from_weka.end());

      if (generic)  // numeric, string, integer...
      {
        if (dat_.get_category(instance[i]).domain !=
            data::from_weka.find(pattern[i])->second)
          return false;
      }
      else
      {
        if (instance[i] != dat_.get_category(pattern[i]))
          return false;
      }
    }

    return true;
  }

  ///
  /// \param[in] categories this is the "dictionary" for the sequence.
  /// \param[in] args size of the sequence.
  /// \return a list of sequences with repetition of fixed length (\a args) of
  ///         elements taken from the given set (\a categories).
  ///
  std::list<src_problem::cvect> src_problem::seq_with_rep(
    const cvect &categories,
    size_t args)
  {
    assert(categories.size());
    assert(args);

    class swr
    {
    public:
      swr(const cvect &categories, size_t args)
        : categories_(categories), args_(args)
      {
      }

      void operator()(unsigned level, const cvect &base, std::list<cvect> *out)
      {
        for (size_t i(0); i < categories_.size(); ++i)
        {
          cvect current(base);
          current.push_back(categories_[i]);

          if (level + 1 < args_)
            operator()(level + 1, current, out);
          else
            out->push_back(current);
        }
      }

    private:
      const cvect &categories_;
      const size_t args_;
    };

    std::list<cvect> out;
    swr(categories, args)(0, {}, &out);
    return out;
  }

  ///
  /// \return number of categories of the problem (>= 1).
  ///
  size_t src_problem::categories() const
  {
    return dat_.categories();
  }

  ///
  /// \return number of classes of the problem (== 0 for a symbolic regression
  ///         problem, > 1 for a classification problem).
  ///
  size_t src_problem::classes() const
  {
    assert(dat_.classes() != 1);

    return dat_.classes();
  }

  ///
  /// \return dimension of the input vectors (i.e. the number of variable of
  ///         the problem).
  ///
  size_t src_problem::variables() const
  {
    return dat_.variables();
  }

  ///
  /// \param[in] ind individual to be transformed in a lambda function.
  /// \return the lambda function associated with \a ind (\c nullptr in case of
  ///         errors).
  ///
  /// The lambda function depends on the active evaluator.
  ///
  std::unique_ptr<lambda_f> src_problem::lambdify(const individual &ind)
  {
    return active_eva_->lambdify(ind);

/*
    switch (i)
    {
    case k_count_evaluator:
    case k_sae_evaluator:
    case k_sse_evaluator:
      return std::unique_ptr<lambda_f>(new lambda_f(ind));

    case k_dyn_slot_evaluator:
      return std::unique_ptr<lambda_f>(new dyn_slot_lambda_f(ind, dat_));

    case k_gaussian_evaluator:
      return std::unique_ptr<lambda_f>(new gaussian_lambda_f(ind, dat_));
    }

    return nullptr;
*/
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool src_problem::check(bool verbose) const
  {
    if (!problem::check(verbose))
      return false;

    if (!dat_.check())
      return false;

    if (p_symre > k_max_evaluator)
    {
      if (verbose)
        std::cerr << "Incorrect ID for preferred sym.reg. evaluator."
                  << std::endl;
      return false;
    }

    if (p_class > k_max_evaluator)
    {
      if (verbose)
        std::cerr << "Incorrect ID for preferred classification evaluator."
                  << std::endl;
      return false;
    }

    return true;
  }
}  // namespace vita
