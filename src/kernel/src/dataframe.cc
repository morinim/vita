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
// `convert("123.1", d_double) == value_t(123.1f)`
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
    {"real",    domain_t::d_double},

    // Nominal values are defined by providing a list of possible values.
    {"nominal", domain_t::d_string},

    // String attributes allow us to create attributes containing arbitrary
    // textual values. This is very useful in text-mining applications.
    {"string",  domain_t::d_string}

    // {"date", ?}, {"relational", ?}
  });

  const auto &i(map.find(n));
  return i == map.end() ? domain_t::d_void : i->second;
}

///
/// Arrange the object in the empty starting state.
///
dataframe::columns_info::columns_info() : cols_()
{
}

void dataframe::columns_info::push_back(const column_info &v)
{
  cols_.push_back(v);
}

void dataframe::columns_info::push_front(const column_info &v)
{
  cols_.insert(begin(), v);
}

///
/// Given an example compiles information about the columns of the dataframe.
///
/// \param[in] r            a record containing an example
/// \param[in] header_first `true` if the first example contains the header
/// \param[in] categories   set of categories (updated according to `r`)
///
/// The function can be called multiple times to incrementally collect
/// information from different examples.
///
/// When `header_first` is `true` the first example is used to gather the names
/// of the columns and successive example contribute to determine the category
/// of each column.
///
/// \remark The function assumes columns `0` as the output column.
///
template<>
void dataframe::columns_info::build(const dataframe::record_t &r,
                                    bool header_first,
                                    category_set &categories)
{
  Expects(r.size());

  // When available returns the name of the column; otherwise returns a
  // surrogate, unique name (e.g. `column123`).
  const auto col_name(
    [this](std::size_t idx)
    {
      if (cols_[idx].name.empty())
        return idx ? "column" + std::to_string(idx) : "output";

      return cols_[idx].name;
    });

  // Sets the category associated to a column.
  const auto set_col_category(
    [&](std::size_t idx)
    {
      if (cols_[idx].category_id != undefined_category)
        return;

      const std::string &value(trim(r[idx]));
      if (value.empty())
        return;  // value not available

      // For numbers we aren't adopting strong typing by default. So values
      // from distinct numerical columns can be mixed (since they're mapped to
      // the same domain / category).
      std::string domain_name(is_number(value) ? "numeric" : col_name(idx));

      // For classification tasks we use discriminant functions and the actual
      // output type is always numeric.
      const bool output(!r.front().empty());
      const bool classification(output && !is_number(r.front()));
      if (idx == 0 && classification)
        domain_name = "numeric";

      const domain_t domain(domain_name == "numeric" ? d_double : d_string);

      cols_[idx].category_id = categories.insert({domain_name, domain, {}});
    });

  const auto fields(r.size());

  if (cols_.empty())
  {
    cols_.reserve(fields);

    if (header_first)  // first line contains the names of the columns
    {
      vitaDEBUG << "Reading columns header";
      for (const auto &name : r)
        cols_.push_back({name, undefined_category});
      return;
    }
    else
      std::fill_n(std::back_inserter(cols_), fields, column_info());
  }

  assert(size() == r.size());

  for (std::size_t field(0); field < fields; ++field)
    set_col_category(field);
}

///
/// Swaps two categories of the columns_info object.
///
/// \param[in] c1 a category
/// \param[in] c2 a category
///
void dataframe::columns_info::swap_category(category_t c1, category_t c2,
                                            dataframe_only)
{
  for (auto &c : cols_)
    if (c.category_id == c1)
      c.category_id = c2;
    else if (c.category_id == c2)
      c.category_id = c1;
}

///
/// New empty data instance.
///
dataframe::dataframe() : columns(), classes_map_(), categories_(), dataset_()
{
  Ensures(debug());
}

///
/// New dataframe instance containing the learning collection from a stream.
///
/// \param[in] is input stream
/// \param[in] p  additional, optional, parameters (see `params` structure)
///
/// \remark Data from the input stream must be in CSV format.
///
dataframe::dataframe(std::istream &is, const params &p) : dataframe()
{
  Expects(is.good());
  read_csv(is, p);
  Ensures(debug());
}
dataframe::dataframe(std::istream &is) : dataframe(is, {}) {}


///
/// New datafame instance containing the learning collection from a file.
///
/// \param[in] fn name of the file containing the learning collection (CSV /
///               XRFF format)
/// \param[in] p  additional, optional, parameters (see `params` structure)
///
dataframe::dataframe(const std::filesystem::path &fn, const params &p)
  : dataframe()
{
  Expects(!fn.empty());
  read(fn, p);
  Ensures(debug());
}
dataframe::dataframe(const std::filesystem::path &fn) : dataframe(fn, {}) {}

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
/// Returns a constant reference to the first element in the dataframe.
///
/// \return a constant reference to the first element int the dataframe
///
/// \remark Calling `front` on an empty dataframe is undefined.
///
dataframe::value_type dataframe::front() const
{
  return dataset_.front();
}

