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

#include <set>

#include "kernel/gp/src/problem.h"
#include "kernel/gp/src/constant.h"
#include "kernel/gp/src/evaluator.h"
#include "kernel/gp/src/lambda_f.h"
#include "kernel/gp/src/variable.h"

#include "third_party/tinyxml2/tinyxml2.h"

namespace vita
{

const src_problem::default_symbols_t src_problem::default_symbols = {};

namespace detail
{
///
/// \param[in] availables the "dictionary" for the sequence
/// \param[in] size       length of the output sequence
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

  std::function<void (std::size_t, std::vector<C>)> swr;
  swr = [&](std::size_t left, std::vector<C> current)
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
        };

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
/// \param[in] t  weak or strong typing
///
/// \warning
/// - Users **must** specify, at least, the functions to be used;
/// - terminals directly derived from the data (variables / labels) are
///   automatically inserted;
/// - any additional terminal (ephemeral random constant, problem specific
///   constant...) can be manually inserted.
///
src_problem::src_problem(const std::filesystem::path &ds, typing t)
  : src_problem()
{
  vitaINFO << "Reading dataset " << ds << "...";
  data(dataset_t::training).read(ds);

  vitaINFO << "...dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_terminals(t);
}

///
/// Initializes problem dataset with examples coming from a file.
///
/// \param[in] ds dataset
/// \param[in] t  weak or strong typing
///
/// \warning
/// - Users **must** specify, at least, the functions to be used;
/// - terminals directly derived from the data (variables / labels) are
///   automatically inserted;
/// - any additional terminal (ephemeral random constant, problem specific
///   constants...) can be manually inserted.
///
src_problem::src_problem(std::istream &ds, typing t) : src_problem()
{
  vitaINFO << "Reading dataset from input stream...";
  data(dataset_t::training).read_csv(ds);

  vitaINFO << "...dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_terminals(t);
}

///
/// Initializes the problem with the default symbol set and data coming from a
/// file.
///
/// \param[in] ds name of the dataset file
/// \param[in] t  weak or strong typing
///
/// Mainly useful for simple problems (single category regression /
/// classification) or for the initial approach.
///
src_problem::src_problem(const std::filesystem::path &ds,
                         const default_symbols_t &, typing t)
  : src_problem(ds, std::filesystem::path(), t)
{
}

///
/// Initializes the problem with data / symbols coming from input files.
///
/// \param[in] ds      name of the training dataset file
/// \param[in] symbols name of the file containing the symbols to be used.
/// \param[in] t       weak or strong typing
///
src_problem::src_problem(const std::filesystem::path &ds,
                         const std::filesystem::path &symbols, typing t)
  : src_problem()
{
  vitaINFO << "Reading dataset " << ds << "...";
  data(dataset_t::training).read(ds);

  vitaINFO << "....dataset read. Examples: " << data(dataset_t::training).size()
           << ", categories: " << categories()
           << ", features: " << variables()
           << ", classes: " << classes();

  setup_symbols(symbols, t);
}

///
/// \return `false` if the current problem isn't ready for a run
///
bool src_problem::operator!() const
{
  return !training_.size() || !sset.enough_terminals();
}

///
/// Inserts variables and states for nominal attributes into the symbol_set.
///
/// \param[in] t weak or strong typing
///
/// \exception `std::data_format`       unsupported state domain
///
/// There is one variable for each feature.
///
/// The names used for variables, if not specified in the dataset, are in the
/// form `X1`, ... `Xn`.
///
void src_problem::setup_terminals(typing t)
{
  vitaINFO << "Setting up terminals...";

  const auto &columns(training_.columns);
  if (columns.size() < 2)
    throw exception::insufficient_data("Cannot generate the terminal set");

  std::string variables;

  category_set categories(training_.columns, t);
  for (std::size_t i(1); i < columns.size(); ++i)
  {
    // Sets up the variables (features).
    const auto provided_name(columns[i].name);
    const auto name(provided_name.empty() ? "X" + std::to_string(i)
                                          : provided_name);
    const category_t category(categories.column(i).category);

    if (insert<variable>(name, static_cast<unsigned>(i - 1), category))
      variables += " `" + name + "`";

    // Sets up states for nominal attributes.
    for (const auto &s : columns[i].states)
      switch (columns[i].domain)
      {
      case d_double:
        insert<constant<D_DOUBLE>>(std::get<D_DOUBLE>(s), category);
        break;
      case d_int:
        insert<constant<D_INT>>(std::get<D_INT>(s), category);
        break;
      case d_string:
        insert<constant<D_STRING>>(std::get<D_STRING>(s), category);
        break;
      default:
        exception::insufficient_data("Cannot generate the terminal set");
      }

    /*std::visit([this, category](const auto &cs)
                 {
                   using T = std::decay_t<decltype(cs)>>;
                   insert<constant<T>(cs, category);
                   });*/
  }

  vitaINFO << "...terminals ready. Variables:" << variables;
}

///
/// Sets up the symbol set.
///
/// \param[in] t weak or strong typing
/// \return      number of parsed symbols
///
/// A predefined set is arranged (useful for simple problems: single category
/// regression / classification).
///
/// \warning
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
std::size_t src_problem::setup_symbols(typing t)
{
  return setup_symbols({}, t);
}

///
/// Sets up the symbol set.
///
/// \param[in] file name of the file containing the symbols
/// \return         number of parsed symbols
///
/// If a file isn't specified, a predefined set is arranged (useful for simple
/// problems: single category regression / classification).
///
/// \warning
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
std::size_t src_problem::setup_symbols(const std::filesystem::path &file,
                                       typing t)
{
  sset.clear();

  setup_terminals(t);

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

  category_set categories(training_.columns);
  const auto used_categories(categories.used_categories());
  std::size_t inserted(0);

  for (const auto &category : used_categories)
    if (compatible({category}, {"numeric"}, categories))
    {
      const std::vector<std::string> base(
        {"1.0", "2.0", "3.0", "4.0", "5.0", "6.0", "7.0", "8.0", "9.0",
         "FABS", "FADD", "FDIV", "FLN", "FMUL", "FMOD", "FSUB"});

      for (const auto &s: base)
        if (sset.insert(factory_.make(s, {category})))
          ++inserted;
    }
    else if (compatible({category}, {"string"}, categories))
    {
      if (sset.insert(factory_.make("SIFE", {category, 0})))
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
  if (doc.LoadFile(file.string().c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("Symbol set format error");

  category_set categories(training_.columns);
  const auto used_categories(categories.used_categories());
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
    if (!s->Attribute("name"))
    {
      vitaERROR << "Skipped unnamed symbol in symbolset";
      continue;
    }
    const std::string sym_name(s->Attribute("name"));

    if (const char *sym_sig = s->Attribute("signature")) // single category,
    {                                                    // uniform init
      for (auto category : used_categories)
        if (compatible({category}, {std::string(sym_sig)}, categories))
        {
          const auto n_args(factory_.args(sym_name));
          std::string signature(sym_name + ":");

          for (std::size_t j(0); j < n_args; ++j)
            signature += " " + std::to_string(category);
          vitaDEBUG << "Adding to symbol set " << signature;

          sset.insert(factory_.make(sym_name, cvect(n_args, category)));
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
        if (compatible(seq, args, categories))
        {
          std::string signature(sym_name + ":");
          for (const auto &j : seq)
            signature += " " + std::to_string(j);
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
/// \param[in] instance   a vector of categories
/// \param[in] pattern    a mixed vector of category names and domain names
/// \param[in] categories
/// \return               `true` if `instance` match `pattern`
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
                             const std::vector<std::string> &pattern,
                             const category_set &categories) const
{
  Expects(instance.size() == pattern.size());

  const auto sup(instance.size());
  for (std::size_t i(0); i < sup; ++i)
  {
    const std::string p_i(pattern[i]);
    const bool generic(from_weka(p_i) != d_void);

    if (generic)  // numeric, string, integer...
    {
      if (categories.category(instance[i]).domain != from_weka(p_i))
        return false;
    }
    else
    {
      if (instance[i] != categories.column(p_i).category)
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
  return static_cast<unsigned>(sset.categories());
}

///
/// \return number of classes of the problem (`== 0` for a symbolic regression
///         problem, `> 1` for a classification problem)
///
unsigned src_problem::classes() const
{
  return static_cast<unsigned>(training_.classes());
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
bool src_problem::is_valid() const
{
  return problem::is_valid();
}

}  // namespace vita
