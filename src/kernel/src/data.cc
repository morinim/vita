/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <vector>

#include "kernel/src/data.h"
#include "kernel/log.h"
#include "kernel/random.h"
#include "kernel/symbol.h"

#include "tinyxml2/tinyxml2.h"

namespace vita
{

namespace
{
///
/// \param[in] s the string to be converted.
/// \param[in] d what type should `s` be converted in?
/// \return the converted data or an empty `any` if no conversion can be
///         applied.
///
///     convert("123.1", sym_double) == any(123.1f)
///
any convert(const std::string &s, domain_t d)
{
  switch (d)
  {
  case domain_t::d_bool:   return any(std::stoi(s));
  case domain_t::d_int:    return any(std::stoi(s));
  case domain_t::d_double: return any(std::stod(s));
  case domain_t::d_string: return            any(s);
  default:                 return             any();
  }
}

///
/// \param[in] s the string to be tested.
/// \return `true` if `s` contains a number.
///
bool is_number(const std::string &s)
{
  try
  {
    std::stod(s);
  }
  catch(std::invalid_argument &)
  {
    return false;
  }

  return true;
}
}  // namespace

///
/// New empty data instance.
///
src_data::src_data() : data(training), classes_map_(), header_(),
                       categories_(), datasets_(npos)
{
  Ensures(debug());
}

///
/// \param[in] filename name of the file containing the learning collection.
/// \param[in] ft a filter and transform function.
///
/// \brief New data instance containing the learning collection from
///        `filename`.
///
src_data::src_data(const std::string &filename, filter_hook_t ft) : data()
{
  Expects(!filename.empty());

  load(filename, ft);

  Ensures(debug());
}

///
/// \brief Resets the object.
///
void src_data::clear()
{
  *this = src_data();
}

///
/// \return reference to the first element of the active dataset.
///
src_data::iterator src_data::begin()
{
  return begin(active_dataset());
}

///
/// \return a constant reference to the first element of the active dataset.
///
src_data::const_iterator src_data::begin() const
{
  return begin(active_dataset());
}

///
/// \param[in] d a dataset (training / validation / test set).
/// \return reference to the first element of dataset `d`.
///
src_data::iterator src_data::begin(dataset_t d)
{
  Expects(d != npos);
  return datasets_[d].begin();
}

///
/// \param[in] d a dataset (training / validation / test set).
/// \return a constant reference to the first element dataset `d`.
///
src_data::const_iterator src_data::begin(dataset_t d) const
{
  Expects(d != npos);
  return datasets_[d].begin();
}

///
/// \return a reference to the sentinel element of the active dataset.
///
src_data::iterator src_data::end()
{
  return end(active_dataset());
}

///
/// \return a constant reference to the sentinel element of the active dataset.
///
src_data::const_iterator src_data::end() const
{
  return end(active_dataset());
}

///
/// \param[in] d a dataset (training / validation / test set).
/// \return a reference to the sentinel element of dataset `d`
///
src_data::iterator src_data::end(dataset_t d)
{
  Expects(d != npos);
  return datasets_[d].end();
}

///
/// \param[in] d a dataset (training / validation / test set).
/// \return a constant reference to the sentinel element of dataset `d`.
///
src_data::const_iterator src_data::end(dataset_t d) const
{
  Expects(d != npos);
  return datasets_[d].end();
}

///
/// \param[in] d a dataset (training / validation / test set).
/// \return the size of the dataset `d`.
///
std::size_t src_data::size(dataset_t d) const
{
  return datasets_[d].size();
}

///
/// \return the size of the active dataset.
///
std::size_t src_data::size() const
{
  return size(active_dataset());
}

bool src_data::has(dataset_t d) const
{
  return size(d);
}

///
/// \return a const reference to the set of categories associated with the
///         dataset.
///
const category_set &src_data::categories() const
{
  return categories_;
}

///
/// \param[in] i index of a column.
/// \return a const reference to the i-th column of the dataset.
///
const src_data::column &src_data::get_column(unsigned i) const
{
  Expects(i < columns());
  return header_[i];
}

void src_data::move_append(dataset_t src, dataset_t dst)
{
  Expects(src != npos);
  Expects(dst != npos);
  Expects(src != dst);

  if (datasets_[dst].empty())
    std::swap(datasets_[src], datasets_[dst]);
  else
  {
    std::move(begin(src), end(src), std::back_inserter(datasets_[dst]));
    datasets_[src].clear();
  }
}

void src_data::move_append(dataset_t src, dataset_t dst, std::size_t n)
{
  Expects(src != npos);
  Expects(dst != npos);
  Expects(src != dst);
  Expects(n < size(src));

  const auto from_src(std::next(end(src), -static_cast<std::ptrdiff_t>(n)));

  std::move(from_src, end(src), std::back_inserter(datasets_[dst]));

  datasets_[src].erase(from_src, end(src));
}

///
/// \return number of columns of the dataset.
///
/// \note
/// data class supports just one output for every instance, so, if
/// the dataset is not empty: `variables() + 1 == columns()`.
///
unsigned src_data::columns() const
{
  Expects(datasets_[active_dataset()].empty() ||
          variables() + 1 == header_.size());

  return static_cast<unsigned>(header_.size());
}

///
/// \return number of classes of the problem (== 0 for a symbolic regression
///         problem, > 1 for a classification problem).
///
class_t src_data::classes() const
{
  return static_cast<class_t>(classes_map_.size());
}

///
/// \return input vector dimension.
///
/// \note data class supports just one output for every instance, so, if
/// the dataset is not empty, `variables() + 1 == columns()`.
///
unsigned src_data::variables() const
{
  const auto n(datasets_[active_dataset()].empty()
               ? 0u : static_cast<unsigned>(begin()->input.size()));

  Ensures(datasets_[active_dataset()].empty() || n + 1 == header_.size());
  return n;
}

///
/// \brief Appends the given element to the end of the active dataset
///
/// \param[in] e the value of the element to append
///
void src_data::push_back(const example &e)
{
  datasets_[active_dataset()].push_back(e);
}

///
/// \param[in] label name of a class of the learning collection.
/// \return the (numerical) tag associated with class `label`.
///
class_t src_data::encode(const std::string &label)
{
  if (classes_map_.find(label) == classes_map_.end())
  {
    const auto n(static_cast<category_t>(classes_map_.size()));
    classes_map_[label] = n;
  }

  return classes_map_[label];
}

///
/// \param[in] i the encoded (src_data::encode()) value of a class.
/// \return the name of the class encoded with the `unsigned i` (or an
///         empty string if such class cannot be find).
///
/// \note
/// Boost Bitmap could be used to speed up the search in `classes_map_`, but
/// to date speed isn't an issue.
///
std::string src_data::class_name(class_t i) const
{
  for (const auto &p : classes_map_)
    if (p.second == i)
      return p.first;

  return "";
}

///
/// \param[in] c1 a category.
/// \param[in] c2 a category.
///
/// Swap categories `c1` and `c2`, updating the `header_` and `categories_`
/// vector.
///
void src_data::swap_category(category_t c1, category_t c2)
{
  const auto n_col(columns());

  assert(c1 < n_col);
  assert(c2 < n_col);

  categories_.swap(c1, c2);

  for (auto i(decltype(n_col){0}); i < n_col; ++i)
    if (header_[i].category_id == c1)
      header_[i].category_id = c2;
    else if (header_[i].category_id == c2)
      header_[i].category_id = c1;
}

///
/// \param[in] v a record containing the example (all fields, i.e. feature, are
///              `std::string`.
/// \param[in] classification is this a classification task?
/// \param[in] add_label should we automatically add labels for text-features?
/// \return `v` converted to `example` type.
///
/// \remark
/// When `add_label` is `true` the function can have side-effects (changing
/// the set of labels associated to a category).
///
src_data::example src_data::to_example(const std::vector<std::string> &v,
                                       bool classification, bool add_label)
{
  Expects(v.size());

  example ret;

  unsigned index(0);
  for (const auto &feature : v)
  {
    const auto categ(header_[index].category_id);
    const auto domain(categories_.find(categ).domain);

    if (index)  // input value
    {
      ret.input.push_back(convert(feature, domain));

      if (add_label && domain == domain_t::d_string)
        categories_.add_label(categ, feature);
    }
    else if (!feature.empty())  // output value (not empty)
    {
      // Strings could be used as label for classes, but integers
      // are simpler and faster to manage (arrays instead of maps).
      if (classification)
        ret.output = encode(feature);
      else
        ret.output = convert(feature, domain);
    }

    ++index;
  }

  // The output class/value can be empty only for the test-set.
  Ensures(active_dataset() == test || !ret.output.empty());

  return ret;
}

///
/// \brief Loads the content of `filename` into the active dataset
/// \param[in] filename the xrff file.
/// \param[in] ft a filter and transform function.
/// \return number of lines parsed (0 in case of errors).
///
/// \note
/// An XRFF (eXtensible attribute-Relation File Format) file describes a list
/// of instances sharing a set of attributes.
/// The original format is defined in http://weka.wikispaces.com/XRFF, we
/// extend it with an additional (non-standard) feature: attribute category.
///
///     <attribute name="vehicle length" type="numeric" category="length" />
///     <attribute name="vehicle width" type="numeric" category="length" />
///     <attribute name="vehicle weight" type="numeric" category="weight" />
///
/// This feature is used to constrain the search (Strongly Typed Genetic
/// Programming).
///
/// \post
/// * `header_[0]` is the output column (it contains informations about
///   problem's output);
/// * `category(0)` is the output category (for symbolic regression
///   problems it's the output type of the xrff file, for classification
///   problems it's the `numeric` type).
///
/// \warning
/// To date:
/// * we don't support compressed XRFF files;
/// * XRFF files cannot be used to load test set (problems with missing
///   output column and possible column category redefinition).
///
/// \note Test set examples can have an empty output value.
///
std::size_t src_data::load_xrff(const std::string &filename, filter_hook_t ft)
{
  Expects(active_dataset() == training);

  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_NO_ERROR)
    return 0;