///
/// Returns a reference to the first element in the dataframe.
///
/// \return a reference to the first element in the dataframe
///
/// \remark Calling `front` on an empty dataframe is undefined.
///
dataframe::value_type &dataframe::front()
{
  return dataset_.front();
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
///       dataset is not empty, `variables() + 1 == columns.size()`.
///
unsigned dataframe::variables() const
{
  const auto n(empty() ? 0u : static_cast<unsigned>(begin()->input.size()));

  Ensures(empty() || n + 1 == columns.size());
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
/// \param[in] add_label      should we automatically add instances for
///                           text-features?
/// \return                   `v` converted to `example` type
///
/// \remark
/// When `add_label` is `true` the function can have side-effects (changing
/// the set of labels associated with a category).
///
dataframe::example dataframe::to_example(const record_t &v, bool add_label)
{
  Expects(v.size());
  Expects(v.size() == columns.size());

  const bool output(!v.front().empty());
  const bool classification(output && !is_number(v.front()));

  example ret;

  std::size_t index(0);
  for (const auto &feature : v)
  {
    const auto categ(columns[index].category_id);
    const auto domain(categories_[categ].domain);

    if (index)  // input value
    {
      ret.input.push_back(convert(feature, domain));

      if (add_label && domain == d_string)
        categories_.add_label(categ, feature);
    }
    else if (!feature.empty())  // output value (not empty)
    {
      // Strings could be used as label for classes, but integers
      // are simpler and faster to manage (arrays instead of maps).
      if (classification)
        ret.output = static_cast<D_INT>(encode(feature));
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
  Expects(r.size());

  if (r.size() != columns.size())  // skip lines with wrong number of columns
  {
    vitaWARNING << "Malformed exampled skipped";
    return false;
  }

  const auto instance(to_example(r, true));
  push_back(instance);

  return true;
}

///
/// \param[in] i the encoded (dataframe::encode()) value of a class
/// \return      the name of the class encoded by `i` (or an empty string if
///              such class cannot be find)
///
std::string dataframe::class_name(class_t i) const
{
  for (const auto &p : classes_map_)
    if (p.second == i)
      return p.first;

  return {};
}

///
/// Swaps two categories of the dataframe.
///
/// \param[in] c1 a category
/// \param[in] c2 a category
///
void dataframe::swap_category(category_t c1, category_t c2)
{
  categories_.swap(c1, c2);
  columns.swap_category(c1, c2, {});
}

///
/// Loads a XRFF file from a file into the dataframe.
///
/// \param[in] fn the xrff filename
/// \param[in] p  additional, optional, parameters (see `params` structure)
/// \return       number of lines parsed (`0` in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
/// \see `dataframe::load_xrff(tinyxml2::XMLDocument &)` for details.
///
std::size_t dataframe::read_xrff(const std::filesystem::path &fn,
                                 const params &p)
{
  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(fn.c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("XRFF data file format error");

  return read_xrff(doc, p);
}

///
/// Loads a XRFF file from a stream into the dataframe.
///
/// \param[in] in the xrff stream
/// \param[in] p  additional, optional, parameters (see `params` structure)
/// \return       number of lines parsed (`0` in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
/// \see `dataframe::read_xrff(tinyxml2::XMLDocument &)` for details.
///
std::size_t dataframe::read_xrff(std::istream &in, const params &p)
{
  std::ostringstream ss;
  ss << in.rdbuf();

  tinyxml2::XMLDocument doc;
  if (doc.Parse(ss.str().c_str()) != tinyxml2::XML_SUCCESS)
    throw exception::data_format("XRFF data file format error");

  return read_xrff(doc, p);
}
std::size_t dataframe::read_xrff(std::istream &in)
{
  return read_xrff(in, {});
}

///
/// Loads a XRFF document into the active dataset.
///
/// \param[in] doc object containing the xrff file
/// \param[in] p   additional, optional, parameters (see `params` structure)
/// \return        number of lines parsed (0 in case of errors)
///
/// \exception exception::data_format wrong data format for data file
///
/// An XRFF (eXtensible attribute-Relation File Format) file describes a list
/// of instances sharing a set of attributes.
/// The original format is defined in
/// https://waikato.github.io/weka-wiki/formats_and_processing/xrff/, we
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
/// To date we don't support compressed and sparse format XRFF files.
///
std::size_t dataframe::read_xrff(tinyxml2::XMLDocument &doc, const params &p)
{
  // Iterate over `dataset.header.attributes` selection and store all found
  // attributes in the header vector.
  tinyxml2::XMLHandle handle(&doc);
  auto *attributes(handle.FirstChildElement("dataset")
                         .FirstChildElement("header")
                         .FirstChildElement("attributes").ToElement());
  if (!attributes)
    throw exception::data_format("Missing `attributes` element in XRFF file");

  clear();

  unsigned n_output(0);
  bool classification(false);

  for (auto *attribute = attributes->FirstChildElement("attribute");
       attribute;
       attribute = attribute->NextSiblingElement("attribute"))
  {
    columns_info::column_info a;

    const char *s(attribute->Attribute("name"));
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
      for (auto *l(attribute->FirstChildElement("label"));
           l;
           l = l->NextSiblingElement("label"))
      {
        const std::string label(l->GetText() ? l->GetText() : "");
        categories_.add_label(a.category_id, label);
      }

    // Output column is always the first one.
    if (output)
      columns.push_front(a);
    else
      columns.push_back(a);
  }

  // XRFF needs information about the columns.
  if (columns.empty())
    throw exception::data_format("Missing column information in XRFF file");

  // If no output column is specified the default XRFF output column is the
  // last one (and it's the first element of the `header_` vector).
  if (n_output == 0)
  {
    columns.push_front(columns.back());
    columns.pop_back();
  }

  // Category 0 is the output category.
  swap_category(0, columns[0].category_id);

  if (auto *instances = handle.FirstChildElement("dataset")
                        .FirstChildElement("body")
                        .FirstChildElement("instances").ToElement())
  {
    for (auto *i(instances->FirstChildElement("instance"));
         i;
         i = i->NextSiblingElement("instance"))
    {
      record_t record;
      for (auto *v(i->FirstChildElement("value"));
           v;
           v = v->NextSiblingElement("value"))
        record.push_back(v->GetText() ? v->GetText() : "");

      if (p.filter && p.filter(record) == false)
        continue;

      const auto instance(to_example(record, false));

      if (instance.input.size() + 1 == columns.size())
        push_back(instance);
      else
        vitaWARNING << "Malformed example " << size() << " skipped";
    }
  }
  else
    throw exception::data_format("Missing `instances` element in XRFF file");

  return debug() ? size() : static_cast<std::size_t>(0);
}

///
/// Loads a CSV file into the active dataset.
///
/// \param[in] fn the csv filename
/// \param[in] p  additional, optional, parameters (see `params` structure)
/// \return       number of lines parsed (0 in case of errors)
///
/// \exception std::runtime_error    cannot read CSV data file
///
/// \see `dataframe::load_csv(const std::string &)` for details.
///
std::size_t dataframe::read_csv(const std::filesystem::path &fn,
                                const params &p)
{
  std::ifstream in(fn);
  if (!in)
    throw std::runtime_error("Cannot read CSV data file");

  return read_csv(in, p);
}

///
/// Loads a CSV file into the active dataset.
///
/// \param[in] from   the csv stream
/// \param[in] p  additional, optional, parameters (see `params` structure)
/// \return           number of lines parsed (0 in case of errors)
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
/// * the column containing the labels (numeric or string) for the examples can
///   be specified by the user; if not specified, the the first column is the
///   default. If the label is numeric Vita assumes a REGRESSION model; if it's
///   a string, a CATEGORIZATION (i.e. classification) model is assumed.
/// * each column must describe the same kind of information;
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
std::size_t dataframe::read_csv(std::istream &from, const params &p)
{
  clear();

  std::size_t count(0);
  for (auto record : csv_parser(from).filter_hook(p.filter))
  {
    if (p.output_index)
    {
      assert(p.output_index < record.size());
      //std::swap(record[0], record[*p.output_index]);
      if (p.output_index > 0)
        std::rotate(record.begin(),
                    std::next(record.begin(), *p.output_index),
                    std::next(record.begin(), *p.output_index + 1));
    }
    else
      // When the output index is unspecified, all the columns are treated as
      // input columns (this is obtained adding a surrogate, empty output
      // column).
      record.insert(record.begin(), {});

    // Every new record may add further information about the column domain /
    // category.
    if (count < 10)
      columns.build(record, p.has_header, categories_);
    if (!p.has_header || count++)
      read_record(record);
  }

  if (!debug() || !size())
    throw exception::insufficient_data("Empty / undersized CSV data file");

  return size();
}
std::size_t dataframe::read_csv(std::istream &from)
{
  return read_csv(from, {});
}

///
/// Loads the content of a file into the active dataset.
///
/// \param[in] fn name of the file containing the data set (CSV / XRFF format)
/// \param[in] p  additional, optional, parameters (see `params` structure)
/// \return       number of lines parsed
///
/// \exception std::invalid_argument missing dataset file name
///
/// \note Test set can have an empty output value.
///
std::size_t dataframe::read(const std::filesystem::path &fn, const params &p)
{
  if (fn.empty())
    throw std::invalid_argument("Missing dataset filename");

  const auto ext(fn.extension());
  const bool xrff(iequals(ext, ".xrff") || iequals(ext, ".xml"));

  return xrff ? read_xrff(fn, p) : read_csv(fn, p);
}
std::size_t dataframe::read(const std::filesystem::path &fn)
{
  return read(fn, {});
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
