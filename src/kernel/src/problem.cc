/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
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

namespace detail
{
///
/// \param[in] availables the "dictionary" for the sequence.
/// \param[in] size size of the output sequence.
/// \return a vector of sequences with repetition with elements taken from a
///         given set (`availables`) and fixed length (`size`).
///
template<class C>
std::vector<C> seq_with_rep(const C &availables, std::size_t size)
{
  assert(availables.size());
  assert(size);

  std::vector<C> ret;

  std::function<void (std::size_t, C)> swr(
    [&](std::size_t left, C current)
    {
      if (!left)  // we have a sequence of the correct length
      {
        ret.push_back(current);
        return;
      }

      for (auto elem : availables)
      {
        current.push_back(elem);
        swr(left - 1, current);
        current.pop_back();
      }
    });

  swr(size, {});
  return ret;
}
}  // namespace detail

///
/// \brief New empty instance of src_problem
///
/// Usually the environment isn't initialized so that the search class would
/// choose the best values for the specific problem before starting the
/// run (this is how the constructor works).
///
/// Anyway, for debug purpose, we can set up a default environment in a
/// two steps:
///
///     src_problem p;
///     p.clear(true);
///
src_problem::src_problem() : problem()
{
}

///
/// \param[in] ds name of the dataset file (training/validation set).
/// \param[in] ts name of the test set.
/// \param[in] symbols name of the file containing the symbols. If it is
///                    empty, src_problem::setup_default_symbols is called.
///
/// Initialize the problem with data from the input files.
///
src_problem::src_problem(const std::string &ds, const std::string &ts,
                         const std::string &symbols)
  : problem()
{
  load(ds, ts, symbols);
}

///
/// \return `false` if the current problem isn't ready for a run.
///
bool src_problem::operator!() const
{
  return !dat_.size(data::training) || !env.sset->enough_terminals();
}

///
/// \param[in] initialize if `true` initialize the environment with default
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
///                    empty, src_problem::setup_default_symbols is called.
/// \return number of examples (lines) parsed and number of symbols parsed.
///
/// Loads `data` into the active dataset.
///
std::pair<std::size_t, std::size_t> src_problem::load(
  const std::string &ds, const std::string &ts, const std::string &symbols)
{
  if (ds.empty())
    return {0, 0};

  env.sset->clear();
  dat_.clear();

  const auto n_examples(dat_.open(ds, env.verbosity));

  if (!ts.empty())
    load_test_set(ts);

  std::size_t n_symbols(0);
  if (symbols.empty())
    setup_default_symbols();
  else
    n_symbols = load_symbols(symbols);

  return {n_examples, n_symbols};
}

///
/// \param[in] ts name of the file containing the test set.
/// \return number of examples parsed.
///
/// Load the test set.
///
std::size_t src_problem::load_test_set(const std::string &ts)
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
/// Inserts variables and labels for nominal attributes into the symbol_set.
/// The name used for variables, if not specified in the dataset, are in the
/// form `X1`, ... `Xn`.
///
void src_problem::setup_terminals_from_data(const std::set<unsigned> &skip)
{
  env.sset->clear();

  // Sets up the variables (features).
  const auto columns(dat_.columns());
  for (auto i(decltype(columns){1}); i < columns; ++i)
    if (skip.find(i) == skip.end())
    {
      std::string name(dat_.get_column(i).name);
      if (name.empty())
        name = "X" + std::to_string(i);

      const category_t category(dat_.get_column(i).category_id);
      env.sset->insert(vita::make_unique<variable>(name, i - 1, category));
    }

  // Sets up the labels for nominal attributes.
  for (const category &c : dat_.categories())
    for (const std::string &l : c.labels)
      env.sset->insert(vita::make_unique<constant<std::string>>(l, c.tag));
}