  unsigned n_output(0);
  bool classification(false);

  // Iterate over `dataset.header.attributes` selection and store all found
  // attributes in the header vector.
  tinyxml2::XMLHandle handle(&doc);
  auto *attributes = handle.FirstChildElement("dataset")
                           .FirstChildElement("header")
                           .FirstChildElement("attributes").ToElement();
  if (!attributes)
    return 0;

  for (auto *attribute = attributes->FirstChildElement("attribute");
       attribute;
       attribute = attribute->NextSiblingElement("attribute"))
  {
    column a;

    const char *s = attribute->Attribute("name");
    if (s)
      a.name = s;

    // One can define which attribute should act as output value via the
    // `class="yes"` attribute in the attribute specification of the header.
    const bool output(attribute->Attribute("class", "yes"));

    s = attribute->Attribute("type");
    std::string xml_type(s ? s : "");

    s = attribute->Attribute("category");
    std::string category_name(s ? s : xml_type);

    if (output)
    {
      ++n_output;

      // We can manage only one output column.
      if (n_output > 1)
        return 0;

      // For classification problems we use discriminant functions, so the
      // actual output type is always numeric.
      classification = (xml_type == "nominal" || xml_type == "string");
      if (classification)
      {
        xml_type = "numeric";
        category_name = "numeric";
      }
    }

    a.category_id = categories_.insert({category_name, from_weka(xml_type),
                                        {}});

    // Store label1... labelN.
    if (xml_type == "nominal")
      for (auto *l = attribute->FirstChildElement("label");
           l;
           l = l->NextSiblingElement("label"))
      {
        const std::string label(l->GetText() ? l->GetText() : "");
        categories_.add_label(a.category_id, label);
      }

    // Output column is always the first one.
    if (output)
      header_.insert(header_.begin(), a);
    else
      header_.push_back(a);
  }

