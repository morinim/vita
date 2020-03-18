/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
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

const src_problem::default_symbols_t src_problem::default_symbols = {};

namespace detail
{
///
/// \param[in] availables the "dictionary" for the sequence
/// \param[in] size       size of the output sequence
/// \return               a set of sequences with repetition with elements
///                       taken from a given set (`availables`) and fixed
///                       length (`size`).
///
/// \note This is in the `detail` namespace for ease of testing.
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
/// \warning
/// Users **must** initialize:
/// - the training dataset;
/// - the entire symbol set (functions and terminals)
/// before starting the evolution.
///
src_problem::src_problem() : problem(), training_(), validation_(), factory_()
{
}

///
/// Initializes problem dataset with examples coming from a file.
///
/// \param[in] ds name of the dataset file (CSV or XRFF format)
///
/// \warning
/// - Users **must** specify, at least, the functions to be used;
/// - terminal directly derived from the data (variables / labels) are
///   automatically inserted;
/// - any additional terminals (ephemeral random constants, problem specific
///   constants...) can be manually inserted.
///
src_problem::src_problem(const std::filesystem::path &ds) : src_problem()
{
  vitaINFO << "Reading dataset " << ds << "...";
  data(dataset_t::training).read(ds);

  vitaINFO << "...dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_terminals();
}

///
/// Initializes problem dataset with examples coming from a file.
///
/// \param[in] ds name of the dataset file (CSV or XRFF format)
///
/// \warning
/// - Users **must** specify, at least, the functions to be used;
/// - terminal directly derived from the data (variables / labels) are
///   automatically inserted;
/// - any additional terminals (ephemeral random constants, problem specific
///   constants...) can be manually inserted.
///
src_problem::src_problem(std::istream &ds) : src_problem()
{
  vitaINFO << "Reading dataset from input stream...";
  data(dataset_t::training).read_csv(ds);

  vitaINFO << "...dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_terminals();
}

///
/// Initializes the problem with the default symbol set and data coming from a
/// file.
///
/// \param[in] ds name of the dataset file
///
/// Mainly useful for simple problems (single category regression /
/// classification).
///
src_problem::src_problem(const std::filesystem::path &ds,
                         const default_symbols_t &)
  : src_problem(ds, std::filesystem::path())
{
}

///
/// Initializes the problem with data / symbols coming from input files.
///
/// \param[in] ds      name of the training dataset file
/// \param[in] symbols name of the file containing the symbols to be used.
///
src_problem::src_problem(const std::filesystem::path &ds,
                         const std::filesystem::path &symbols)
  : src_problem()
{
  vitaINFO << "Reading dataset " << ds << "...";
  data(dataset_t::training).read(ds);

  vitaINFO << "....dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_symbols(symbols);
}

///
/// \return `false` if the current problem isn't ready for a run
///
bool src_problem::operator!() const
{
  return !training_.size() || !sset.enough_terminals();
}

///
/// Inserts variables and labels for nominal attributes into the symbol_set.
///
/// \param[in] skip features in this set will be ignored
/// \return         number of variables inserted (one variable per feature)
///
/// \exception `std::insufficient_data` insufficient data to generate the
///             terminal set
///
/// The names used for variables, if not specified in the dataset, are in the
/// form `X1`, ... `Xn`.
///
std::size_t src_problem::setup_terminals(const std::set<unsigned> &skip)
{
  std::size_t variables(0);

  // Sets up the variables (features).
  const auto columns(training_.columns());
  for (std::size_t i(1); i < columns; ++i)
    if (skip.find(i) == skip.end())
    {
      const auto provided_name(training_.get_column(i).name);
      const auto name(provided_name.empty() ? "X" + std::to_string(i)
                                            : provided_name);
      const category_t category(training_.get_column(i).category_id);
      insert<variable>(name, i - 1, category);

      ++variables;
    }

  if (!variables)
    throw exception::insufficient_data("Cannot generate the terminal set");

  // Sets up the labels for nominal attributes.
  for (const category &c : training_.categories())
    for (const std::string &l : c.labels)
      insert<constant<std::string>>(l, c.tag);

  return variables;
}

///
/// Sets up the symbol set.
///
/// \param[in] file name of the file containing the symbols
/// \return         number of parsed symbols
///
/// If a file isn't specified, a predefined set is arranged (useful for simple
/// problems (single category regression / classification).
///
/// \warning
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
std::size_t src_problem::setup_symbols(const std::filesystem::path &file)
{
  sset.clear();

  setup_terminals();

  return file.empty() ? setup_symbols_impl() : setup_symbols_impl(file);
}

///
/// Default symbol set.
///
/// \return number of symbols inserted
///
/// This is useful for simple problems (single category regression /
/// classification).
///
std::size_t src_problem::setup_symbols_impl()
{
  vitaINFO << "Setting up default symbol set...";

  std::size_t inserted(0);

  for (category_t tag(0), sup(categories()); tag < sup; ++tag)
    if (compatible({tag}, {"numeric"}))
    {
      const std::vector<std::string> base(
        {"1.0", "2.0", "3.0", "4.0", "5.0", "6.0", "7.0", "8.0", "9.0",
         "FABS", "FADD", "FDIV", "FLN", "FMUL", "FMOD", "FSUB"});

      for (const auto &s: base)
        if (sset.insert(factory_.make(s, {tag})))
          ++inserted;
    }
    else if (compatible({tag}, {"string"}))
    {
      // for (category_t tag j(0); j < sup; ++j)
      //   if (j != tag)
      //     sset.insert(factory_.make("SIFE", {tag, j}));
      if (sset.insert(factory_.make("SIFE", {tag, 0})))
        ++inserted;
    }

  vitaINFO << "...default symbol set ready. Symbols: " << inserted;
  return inserted;
}

///
/// Initialize the symbols set reading symbols from a file.
///
/// \param[in] file name of the file containing the symbols
/// \return         number of parsed symbols
///
/// \exception exception::data_format wrong data format for symbol file
///
std::size_t src_problem::setup_symbols_impl(const std::filesystem::path &file)
{
  vitaINFO << "Reading symbol set " << file << "...";
  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(file.c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("Symbol set format error");

  // Records the set of categories as inferred from the dataset.
  std::set<category_t> used_categories;
  for (const category &c : training_.categories())
  {
    vitaDEBUG << "Using " << c;
    used_categories.insert(c.tag);
  }

  std::size_t parsed(0);

  // When I wrote this, only God and I understood what I was doing.
  // Now, God only knows.
  tinyxml2::XMLHandle handle(&doc);
  auto *symbolset(handle.FirstChildElement("symbolset").ToElement());

  if (!symbolset)
    throw exception::data_format("Empty symbol set");

  for (auto *s(symbolset->FirstChildElement("symbol"));
       s;
       s = s->NextSiblingElement("symbol"))
  {
    const char *c_sym_name(s->Attribute("name"));
    if (!c_sym_name)
    {
      vitaERROR << "Skipped unnamed symbol in symbolset";
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
            signature += " " + training_.categories()[tag].name;
          vitaDEBUG << "Adding to symbol set " << signature;

          sset.insert(factory_.make(sym_name, cvect(n_args, tag)));
        }
    }
    else  // !sym_sig => complex signature
    {
      auto *sig(s->FirstChildElement("signature"));
      if (!sig)
      {
        vitaERROR << "Skipping " << sym_name << " symbol (empty signature)";
        continue;
      }

      std::vector<std::string> args;
      for (auto *arg(sig->FirstChildElement("arg"));
           arg;
           arg = arg->NextSiblingElement("arg"))
      {
        if (!arg->GetText())
        {
          vitaERROR << "Skipping " << sym_name << " symbol (wrong signature)";
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
            signature += " " + training_.categories()[j].name;
          vitaDEBUG << "Adding to symbol set " << signature;

          sset.insert(factory_.make(sym_name, seq));
        }
    }

    ++parsed;
  }

  vitaINFO << "...symbol set read. Symbols: " << parsed;
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
    const bool generic(from_weka(p_i) != domain_t::d_void);

    if (generic)  // numeric, string, integer...
    {
      if (training_.categories()[instance[i]].domain != from_weka(p_i))
        return false;
    }
    else
    {
      if (instance[i] != training_.categories().find(p_i).tag)
        return false;
    }
  }

  return true;
}

///
/// \return number of categories of the problem (`>= 1`)
///
unsigned src_problem::categories() const
{
  return training_.categories().size();
}

///
/// \return number of classes of the problem (`== 0` for a symbolic regression
///         problem, `> 1` for a classification problem)
///
unsigned src_problem::classes() const
{
  return training_.classes();
}

///
/// \return dimension of the input vectors (i.e. the number of variable of
///         the problem)
///
unsigned src_problem::variables() const
{
  return training_.variables();
}

///
/// \param[in] t a dataset type
/// \return      a reference to the specified dataset
///
dataframe &src_problem::data(dataset_t t)
{
  return t == dataset_t::training ? training_ : validation_;
}

///
/// \param[in] t a dataset type
/// \return      a const reference to the specified dataset
///
const dataframe &src_problem::data(dataset_t t) const
{
  return t == dataset_t::training ? training_ : validation_;
}

///
/// \return `true` if the object passes the internal consistency check
///
bool src_problem::debug() const
{
  if (!problem::debug())
    return false;

  return training_.debug() && validation_.debug();
}

}  // namespace vita