///
/// Default symbol set. This is useful for simple problems (single category
/// regression / classification).
///
void src_problem::setup_default_symbols()
{
  setup_terminals_from_data();

  symbol_factory &factory(symbol_factory::instance());

  for (category_t tag(0), sup(categories()); tag < sup; ++tag)
    if (compatible({tag}, {"numeric"}))
    {
      env.sset->insert(factory.make("1.0", {tag}));
      env.sset->insert(factory.make("2.0", {tag}));
      env.sset->insert(factory.make("3.0", {tag}));
      env.sset->insert(factory.make("4.0", {tag}));
      env.sset->insert(factory.make("5.0", {tag}));
      env.sset->insert(factory.make("6.0", {tag}));
      env.sset->insert(factory.make("7.0", {tag}));
      env.sset->insert(factory.make("8.0", {tag}));
      env.sset->insert(factory.make("9.0", {tag}));
      env.sset->insert(factory.make("FABS", {tag}));
      env.sset->insert(factory.make("FADD", {tag}));
      env.sset->insert(factory.make("FDIV", {tag}));
      env.sset->insert(factory.make("FLN",  {tag}));
      env.sset->insert(factory.make("FMUL", {tag}));
      env.sset->insert(factory.make("FMOD", {tag}));
      env.sset->insert(factory.make("FSUB", {tag}));
    }
    else if (compatible({tag}, {"string"}))
    {
      //for (decltype(tag) j(0); j < sup; ++j)
      // if (j != tag)
      //   env.sset->insert(factory.make("SIFE", {tag, j}));
      env.sset->insert(factory.make("SIFE", {tag, 0}));
    }
}

///
/// \param[in] s_file name of the file containing the symbols.
/// \return number of parsed symbols.
///
/// \note
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
unsigned src_problem::load_symbols(const std::string &s_file)
{
  setup_terminals_from_data();

  unsigned parsed(0);

  cvect used_categories(categories());
  std::iota(used_categories.begin(), used_categories.end(), 0);

  boost::property_tree::ptree pt;
  read_xml(s_file, pt);

  // Prints the list of categories as inferred from the dataset.
  if (env.verbosity >= 3)
  {
    std::cout << "\n\n";

    for (const category &c : dat_.categories())
      std::cout << k_s_debug << ' ' << c << '\n';

    std::cout << '\n';
  }

  symbol_factory &factory(symbol_factory::instance());

  // When I wrote this, only God and I understood what I was doing.
  // Now, God only knows.
  for (const auto &s : pt.get_child("symbolset"))
  {
    if (s.first != "symbol")
      continue;

    const auto sym_name(s.second.get<std::string>("<xmlattr>.name"));
    const auto sym_sig(s.second.get<std::string>("<xmlattr>.signature", ""));

    if (sym_sig.empty())
    {
      for (const auto &sig : s.second)
        if (sig.first == "signature")
        {
          std::vector<std::string> args;
          for (const auto &arg : sig.second)
            if (arg.first == "arg")
              args.push_back(arg.second.data());

          // From the list of all the sequences with repetition of
          // args.size() elements (categories)...
          const auto sequences(detail::seq_with_rep(used_categories,
                                                    args.size()));

          // ...we choose those compatible with the xml signature of the
          // current symbol.
          for (const auto &seq : sequences)
            if (compatible(seq, args))
            {
              if (env.verbosity >= 3)
              {
                std::cout << k_s_debug << ' ' << sym_name << '(';
                for (const auto &j : seq)
                  std::cout << dat_.categories().find(j).name
                            << (&j == &seq.back() ? ")" : ", ");
                std::cout << '\n';
              }

              env.sset->insert(factory.make(sym_name, seq));
            }
        }
    }
    else  // !sym_sig.empty() => single category, uniform initialization
    {
      for (category_t tag(0), sup(categories()); tag < sup; ++tag)
        if (compatible({tag}, {sym_sig}))
        {
          const auto n_args(factory.args(sym_name));

          if (env.verbosity >= 3)
          {
            std::cout << k_s_debug << ' ' << sym_name << '(';
            for (auto j(decltype(n_args){0}); j < n_args; ++j)
              std::cout << dat_.categories().find(tag).name
                        << (j + 1 == n_args ? ")" : ", ");
            std::cout << '\n';
          }

          env.sset->insert(factory.make(sym_name, cvect(n_args, tag)));
        }
    }

    ++parsed;
  }

  return parsed;
}

///
/// \param[in] instance a vector of categories.
/// \param[in] pattern a mixed vector of category names and domain names.
/// \return `true` if `instance` match `pattern`.
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

  const auto sup(instance.size());
  for (auto i(decltype(sup){0}); i < sup; ++i)
  {
    const std::string p_i(pattern[i]);
    const bool generic(data::from_weka(p_i) != domain_t::d_void);

    if (generic)  // numeric, string, integer...
    {
      if (dat_.categories().find(instance[i]).domain != data::from_weka(p_i))
        return false;
    }
    else
    {
      if (instance[i] != dat_.categories().find(p_i).tag)
        return false;
    }
  }

  return true;
}

///
/// \return number of categories of the problem (>= 1).
///
unsigned src_problem::categories() const
{
  return dat_.categories().size();
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
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the object passes the internal consistency check.
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