  // XRFF needs informations about the columns.
  if (!columns())
    return 0;

  // If no output column is specified the default XRFF output column is the
  // last one (and it's the first element of the `header_` vector).
  if (n_output == 0)
  {
    header_.insert(header_.begin(), header_.back());
    header_.pop_back();
  }

  // Category 0 is the output category.
  swap_category(category_t(0), header_[0].category_id);

  auto *instances = handle.FirstChildElement("dataset")
                          .FirstChildElement("body")
                          .FirstChildElement("instances").ToElement();
  if (!instances)
    return 0;

  for (auto *i = instances->FirstChildElement("instance");
       i;
       i = i->NextSiblingElement("instance"))
  {
    std::vector<std::string> record;
    for (auto *v = i->FirstChildElement("value");
         v;
         v = v->NextSiblingElement("value"))
      record.push_back(v->GetText() ? v->GetText() : "");

    if (ft && ft(&record) == false)
      continue;

    const auto instance(to_example(record, classification, false));

    if (instance.input.size() + 1 == columns())
      push_back(instance);
    else
      print.warning("Malformed example ", size(), " skipped");

  }

  return debug() ? size() : static_cast<std::size_t>(0);
}

///
/// \param[in] filename the csv file.
/// \param[in] ft a filter and transform function.
/// \return number of lines parsed (0 in case of errors).
///
/// We follow the Google Prediction API convention
/// (https://developers.google.com/prediction/docs/developer-guide?hl=it#data-format):
/// * NO HEADER ROW is allowed;
/// * only one example is allowed per line. A single example cannot contain
///   newlines and cannot span multiple lines.
///   Note than CSV standard (e.g.
///   http://en.wikipedia.org/wiki/Comma-separated_values) allows for the
///   newline character `\n` to be part of a csv field if the field is
///   surrounded by quotes;
/// * columns are separated by commas. Commas inside a quoted string aren't
///   column delimiters;
/// * THE FIRST COLUMN REPRESENTS THE VALUE (numeric or string) for that
///   example. If the first column is numeric, this model is a REGRESSION
///   model; if the first column is a string, it's a CATEGORIZATION
///   (i.e. classification) model. Each column must describe the same kind of
///   information for that example;
/// * the column order of features in the table does not weight the results.
///   The first feature is not weighted any more than the last;
/// * as a best practice, remove punctuation (other than apostrophes) from
///   your data. This is because commas, periods and other punctuation
///   rarely add meaning to the training data, but are treated as meaningful
///   elements by the learning engine. For example, "end." is not matched to
///   "end";
/// * TEXT STRINGS:
///   * place double quotes around all text strings;
///   * text matching is case-sensitive: "wine" is different from "Wine.";
///   * if a string contains a double quote, the double quote must be escaped
///     with another double quote, for example:
///     "sentence with a ""double"" quote inside";
/// * NUMERIC VALUES:
///   * both integer and decimal values are supported;
///   * numbers in quotes without whitespace will be treated as numbers, even
///     if they are in quotation marks. Multiple numeric values within
///     quotation marks in the same field will be treated as a string. For
///     example:
///       Numbers: "2", "12", "236"
///       Strings: "2 12", "a 23"
///
/// \note Test set can have an empty output value.
///
std::size_t src_data::load_csv(const std::string &filename, filter_hook_t ft)
{
  std::ifstream from(filename);
  if (!from)
    return 0;

  bool classification(false), format(columns());

  for (auto record : csv_parser(from).filter_hook(ft))
  {
    const auto fields(record.size());

    // If we don't know the dataset format, the first line will be used to
    // learn it.
    if (!format)
    {
      classification = !is_number(record[0]);

      header_.reserve(fields);

      for (auto field(decltype(fields){0}); field < fields; ++field)
      {
        assert(!size());  // If we have data then data format must be known

        std::string s_domain(is_number(record[field])
                             ? "numeric"
                             : "string" + std::to_string(field));

        // For classification problems we use discriminant functions, so the
        // actual output type is always numeric.
        if (field == 0 && classification)
          s_domain = "numeric";

        const domain_t domain(s_domain == "numeric" ? domain_t::d_double
                                                    : domain_t::d_string);

        const category_t tag(categories_.insert({s_domain, domain, {}}));

        header_.push_back({"", tag});
      }

      format = true;
    }  // if (!format)

    if (fields != columns())  // skip lines with wrong number of columns
      continue;

    const auto instance(to_example(record, classification, true));

    if (instance.input.size() + 1 == columns())
      push_back(instance);
    else
      print.warning("Malformed line ", size(), " skipped");
  }

  return debug() ? size() : static_cast<std::size_t>(0);
}

