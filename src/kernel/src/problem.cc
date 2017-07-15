/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "tinyxml2/tinyxml2.h"

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
/// \param[in] availables the "dictionary" for the sequence
/// \param[in] size       size of the output sequence
/// \return               a set of sequences with repetition with elements
///                       taken from a given set (`availables`) and fixed
///                       length (`size`).
///
template<class C>
std::set<std::vector<C>> seq_with_rep(const std::set<C> &availables,
                                      std::size_t size)
{
  Expects(availables.size());
  Expects(size);

  std::set<std::vector<C>> ret;

  std::function<void (std::size_t, std::vector<C>)> swr(
    [&](std::size_t left, std::vector<C> current)
    {
      if (!left)  // we have a sequence of the correct length
      {
        ret.insert(current);
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
/// New empty instance of src_problem.
///
/// \param[in] initialize should the environment be initialized with default
///                       values?
///
src_problem::src_problem(bool initialize) : problem(initialize), dat_(),
                                            factory_()
{
}

///
/// A delegating constructor to avoid that the `src_problem("data.csv")` call
/// call could be resolved with `src_problem(bool)`.
///
/// Conversion from `const char []` to `bool` is a standard conversion, while
/// the one to `std::string` is a user-defined conversion.
///
src_problem::src_problem(const char ds[])
  : src_problem(std::string(ds), std::string(), std::string())
{
}

///
/// Initializes the problem with data from the input files.
///
/// \param[in] ds name of the dataset file (training/validation set)
/// \param[in] ts name of the test set
/// \param[in] symbols name of the file containing the symbols. If it is
///                    empty, src_problem::setup_default_symbols is called
///
src_problem::src_problem(const std::string &ds, const std::string &ts,
                         const std::string &symbols)
  : problem(false), dat_(), factory_()
{
  load(ds, ts, symbols);
}

///
/// \return `false` if the current problem isn't ready for a run
///
bool src_problem::operator!() const
{
  return !dat_.size(data::training) || !env.sset->enough_terminals();
}

///
/// Loads data from file.
///
/// \param[in] ds      filename of the dataset file (training/validation set)
/// \param[in] ts      filename of the test set
/// \param[in] symbols name of the file containing the symbols. If it's empty,
///                    src_problem::setup_default_symbols is called
/// \return            number of examples (lines) parsed and number of symbols
///                    parsed
///
std::pair<std::size_t, std::size_t> src_problem::load(
  const std::string &ds, const std::string &ts, const std::string &symbols)
{
  if (ds.empty())
    return {0, 0};

  env.sset->clear();
  dat_.clear();

  const auto n_examples(dat_.load(ds));

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
/// Loads the test set.
///
/// \param[in] ts filename of the file containing the test set
/// \return       number of examples parsed
///
std::size_t src_problem::load_test_set(const std::string &ts)
{
  const auto backup(dat_.active_dataset());
  dat_.select(data::test);
  const auto n(dat_.load(ts));
  dat_.select(backup);

  return n;
}

///
/// Inserts variables and labels for nominal attributes into the symbol_set.
///
/// param[in] skip features in this set will be ignored
///
/// The names used for variables, if not specified in the dataset, are in the
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
      const auto provided_name(dat_.get_column(i).name);
      const auto name(provided_name.empty() ? "X" + std::to_string(i)
                                            : provided_name);
      const category_t category(dat_.get_column(i).category_id);
      env.sset->insert(std::make_unique<variable>(name, i - 1, category));
    }

  // Sets up the labels for nominal attributes.
  for (const category &c : dat_.categories())
    for (const std::string &l : c.labels)
      env.sset->insert(std::make_unique<constant<std::string>>(l, c.tag));
}

///
/// Default symbol set.
///
/// This is useful for simple problems (single category regression /
/// classification).
///
void src_problem::setup_default_symbols()
{
  setup_terminals_from_data();

  for (category_t tag(0), sup(categories()); tag < sup; ++tag)
    if (compatible({tag}, {"numeric"}))
    {
      env.sset->insert(factory_.make("1.0", {tag}));
      env.sset->insert(factory_.make("2.0", {tag}));
      env.sset->insert(factory_.make("3.0", {tag}));
      env.sset->insert(factory_.make("4.0", {tag}));
      env.sset->insert(factory_.make("5.0", {tag}));
      env.sset->insert(factory_.make("6.0", {tag}));
      env.sset->insert(factory_.make("7.0", {tag}));
      env.sset->insert(factory_.make("8.0", {tag}));
      env.sset->insert(factory_.make("9.0", {tag}));
      env.sset->insert(factory_.make("FABS", {tag}));
      env.sset->insert(factory_.make("FADD", {tag}));
      env.sset->insert(factory_.make("FDIV", {tag}));
      env.sset->insert(factory_.make("FLN",  {tag}));
      env.sset->insert(factory_.make("FMUL", {tag}));
      env.sset->insert(factory_.make("FMOD", {tag}));
      env.sset->insert(factory_.make("FSUB", {tag}));
    }
    else if (compatible({tag}, {"string"}))
    {
      // for (decltype(tag) j(0); j < sup; ++j)
      //   if (j != tag)
      //     env.sset->insert(factory_.make("SIFE", {tag, j}));
      env.sset->insert(factory_.make("SIFE", {tag, 0}));
    }
}

///
/// \param[in] s_file name of the file containing the symbols
/// \return           number of parsed symbols
///
/// \note
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
std::size_t src_problem::load_symbols(const std::string &s_file)
{
  setup_terminals_from_data();

  // Prints the list of categories as inferred from the dataset.
  for (const category &c : dat_.categories())
    print.debug("Using ", c);

  std::set<category_t> used_categories;
  for (category_t i(0); i < categories(); ++i)
    used_categories.insert(used_categories.end(), i);

  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(s_file.c_str()) != tinyxml2::XML_SUCCESS)
    return 0;

  std::size_t parsed(0);

  // When I wrote this, only God and I understood what I was doing.
  // Now, God only knows.
  tinyxml2::XMLHandle handle(&doc);
  auto *symbolset(handle.FirstChildElement("symbolset").ToElement());

  if (!symbolset)
    return 0;

  for (auto *s(symbolset->FirstChildElement("symbol"));
       s;
       s = s->NextSiblingElement("symbol"))
  {
    const char *c_sym_name(s->Attribute("name"));
    if (!c_sym_name)
    {
      print.error("Skipped unnamed symbol in symbolset");
      continue;
    }
    const std::string sym_name(c_sym_name);

    const char *sym_sig(s->Attribute("signature"));

    if (sym_sig)  // single category, uniform initialization
    {
      for (auto tag : used_categories)
        if (compatible({tag}, {std::string(sym_sig)}))
        {
          const auto n_args(factory_.args(sym_name));
          std::string signature(sym_name + ":");

          for (auto j(decltype(n_args){0}); j < n_args; ++j)
            signature += " " + dat_.categories().find(tag).name;
          print.debug("Adding to symbol set ", signature);

          env.sset->insert(factory_.make(sym_name, cvect(n_args, tag)));
        }
    }
    else  // !sym_sig => complex signature
    {
      auto *sig(s->FirstChildElement("signature"));
      if (!sig)
      {
        print.error("Skipping " + sym_name + " symbol (empty signature)");
        continue;
      }

      std::vector<std::string> args;
      for (auto *arg(sig->FirstChildElement("arg"));
           arg;
           arg = arg->NextSiblingElement("arg"))
      {
        if (!arg->GetText())
        {
          print.error("Skipping " + sym_name + " symbol (wrong signature)");
          args.clear();
          break;
        }

        args.push_back(arg->GetText());
      }

      // From the list of all the sequences with repetition of `args.size()`
      // elements...
      const auto sequences(detail::seq_with_rep(used_categories, args.size()));

      // ...we choose those compatible with the xml signature of the current
      // symbol.
      for (const auto &seq : sequences)
        if (compatible(seq, args))
        {
          std::string signature(sym_name + ":");
          for (const auto &j : seq)
            signature += " " + dat_.categories().find(j).name;
          print.debug("Adding to symbol set ", signature);

          env.sset->insert(factory_.make(sym_name, seq));
        }
    }

    ++parsed;
  }

  return parsed;
}

///
/// Checks if a sequence of categories matches a sequence of domain names.
///
/// \param[in] instance a vector of categories
/// \param[in] pattern  a mixed vector of category names and domain names
/// \return             `true` if `instance` match `pattern`
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
  Expects(instance.size() == pattern.size());

  const auto sup(instance.size());
  for (auto i(decltype(sup){0}); i < sup; ++i)
  {
    const std::string p_i(pattern[i]);
    const bool generic(src_data::from_weka(p_i) != domain_t::d_void);

    if (generic)  // numeric, string, integer...
    {
      if (dat_.categories().find(instance[i]).domain !=
          src_data::from_weka(p_i))
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
/// \return number of categories of the problem (>= 1)
///
unsigned src_problem::categories() const
{
  return dat_.categories().size();
}

///
/// \return number of classes of the problem (`== 0` for a symbolic regression
///         problem, > 1 for a classification problem)
///
unsigned src_problem::classes() const
{
  return dat_.classes();
}

///
/// \return dimension of the input vectors (i.e. the number of variable of
///         the problem)
///
unsigned src_problem::variables() const
{
  return dat_.variables();
}

///
/// \return `true` if the object passes the internal consistency check
///
bool src_problem::debug() const
{
  if (!problem::debug())
    return false;

  if (!dat_.debug())
    return false;

  return true;
}

}  // namespace vita
