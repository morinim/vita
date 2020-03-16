/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>

#include "kernel/src/dataframe.h"
#include "kernel/exceptions.h"
#include "kernel/log.h"
#include "kernel/random.h"
#include "kernel/symbol.h"

#include "utility/csv_parser.h"

#include "tinyxml2/tinyxml2.h"

namespace vita
{

namespace
{
// \param[in] s the string to be converted
// \param[in] d what type should `s` be converted in?
// \return      the converted data or an empty value if no conversion can be
//              applied
//
// `convert("123.1", sym_double) == any(123.1f)`
value_t convert(const std::string &s, domain_t d)
{
  switch (d)
  {
  case d_int:    return std::stoi(s);
  case d_double: return std::stod(s);
  case d_string: return            s;
  default:       return           {};
  }
}

// \param[in] s the string to be tested
// \return      `true` if `s` contains a number
bool is_number(const std::string &s)
{
  char *end;
  strtod(s.c_str(), &end);  // if no conversion can be performed, ​`end` is set
                            // to `s.c_str()`
  return end != s.c_str() && *end == '\0';
}
}  // unnamed namespace

///
/// \param[in] n the name of a weka domain
/// \return      the internel id of the weka-domain `n` (`d_void` if it's
///              unknown or not managed)
///
domain_t from_weka(const std::string &n)
{
  static const std::map<const std::string, domain_t> map(
  {
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

///
/// New empty data instance.
///
dataframe::dataframe() : classes_map_(), header_(), categories_(), dataset_()
{
  Ensures(debug());
}

///
/// New dataframe instance containing the learning collection from a stream.
///
/// \param[in] is input stream
/// \param[in] ft a filter and transform function
///
/// \remark Data from the input stream must be in CSV format.
///
dataframe::dataframe(std::istream &is, filter_hook_t ft)
  : dataframe()
{
  Expects(is.good());
  read_csv(is, ft);
  Ensures(debug());
}

///
/// New datafame instance containing the learning collection from a file.
///
/// \param[in] filename name of the file containing the learning collection
///                     (CSV / XRFF format)
/// \param[in] ft       a filter and transform function
///
dataframe::dataframe(const std::string &filename, filter_hook_t ft)
  : dataframe()
{
  Expects(!filename.empty());
  read(filename, ft);
  Ensures(debug());
}

///
/// Removes all elements from the container.
///
/// Invalidates any references, pointers or iterators referring to contained
/// examples. Any past-the-end iterators are also invalidated.
///
/// Leaves the associated metadata unchanged.
///
void dataframe::clear()
{
  dataset_.clear();
}

///
/// \return reference to the first element of the active dataset
///
dataframe::iterator dataframe::begin()
{
  return dataset_.begin();
}

///
/// \return a constant reference to the first element of the dataset
///
dataframe::const_iterator dataframe::begin() const
{
  return dataset_.begin();
}

///
/// \return a reference to the sentinel element of the active dataset
///
dataframe::iterator dataframe::end()
{
  return dataset_.end();
}

///
/// \return a constant reference to the sentinel element of the active dataset
///
dataframe::const_iterator dataframe::end() const
{
  return dataset_.end();
}

///
/// \return the size of the active dataset
///
std::size_t dataframe::size() const
{
  return dataset_.size();
}

///
/// \return `true` if the dataframe is empty
///
bool dataframe::empty() const
{
  return size() == 0;
}

///
/// \return a const reference to the set of categories associated with the
///         dataset
///
const category_set &dataframe::categories() const
{
  return categories_;
}

///
/// \param[in] i index of a column
/// \return      a const reference to the `i`-th column of the dataset
///
const dataframe::column &dataframe::get_column(unsigned i) const
{
  Expects(i < columns());
  return header_[i];
}

///
/// \return number of columns of the dataset
///
/// \note
/// `dataframe` supports just one output for every instance, so, if the dataset
/// is not empty: `variables() + 1 == columns()`.
///
unsigned dataframe::columns() const
{
  Expects(dataset_.empty() || variables() + 1 == header_.size());

  return static_cast<unsigned>(header_.size());
}

///
/// \return number of classes of the problem (`== 0` for a symbolic regression
///         problem, `> 1` for a classification problem)
///
class_t dataframe::classes() const
{
  return static_cast<class_t>(classes_map_.size());
}

///
/// \return input vector dimension
///
/// \note data class supports just one output for every instance, so, if the
///       dataset is not empty, `variables() + 1 == columns()`.
///
unsigned dataframe::variables() const
{
  const auto n(empty() ? 0u : static_cast<unsigned>(begin()->input.size()));

  Ensures(empty() || n + 1 == header_.size());
  return n;
}

///
/// Appends the given element to the end of the active dataset.
///
/// \param[in] e the value of the element to append
///
void dataframe::push_back(const example &e)
{
  dataset_.push_back(e);
}

///
/// \param[in] label name of a class of the learning collection
/// \return          the (numerical) value associated with class `label`
///
class_t dataframe::encode(const std::string &label)
{
  if (classes_map_.find(label) == classes_map_.end())
  {
    const auto n(classes());
    classes_map_[label] = n;
  }

  return classes_map_[label];
}

///
/// \param[in] v              a container for the example (features encoded as
///                           `std::string`s)
/// \param[in] classification is this a classification task?
/// \param[in] add_label      should we automatically add labels for
///                           text-features?
/// \return                   `v` converted to `example` type
///
/// \remark
/// When `add_label` is `true` the function can have side-effects (changing
/// the set of labels associated to a category).
///
dataframe::example dataframe::to_example(const record_t &v,
                                         bool classification, bool add_label)
{
  Expects(v.size());

  example ret;

  unsigned index(0);
  for (const auto &feature : v)
  {
    const auto categ(header_[index].category_id);
    const auto domain(categories_[categ].domain);

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

  return ret;
}

///
/// \param[in] r input record (an example in raw format)
/// \return      `true` for a correctly converted/imported record
///
bool dataframe::read_record(const record_t &r)
{
  const bool classification(!is_number(r.front()));

  const auto fields(r.size());

  // If we don't know the dataset format yet, the current record is used to
  // discover it.
  if (const bool format = columns(); !format)
  {
    header_.reserve(fields);

    for (std::size_t field(0); field < fields; ++field)
    {
      assert(!size());  // if we have data then data format must be known

      std::string s_domain(is_number(r[field])
                           ? "numeric" : "string" + std::to_string(field));

      // For classification problems we use discriminant functions, so the
      // actual output type is always numeric.
      if (field == 0 && classification)
        s_domain = "numeric";

      const domain_t domain(s_domain == "numeric"
                            ? domain_t::d_double : domain_t::d_string);

      const category_t tag(categories_.insert({s_domain, domain, {}}));

      header_.push_back({"", tag});
    }
  }  // if (!format)

  if (fields != columns())  // skip lines with wrong number of columns
  {
    vitaWARNING << "Malformed exampled skipped";
    return false;
  }

  const auto instance(to_example(r, classification, true));
  push_back(instance);

  return true;
}

///
/// \param[in] i the encoded (dataframe::encode()) value of a class
/// \return      the name of the class encoded by `i` (or an empty string if
///              such class cannot be find)
///
/// \note
/// Boost Bitmap could be used to speed up the search in `classes_map_`, but
/// to date speed isn't an issue.
///
std::string dataframe::class_name(class_t i) const
{
  for (const auto &p : classes_map_)
    if (p.second == i)
      return p.first;

  return "";
}

///
/// Swap two categories of the dataframe.
///
/// \param[in] c1 a category
/// \param[in] c2 a category
///
void dataframe::swap_category(category_t c1, category_t c2)
{
  const auto n_col(columns());

  Expects(c1 < n_col);
  Expects(c2 < n_col);

  categories_.swap(c1, c2);

  for (auto i(decltype(n_col){0}); i < n_col; ++i)
    if (header_[i].category_id == c1)
      header_[i].category_id = c2;
    else if (header_[i].category_id == c2)
      header_[i].category_id = c1;
}

///
/// Loads a XRFF file from a file into the dataframe.
///
/// \param[in] filename the xrff filename
/// \param[in] ft       a "filter and transform" function
/// \return             number of lines parsed (`0` in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
/// \see `dataframe::load_xrff(tinyxml2::XMLDocument &)` for details.
///
std::size_t dataframe::read_xrff(const std::string &filename, filter_hook_t ft)
{
  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("XRFF data file format error");

  return read_xrff(doc, ft);
}

///
/// Loads a XRFF file from a stream into the dataframe.
///
/// \param[in] in the xrff stream
/// \param[in] ft a "filter and transform" function
/// \return       number of lines parsed (`0` in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
/// \see `dataframe::read_xrff(tinyxml2::XMLDocument &)` for details.
///
std::size_t dataframe::read_xrff(std::istream &in, filter_hook_t ft)
{
  std::ostringstream ss;
  ss << in.rdbuf();

  tinyxml2::XMLDocument doc;
  if (doc.Parse(ss.str().c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("XRFF data file format error");

  return read_xrff(doc, ft);
}

///
/// Loads a XRFF document into the active dataset.
///
/// \param[in] filename the xrff file
/// \param[in] ft       a filter and transform function
/// \return             number of lines parsed (0 in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
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
/// \warning
/// To date:
/// * we don't support compressed XRFF files;
/// * XRFF files cannot be used to load test set (problems with missing
///   output column and possible column category redefinition).
///
/// \note Test set examples can have an empty output value.
///
std::size_t dataframe::read_xrff(tinyxml2::XMLDocument &doc, filter_hook_t ft)
{
  // Iterate over `dataset.header.attributes` selection and store all found
  // attributes in the header vector.
  tinyxml2::XMLHandle handle(&doc);
  auto *attributes = handle.FirstChildElement("dataset")
                           .FirstChildElement("header")
                           .FirstChildElement("attributes").ToElement();
  if (!attributes)
    throw exception::data_format("Missing `attributes` element in XRFF file");

  clear();

  unsigned n_output(0);
  bool classification(false);

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
        throw exception::data_format("Multiple output columns in XRFF file");

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

  // XRFF needs information about the columns.
  if (!columns())
    throw exception::data_format("Missing column information in XRFF file");

  // If no output column is specified the default XRFF output column is the
  // last one (and it's the first element of the `header_` vector).
  if (n_output == 0)
  {
    header_.insert(header_.begin(), header_.back());
    header_.pop_back();
  }

  // Category 0 is the output category.
  swap_category(0, header_[0].category_id);

  auto *instances = handle.FirstChildElement("dataset")
                          .FirstChildElement("body")
                          .FirstChildElement("instances").ToElement();
  if (!instances)
    throw exception::data_format("Missing `instances` element in XRFF file");

  for (auto *i = instances->FirstChildElement("instance");
       i;
       i = i->NextSiblingElement("instance"))
  {
    record_t record;
    for (auto *v = i->FirstChildElement("value");
         v;
         v = v->NextSiblingElement("value"))
      record.push_back(v->GetText() ? v->GetText() : "");

    if (ft && ft(record) == false)
      continue;

    const auto instance(to_example(record, classification, false));

    if (instance.input.size() + 1 == columns())
      push_back(instance);
    else
      vitaWARNING << "Malformed example " << size() << " skipped";
  }

  return debug() ? size() : static_cast<std::size_t>(0);
}

///
/// Loads a CSV file into the active dataset.
///
/// \param[in] filename the csv filename
/// \param[in] ft       a filter and transform function
/// \return             number of lines parsed (0 in case of errors)
///
/// \exception std::runtime_error    cannot read CSV data file
///
/// \see `dataframe::load_csv(const std::string &)` for details.
///
std::size_t dataframe::read_csv(const std::string &filename, filter_hook_t ft)
{
  std::ifstream in(filename);
  if (!in)
    throw std::runtime_error("Cannot read CSV data file");

  return read_csv(in, ft);
}

///
/// Loads a CSV file into the active dataset.
///
/// \param[in] from the csv stream
/// \param[in] ft   a filter and transform function
/// \return         number of lines parsed (0 in case of errors)
///
/// \exception exception::insufficient_data empty / undersized data file
///
/// General conventions:
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
///   - place double quotes around all text strings;
///   - text matching is case-sensitive: "wine" is different from "Wine.";
///   - if a string contains a double quote, the double quote must be escaped
///     with another double quote, for example:
///     "sentence with a ""double"" quote inside";
/// * NUMERIC VALUES:
///   - both integer and decimal values are supported;
///   - numbers in quotes without whitespace will be treated as numbers, even
///     if they are in quotation marks. Multiple numeric values within
///     quotation marks in the same field will be treated as a string. For
///     example:
///       Numbers: "2", "12", "236"
///       Strings: "2 12", "a 23"
///
/// \note Test set can have an empty output value.
///
std::size_t dataframe::read_csv(std::istream &from, filter_hook_t ft)
{
  clear();

  for (auto record : csv_parser(from).filter_hook(ft))
    read_record(record);

  if (!debug() || !size())
    throw exception::insufficient_data("Empty / undersized CSV data file");

  return size();
}

///
/// Loads the content of a file into the active dataset.
///
/// \param[in] f  name of the file containing the data set (CSV / XRFF format)
/// \param[in] ft a filter and transform function
/// \return       number of lines parsed
///
/// \exception std::invalid_argument missing dataset file name
///
/// \note Test set can have an empty output value.
///
std::size_t dataframe::read(const std::string &f, filter_hook_t ft)
{
  if (trim(f).empty())
    throw std::invalid_argument("Missing dataset filename");

  auto ends_with(
    [](const std::string &name, const std::string &ext)
    {
      return ext.length() <= name.length()
        && std::equal(ext.rbegin(), ext.rend(), name.rbegin());
    });

  const bool xrff(ends_with(f, ".xrff") || ends_with(f, ".xml"));

  return xrff ? read_xrff(f, ft) : read_csv(f, ft);
}

///
/// \return `true` if the current dataset is empty
///
bool dataframe::operator!() const
{
  return size() == 0;
}

///
/// Removes specified elements from the dataframe.
///
/// \param[in] first first element of the range
/// \param[in] last  end of the range
/// \return          iterator following the last removed element
///
dataframe::iterator dataframe::erase(iterator first, iterator last)
{
  return dataset_.erase(first, last);
}

///
/// \return `true` if the object passes the internal consistency check
///
bool dataframe::debug() const
{
  const auto cl_size(classes());
  // If this is a classification problem then there should be at least two
  // classes.
  if (cl_size == 1)
    return false;

  if (!empty())
  {
    const auto in_size(begin()->input.size());

    for (const auto &e : *this)
    {
      if (e.input.size() != in_size)
        return false;

      if (cl_size && label(e) >= cl_size)
        return false;
    }
  }

  return true;
}

}  // namespace vita