///
/// \param[in] f name of the file containing the data set.
/// \param[in] ft a filter and transform function.
/// \return number of lines parsed (0 in case of errors).
///
/// Loads the content of `f` into the active dataset.
///
/// \warning
/// * Training/valutation set must be loaded before test set.
/// * Before changing problem the data object should be clear.
///
/// So:
///
///     dataset(training);
///     load("training.csv");
///     dataset(test);
///     load("test.csv");
///     ...
///     clear();
///     dataset(training);
///     load("training2.csv");
///     ...
///
/// \note
/// Test set can have an empty output value.
///
std::size_t src_data::load(const std::string &f, filter_hook_t ft)
{
  auto ends_with =
    [](const std::string &name, const std::string &ext)
    {
      return ext.length() <= name.length() &&
             std::equal(ext.rbegin(), ext.rend(), name.rbegin());
    };

  const bool xrff(ends_with(f, ".xrff") || ends_with(f, ".xml"));

  return xrff ? load_xrff(f, ft) : load_csv(f, ft);
}

///
/// \return `true` if the current dataset is empty.
///
bool src_data::operator!() const
{
  return size() == 0;
}

///
/// \return `true` if the object passes the internal consistency check.
///
bool src_data::debug() const
{
  const auto cl_size(classes());
  // If this is a classification problem then there should be at least two
  // classes.
  if (cl_size == 1)
    return false;

  for (const auto &d : datasets_)
    if (!d.empty() && &d != &datasets_[test])
    {
      const auto in_size(d.begin()->input.size());

      for (const auto &e : d)
      {
        if (e.input.size() != in_size)
          return false;

        if (cl_size && e.tag() >= cl_size)
          return false;
      }
    }

  return true;
}

///
/// \param[in] n the name of a weka domain.
/// \return the internel id of the weka-domain `n` (`d_void` if it's
///         unknown or not managed).
///
domain_t src_data::from_weka(const std::string &n)
{
  static const std::map<const std::string, domain_t> map(
  {
    // This type is vita-specific (not standard).
    {"boolean", domain_t::d_bool},

    {"integer", domain_t::d_int},

    // Real and numeric are treated as double precision number (d_double).
    {"numeric", domain_t::d_double},
    {"real", domain_t::d_double},

    // Nominal values are defined by providing a list of possible values.
    {"nominal", domain_t::d_string},

    // String attributes allow us to create attributes containing arbitrary
    // textual values. This is very useful in text-mining applications.
    {"string", domain_t::d_string}

    // {"date", ?}, {"relational", ?}
  });

  const auto &i(map.find(n));
  return i == map.end() ? domain_t::d_void : i->second;
}
}  // namespace vita
