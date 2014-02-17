/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "kernel/src/problem.h"
#include "kernel/lambda_f.h"
#include "kernel/src/constant.h"
#include "kernel/src/evaluator.h"
#include "kernel/src/variable.h"

namespace vita
{
  ///
  /// \param[in] initialize if \c true initialize the environment with default
  ///                       values.
  /// New empty instance.
  ///
  /// \note
  /// Usually the environment isn't initialized so that the search class would
  /// choose the best values for the specific problem before starting the
  /// run.
  /// Anyway, for debug purpose, we can force a default environment via the
  /// \a initialize parameter.
  ///
  src_problem::src_problem(bool initialize) : problem(initialize)
  {
  }

  ///
  /// \param[in] ds name of the dataset file (training/validation set).
  /// \param[in] ts name of the test set.
  /// \param[in] symbols name of the file containing the symbols. If it is
  ///                    empty, \c src_problem::setup_default_symbols is called.
  ///
  /// Initializa the problem with data from the input files.
  ///
  src_problem::src_problem(const std::string &ds, const std::string &ts,
                           const std::string &symbols) : problem(false)
  {
    load(ds, ts, symbols);
  }

  ///
  /// \return \c false if the current problem isn't ready for a run.
  ///
  bool src_problem::operator!() const
  {
    std::cout << dat_.size(data::training) << sset.enough_terminals()
              << std::endl;
    return !dat_.size(data::training) || !sset.enough_terminals();
  }

  ///
  /// \param[in] initialize if \c true initialize the environment with default
  ///                       values.
  ///
  /// Resets the object.
  ///
  void src_problem::clear(bool initialize)
  {
    problem::clear(initialize);
    dat_.clear();
  }

  ///
  /// \param[in] ds name of the dataset file (training/validation set).
  /// \param[in] ts name of the test set.
  /// \param[in] symbols name of the file containing the symbols. If it is
  ///                    empty, \c src_problem::setup_default_symbols is called.
  /// \return number of examples (lines) parsed and number of symbols parsed.
  ///
  /// Loads \a data into the active dataset.
  ///
  std::pair<size_t, size_t> src_problem::load(const std::string &ds,
                                              const std::string &ts,
                                              const std::string &symbols)
  {
    if (ds.empty())
      return {0, 0};

    sset = vita::symbol_set();
    dat_.clear();

    const auto n_examples(dat_.open(ds, env.verbosity));

    if (!ts.empty())
      load_test_set(ts);

    const unsigned n_symbols(symbols.empty() ? setup_default_symbols(), 0
                                             : load_symbols(symbols));

    return {n_examples, n_symbols};
  }

  ///
  /// \param[in] ts name of the file containing the test set.
  /// \return number of examples parsed.
  ///
  /// Load the test set.
  ///
  size_t src_problem::load_test_set(const std::string &ts)
  {
    const auto backup(dat_.dataset());
    dat_.dataset(data::test);
    const auto n(dat_.open(ts, env.verbosity));
    dat_.dataset(backup);

    return n;
  }

  ///
  /// param[in] skip features in this set will be ignored.
  ///
  /// Inserts into the symbol_set variables and labels for nominal
  /// attributes.
  ///
  void src_problem::setup_terminals_from_data(const std::set<unsigned> &skip)
  {
    sset = vita::symbol_set();

    // Sets up the variables (features).
    const auto columns(dat_.columns());
    for (auto i(decltype(columns){1}); i < columns; ++i)
      if (skip.find(i) == skip.end())
      {
        std::string name(dat_.get_column(i).name);
        if (name.empty())
          name = "X" + boost::lexical_cast<std::string>(i);

        const category_t category(dat_.get_column(i).category_id);
        sset.insert(make_unique<variable>(name, i - 1, category));
      }

    // Sets up the labels for nominal attributes.
    const auto categories(dat_.categories());
    for (category_t i(0); i < categories; ++i)
    {
      const auto &category(dat_.get_category(i));

      for (const std::string &label : category.labels)
        sset.insert(make_unique<constant<std::string>>(label, i));
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
        sset.insert(factory.make("1.0", {category}));
        sset.insert(factory.make("2.0", {category}));
        sset.insert(factory.make("3.0", {category}));
        sset.insert(factory.make("4.0", {category}));
        sset.insert(factory.make("5.0", {category}));
        sset.insert(factory.make("6.0", {category}));
        sset.insert(factory.make("7.0", {category}));
        sset.insert(factory.make("8.0", {category}));
        sset.insert(factory.make("9.0", {category}));
        sset.insert(factory.make("FABS", {category}));
        sset.insert(factory.make("FADD", {category}));
        sset.insert(factory.make("FDIV", {category}));
        sset.insert(factory.make("FLN",  {category}));
        sset.insert(factory.make("FMUL", {category}));
        sset.insert(factory.make("FMOD", {category}));
        sset.insert(factory.make("FSUB", {category}));
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
    const auto c_size(categories.size());
    for (auto i(decltype(c_size){0}); i < c_size; ++i)
      categories[i] = i;

    // Load the XML file (sf) into the property tree (pt).
    using namespace boost::property_tree;
    ptree pt;
    read_xml(sf, pt);

#if !defined(NDEBUG)
    std::cout << std::endl << std::endl;
    for (auto i(decltype(c_size){0}); i < c_size; ++i)
      std::cout << "[DEBUG] Category " << i << ": "
                << dat_.get_category(i) << std::endl;
    std::cout << std::endl;
#endif

    symbol_factory &factory(symbol_factory::instance());

    for (const auto &s : pt.get_child("symbolset"))
      if (s.first == "symbol")
      {
        const auto sym_name(s.second.get<std::string>("<xmlattr>.name"));
        const auto sym_sig(s.second.get<std::string>("<xmlattr>.signature",
                                                     ""));

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
              for (const auto &seq : sequences)
                if (compatible(seq, args))
                {
#if !defined(NDEBUG)
                  //const domain_t domain(dat_.get_category(i.back()).domain);
                  std::cout << "[DEBUG] " << sym_name << '(';
                  for (const auto &j : seq)
                    std::cout << dat_.get_category(j).name
                              << (&j == &seq.back() ? ")" : ", ");
                  std::cout << std::endl;
#endif
                  sset.insert(factory.make(sym_name, seq));
                }
            }
        }
        else  // !sym_sig.empty() => one category, uniform symbol initialization
        {
          for (category_t category(0); category < dat_.categories(); ++category)
            if (compatible({category}, {sym_sig}))
            {
              const auto n_args(factory.args(sym_name));

#if !defined(NDEBUG)
              std::cout << sym_name << '(';
              for (auto j(decltype(n_args){0}); j < n_args; ++j)
                std::cout << dat_.get_category(category).name
                          << (j + 1 == n_args ? ")" : ", ");
              std::cout << std::endl;
#endif
              sset.insert(factory.make(sym_name, cvect(n_args, category)));
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
      bool generic(data::from_weka(pattern[i]) != domain_t::d_void);

      if (generic)  // numeric, string, integer...
      {
        if (dat_.get_category(instance[i]).domain !=
            data::from_weka(pattern[i]))
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
    const cvect &categories, size_t args)
  {
    assert(categories.size());
    assert(args);

    // When I wrote this, only God and I understood what I was doing.
    // Now, God only knows.
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
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool src_problem::debug(bool verbose) const
  {
    if (!problem::debug(verbose))
      return false;

    if (!dat_.debug())
      return false;

    return true;
  }
}  // namespace vita
