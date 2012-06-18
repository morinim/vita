/**
 *
 *  \file src_problem.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011, 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "kernel/src_problem.h"
#include "kernel/individual.h"
#include "kernel/src_evaluator.h"
#include "kernel/primitive/factory.h"

namespace vita
{
  ///
  /// \param[in] st success threashold: when fitness is greater than this value,
  ///               the datum is considered learned (matched, classified,
  ///               resolved...)
  ///
  /// New empty instance.
  ///
  src_problem::src_problem(fitness_t st) : problem(st)
  {
    clear();

    unsigned i;

    i = add_evaluator(std::make_shared<count_evaluator>(&dat_, &vars_));
    assert(i == k_count_evaluator);

    i = add_evaluator(std::make_shared<sae_evaluator>(&dat_, &vars_));
    assert(i == k_sae_evaluator);

    i = add_evaluator(std::make_shared<sse_evaluator>(&dat_, &vars_));
    assert(i == k_sse_evaluator);

    i = add_evaluator(std::make_shared<dyn_slot_evaluator>(&dat_, &vars_));
    assert(i == k_dyn_slot_evaluator);

    //add_evaluator(std::make_shared<gaussian_evaluator>(&dat_, &vars_));
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
  /// \param[in] data name of the file containing the learning collection.
  /// \param[in] symbols name of the file containing the symbols. If it is
  ///                    empty, \c src_problem::setup_default_symbols is called.
  /// \return number of examples (lines) parsed and number of sumbols parsed.
  ///
  std::pair<unsigned, unsigned> src_problem::load(const std::string &data,
                                                  const std::string &symbols)
  {
    env.sset = vita::symbol_set();
    vars_.clear();
    dat_.clear();

    const unsigned n_examples(dat_.open(data));
    if (n_examples > 0)
      set_evaluator(classes() > 1
        ? k_dyn_slot_evaluator   // classification problem
        : k_sse_evaluator);      // symbolic regression problem

    unsigned n_symbols(0);
    if (symbols.empty())
      setup_default_symbols();
    else
      n_symbols = load_symbols(symbols);

    return std::pair<unsigned, unsigned>(n_examples, n_symbols);
  }

  ///
  /// Inserts into the symbol_set variables and labels for nominal
  /// attributes.
  ///
  void src_problem::setup_terminals_from_data()
  {
    env.sset = vita::symbol_set();
    vars_.clear();

    // Sets up the variables (features).
    for (unsigned i(1); i < dat_.columns(); ++i)
    {
      std::string name(dat_.get_column(i).name);
      if (name.empty())
        name = "X" + boost::lexical_cast<std::string>(i);

      const category_t category(dat_.get_column(i).category_id);
      const variable_ptr x(std::make_shared<variable>(name, category));
      vars_.push_back(x);
      env.insert(x);
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
  unsigned src_problem::load_symbols(const std::string &sf)
  {
    setup_terminals_from_data();

    unsigned parsed(0);

    cvect categories(dat_.categories());
    for (unsigned i(0); i < categories.size(); ++i)
      categories[i] = i;

    // Load the XML file (sf) into the property tree (pt).
    using namespace boost::property_tree;
    ptree pt;
    read_xml(sf, pt);

#if !defined(NDEBUG)
    std::cout << std::endl << std::endl;
    for (unsigned i(0); i < dat_.categories(); ++i)
      std::cout << "Category " << i << ": " << dat_.get_category(i).name
                << " (domain " << dat_.get_category(i).domain << ")"
                << std::endl;
    std::cout << std::endl;
#endif

    symbol_factory &factory(symbol_factory::instance());

    BOOST_FOREACH(ptree::value_type s, pt.get_child("symbolset"))
      if (s.first == "symbol")
      {
        const std::string sym_name(s.second.get<std::string>("<xmlattr>.name"));
        const std::string sym_sig(s.second.get<std::string>(
                                    "<xmlattr>.signature", ""));

        if (sym_sig.empty())
        {
          BOOST_FOREACH(ptree::value_type sig, s.second)
            if (sig.first == "signature")
            {
              std::vector<std::string> args;
              BOOST_FOREACH(ptree::value_type arg, sig.second)
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
                  for (unsigned j(0); j < i->size(); ++j)
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
              const unsigned n_args(factory.args(sym_name));

#if !defined(NDEBUG)
              std::cout << sym_name << '(';
              for (unsigned j(0); j < n_args; ++j)
                std::cout << dat_.get_category(category).name
                          << (j+1 == n_args ? ")" : ", ");
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
  /// \verbatim
  /// category_t km_h, name;
  /// compatible({km_h}, {"km/h"}) == true
  /// compatible({km_h}, {"numeric"}) == true
  /// compatible({km_h}, {"string"}) == false
  /// compatible({km_h}, {"name"}) == false
  /// compatible({name}, {"string"}) == true
  /// \endverbatim
  ///
  bool src_problem::compatible(const cvect &instance,
                               const std::vector<std::string> &pattern) const
  {
    assert(instance.size() == pattern.size());

    for (unsigned i(0); i < instance.size(); ++i)
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
    unsigned args)
  {
    assert(categories.size());
    assert(args);

    class swr
    {
    public:
      swr(const cvect &categories, unsigned args)
        : categories_(categories), args_(args)
      {
      }

      void operator()(unsigned level, const cvect &base, std::list<cvect> *out)
      {
        for (unsigned i(0); i < categories_.size(); ++i)
        {
          cvect current(base);
          current.push_back(categories_[i]);

          if (level+1 < args_)
            operator()(level+1, current, out);
          else
            out->push_back(current);
        }
      }

    private:
      const cvect &categories_;
      const unsigned args_;
    };

    std::list<cvect> out;
    swr(categories, args)(0, {}, &out);
    return out;
  }

  ///
  /// \return number of categories of the problem (>= 1).
  ///
  unsigned src_problem::categories() const
  {
    return dat_.categories();
  }

  ///
  /// \return number of classes of the problem (== 0 for a symbolic regression
  ///         problem, > 1 for a classification problem).
  ///
  unsigned src_problem::classes() const
  {
    assert(dat_.classes() != 1);

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
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool src_problem::check(bool verbose) const
  {
    return
      problem::check(verbose) &&
      dat_.check() &&
      vars_.size() == dat_.variables();
  }
}  // namespace vita
