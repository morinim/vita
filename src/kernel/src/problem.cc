/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2018 EOS di Manlio Morini.
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
}  // unnamed namespace

///
/// New empty instance of src_problem.
///
src_problem::src_problem() : problem(), training_(), validation_(), factory_()
{
}

///
/// Initializes the problem with data from the input files.
///
/// \param[in] ds name of the dataset file
/// \param[in] symbols name of the file containing the symbols. If it's empty,
///                    src_problem::setup_default_symbols is called
///
src_problem::src_problem(const std::string &ds, const std::string &symbols)
  : src_problem()
{
  read(ds, symbols);
}

///
/// \return `false` if the current problem isn't ready for a run
///
bool src_problem::operator!() const
{
  return !training_.size() || !sset.enough_terminals();
}

///
/// Loads data in the active dataframe (optionally also read a symbols file).
///
/// \param[in] ds      filename of the dataset file (training/validation set)
/// \param[in] symbols name of the file containing the symbols. If it's empty,
///                    src_problem::setup_default_symbols is called
/// \return            number of examples (lines) parsed and number of symbols
///                    parsed
///
/// \exception `std::invalid_argument` missing dataset file name
///
std::pair<std::size_t, std::size_t> src_problem::read(
  const std::string &ds, const std::string &symbols)
{
  if (ds.empty())
    throw std::invalid_argument("Missing dataset filename");

  data().clear();

  const auto n_examples(data().read(ds));

  std::size_t n_symbols(0);
  if (symbols.empty())
    setup_default_symbols();
  else
    n_symbols = read_symbols(symbols);

  return {n_examples, n_symbols};
}

///
/// Inserts variables and labels for nominal attributes into the symbol_set.
///
/// \param[in] skip features in this set will be ignored
/// \return         number of variables inserted (one variable per feature)
///
/// The names used for variables, if not specified in the dataset, are in the
/// form `X1`, ... `Xn`.
///
std::size_t src_problem::setup_terminals(const std::set<unsigned> &skip)
{
  std::size_t variables(0);

  // Sets up the variables (features).
  const auto columns(training_.columns());
  for (auto i(decltype(columns){1}); i < columns; ++i)
    if (skip.find(i) == skip.end())
    {
      const auto provided_name(training_.get_column(i).name);
      const auto name(provided_name.empty() ? "X" + std::to_string(i)
                                            : provided_name);
      const category_t category(training_.get_column(i).category_id);
      sset.insert<variable>(name, i - 1, category);

      ++variables;
    }

  // Sets up the labels for nominal attributes.
  for (const category &c : training_.categories())
    for (const std::string &l : c.labels)
      sset.insert<constant<std::string>>(l, c.tag);

  return variables;
}

///
/// Default symbol set.
///
/// This is useful for simple problems (single category regression /
/// classification).
///
bool src_problem::setup_default_symbols()
{
  sset.clear();

  if (!setup_terminals())
    return false;

  vitaINFO << "Setting up default symbol set";

  for (category_t tag(0), sup(categories()); tag < sup; ++tag)
    if (compatible({tag}, {"numeric"}))
    {
      sset.insert(factory_.make("1.0", {tag}));
      sset.insert(factory_.make("2.0", {tag}));
      sset.insert(factory_.make("3.0", {tag}));
      sset.insert(factory_.make("4.0", {tag}));
      sset.insert(factory_.make("5.0", {tag}));
      sset.insert(factory_.make("6.0", {tag}));
      sset.insert(factory_.make("7.0", {tag}));
      sset.insert(factory_.make("8.0", {tag}));
      sset.insert(factory_.make("9.0", {tag}));
      sset.insert(factory_.make("FABS", {tag}));
      sset.insert(factory_.make("FADD", {tag}));
      sset.insert(factory_.make("FDIV", {tag}));
      sset.insert(factory_.make("FLN",  {tag}));
      sset.insert(factory_.make("FMUL", {tag}));
      sset.insert(factory_.make("FMOD", {tag}));
      sset.insert(factory_.make("FSUB", {tag}));
    }
    else if (compatible({tag}, {"string"}))
    {
      // for (decltype(tag) j(0); j < sup; ++j)
      //   if (j != tag)
      //     sset.insert(factory_.make("SIFE", {tag, j}));
      sset.insert(factory_.make("SIFE", {tag, 0}));
    }

  return true;
}

///
/// \param[in] s_file name of the file containing the symbols
/// \return           number of parsed symbols
///
/// \warning Check the return value (it must be greater than `0`).
///
/// \warning
/// Data should be loaded before symbols: without data we don't know, among
/// other things, the features the dataset has.
///
std::size_t src_problem::read_symbols(const std::string &s_file)
{
  sset.clear();

  if (!setup_terminals())
    return 0;

  // Prints the list of categories as inferred from the dataset.
  for (const category &c : training_.categories())
    vitaDEBUG << "Using " << c;

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
/// \return a reference to the active dataset
///
dataframe &src_problem::data()
{
  return active_dataset() == problem::training ? training_ : validation_;
}

///
/// \return a const reference to the active dataset
///
const dataframe &src_problem::data() const
{
  return active_dataset() == problem::training ? training_ : validation_;
}

///
/// \param[in] t a dataset type
/// \return      a reference to the specified dataset
///
dataframe &src_problem::data(dataset_t t)
{
  return t == problem::training ? training_ : validation_;
}

///
/// \param[in] t a dataset type
/// \return      a const reference to the specified dataset
///
const dataframe &src_problem::data(dataset_t t) const
{
  return t == problem::training ? training_ : validation_;
}

///
/// Asks if a dataset of a specific kind is available.
///
/// \param[in] d dataset type
/// \return      `true` if dataset of type `d` is available
///
bool src_problem::has(dataset_t d) const
{
  switch (d)
  {
  case problem::training:    return !training_.empty();
  case problem::validation:  return !validation_.empty();
  default:                   return false;
  }
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
