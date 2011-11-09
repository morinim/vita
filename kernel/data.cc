/**
 *
 *  \file data.cc
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "kernel/data.h"
#include "kernel/random.h"
#include "kernel/symbol.h"

namespace vita
{
  ///
  /// \param[in] n number of distinct datasets. E.g.:
  ///              <ol>
  ///                <li>auto choose;</li>
  ///                <li>not partitioned;</li>
  ///                <li>one training set and one validation set;</li>
  ///                <li>two training sets and one validation set...</li>
  ///              </ol>
  /// \see clear
  ///
  /// New empty data instance (partitioned in \a n training sets).
  ///
  data::data(unsigned n)
  {
    assert(n);

    clear(n);

    assert(check());
  }

  ///
  /// \param[in] filename nome of the file containing the learning collection.
  /// \param[in] n number of distinct datasets.
  ///
  /// New \a data instance containing the learning collection from \a filename
  /// and partitioned in \a n datasets.
  ///
  data::data(const std::string &filename, unsigned n)
  {
    assert(filename != "");
    assert(n);

    clear(n);
    open(filename);

    assert(check());
  }

  ///
  /// \param[in] n number of distinct datasets.
  ///
  /// Resets the object.
  ///
  void data::clear(unsigned n)
  {
    assert(n);

    categories_map_.clear();
    classes_map_.clear();

    header_.clear();
    categories_.clear();

    datasets_.clear();
    datasets_.resize(n ? n : 1);

    // This is the active data partition.
    active_ = 0;

    assert(check());
  }

  ///
  /// \return constant reference to the first element of the active dataset.
  ///
  data::const_iterator data::begin() const
  {
    assert(active_ < datasets_.size());
    return datasets_[active_].begin();
  }

  ///
  /// \return constant reference to the last+1 (sentry) element of the active
  ///         dataset.
  ///
  data::const_iterator data::end() const
  {
    assert(active_ < datasets_.size());
    return datasets_[active_].end();
  }

  ///
  /// \return the size of the active dataset.
  ///
  unsigned data::size() const
  {
    return datasets_[active_].size();
  }

  ///
  /// \param[in] name name of a category.
  /// \return the index of the \a name category (0 if it doesn't exist ).
  ///
  category_t data::get_category(const std::string &name) const
  {
    const auto cat(categories_map_.find(name));

    assert(cat == categories_map_.end() || cat->second < categories_.size());

    return cat == categories_map_.end() ? 0 : cat->second;
  }

  ///
  /// \param[in] i index of a category.
  /// \return a const reference to the i-th category.
  ///
  const data::category &data::get_category(unsigned i) const
  {
    assert(i < categories_.size());
    return categories_[i];
  }

  ///
  /// \param[in] i index of a column.
  /// \return a const reference to the i-th column of the dataset.
  ///
  const data::column &data::get_column(unsigned i) const
  {
    assert(i < header_.size());
    return header_[i];
  }

  ///
  /// \return number of categories of the problem (>= 1).
  ///
  /// \attention please note that \c data::categories() may differ from the
  /// intuitive number of categories of the dataset.
  ///
  /// For instance consider the simple Iris classification problem (nominal
  /// attribute as output):
  /// \verbatim
  /// <attribute class="yes" name="class" type="nominal">
  ///   <labels>
  ///     <label>Iris-setosa</label> ... <label>Iris-virginica</label>
  ///   </labels>
  /// </attribute>
  /// \endverbatim
  /// Genetic programming algorithms for classification (at least the
  /// algorithms that learn a discriminant function) don't manipulate the
  /// output category (it is "superfluous", the only relevant information is the
  /// number of output classes).
  ///
  /// So Dataset (m categories) => \c vita::data (n categories) =>
  /// \c vita::src_problem => \c vita::symbol_set (n categories).
  ///
  unsigned data::categories() const
  {
    return categories_.size();
  }

  ///
  /// \return number of columns of the dataset.
  ///
  /// \note data class supports just one output for every instance, so, if
  /// the dataset is not empty, variables() + 1 == columns().
  ///
  unsigned data::columns() const
  {
    assert(datasets_.empty() || variables() + 1 == header_.size());
    return header_.size();
  }

  ///
  /// \return number of classes of the problem (== 0 for a symbolic regression
  ///         problem, > 1 for a classification problem).
  ///
  unsigned data::classes() const
  {
    return classes_map_.size();
  }

  ///
  /// \return input vector dimension.
  ///
  /// \note data class supports just one output for every instance, so, if
  /// the dataset is not empty, variables() + 1 == columns().
  ///
  unsigned data::variables() const
  {
    assert(datasets_.empty() || begin()->input.size() + 1 == header_.size());
    return datasets_.empty() || datasets_[0].empty()
      ? 0 : begin()->input.size();
  }

  ///
  /// \param[in] label name of a class of the learning collection.
  /// \param[in,out] map map used to encode the \a label.
  /// \return a positive integer used as primary key for the class \a label.
  ///
  unsigned data::encode(const std::string &label,
                        std::map<std::string, unsigned> *map)
  {
    if (map->find(label) == map->end())
    {
      const unsigned n(map->size());
      (*map)[label] = n;
    }

    return (*map)[label];
  }

  ///
  /// \param[in] i the encoded (data::encode()) value of a class.
  /// \return the name of the class encoded with the \c unsigned \a i (or an
  ///         empty string if such class cannot be find).
  ///
  /// \note Boost Bimap could be used to speed up the search in \a classes_map_,
  /// but to date speed isn't an issue.
  ///
  std::string data::class_name(unsigned i) const
  {
    for (auto p(classes_map_.begin()); p != classes_map_.end(); ++p)
      if (p->second == i)
        return p->first;

    return "";
  }

  ///
  /// \param[in] line line to be parsed.
  /// \param[in] delimiter separator character for fields.
  /// \param[in] trim if \c true trims leading and trailing spaces adjacent to
  ///                 commas (this practice is contentious and in fact is
  ///                 specifically prohibited by RFC 4180, which states,
  ///                 "Spaces are considered part of a field and should not be
  ///                 ignored."
  /// \return a vector where each element is a field of the CSV line.
  ///
  /// This function parses a line of data by a delimiter. If you pass in a
  /// comma as your delimiter it will parse out a Comma Separated Value (CSV)
  /// file. If you pass in a '\t' char it will parse out a tab delimited file
  /// (.txt or .tsv). CSV files often have commas in the actual data, but
  /// accounts for this by surrounding the data in quotes. This also means the
  /// quotes need to be parsed out, this function accounts for that as well.
  /// The only (known) problem with this code, is that the definition of a csv
  /// (http://en.wikipedia.org/wiki/Comma-separated_values) allows for the
  /// newline character '\n' to be part of a csv field if the field is
  /// surrounded by quotes. The csvline function takes care of this properly,
  /// but the data::open function, which calls csvline, doesn't handle it. Most
  /// CSV files do not have a \n in the middle of the field, so it is usually
  /// not worth worrying about.
  /// This is a slightly modified version of the function at
  /// http://www.zedwood.com/article/112/cpp-csv-parser.
  /// Escaped List Separator class from Boost C++ libraries is also very nice
  /// and efficient for parsing,but it is not as easily applied.
  ///
  std::vector<std::string> data::csvline(const std::string &line,
                                         char delimiter,
                                         bool trim)
  {
    std::vector<std::string> record;

    const unsigned linemax(line.length());
    bool inquotes(false);
    unsigned linepos(0);
    std::string curstring;
    while (line[linepos] != 0 && linepos < linemax)
    {
      const char c(line[linepos]);

      if (!inquotes && curstring.length() == 0 && c == '"')  // begin quote char
        inquotes = true;
      else if (inquotes && c == '"')
      {
        // Quote char.
        if (linepos+1 < linemax && line[linepos+1] == '"')
        {
          // Encountered 2 double quotes in a row (resolves to 1 double quote).
          curstring.push_back(c);
          ++linepos;
        }
        else  // end quote char
          inquotes = false;
      }
      else if (!inquotes && c == delimiter)  // end of field
      {
        record.push_back(curstring);
        curstring = "";
      }
      else if (!inquotes && (c == '\r' || c == '\n'))
        break;
      else
        curstring.push_back(c);

      ++linepos;
    }

    record.push_back(curstring);

    if (trim)
      for (unsigned i(0); i < record.size(); ++i)
        boost::trim(record[i]);

    return record;
  }

  ///
  /// \param[in] s the string to be converted.
  /// \param[in] t what type should \a s be converted in?
  /// \return the converted data.
  ///
  /// convert("123.1", sym_real) == 123.1f
  ///
  boost::any data::convert(const std::string &s, domain_t d)
  {
    switch (d)
    {
    case d_bool:   return   boost::lexical_cast<bool>(s);
    case d_int:    return    boost::lexical_cast<int>(s);
    case d_double: return boost::lexical_cast<double>(s);
    case d_string: return                              s;
    default: throw boost::bad_lexical_cast();
    }
  }

  ///
  /// \param[in] s the string to be tested.
  /// \return \c true if \a s contains a number.
  ///
  bool data::is_number(const std::string &s)
  {
    try
    {
      boost::lexical_cast<double>(s);
    }
    catch(boost::bad_lexical_cast &)
    {
      return false;
    }

    return true;
  }

# pragma GCC diagnostic ignored "-Wtype-limits"
  ///
  /// \param[in] filename the xrff file.
  /// \return number of lines parsed (0 in case of errors).
  ///
  /// An XRFF (eXtensible attribute-Relation File Format) file describes a list
  /// of instances sharing a set of attributes. To date we don't support
  /// compressed XRFF files.
  /// The original format is defined in http://weka.wikispaces.com/XRFF, we
  /// extend it with an additional (non-standard) feature: attribute category.
  /// \verbatim
  /// <attribute name="vehicle length" type="numeric" category="length" /
  /// <attribute name="vehicle width" type="numeric" category="length" />
  /// <attribute name="vehicle weight" type="numeric" category="weight" />
  /// \endverbatim
  /// This feature is used to constrain GP search (Strongly Typed Genetic
  /// Programming).
  ///
  unsigned data::load_xrff(const std::string &filename)
  {
    static std::map<const std::string, domain_t> from_weka =
    {
      // This type is vita-specific (not standard).
      {"boolean", domain_t::d_bool},

      {"integer", domain_t::d_int},

      // Real and numeric are treated as double precisione number (d_double).
      {"numeric", domain_t::d_double},
      {"real", domain_t::d_double},

      // Nominal values are deined by providing a list of possible values.
      {"nominal", domain_t::d_string},

      // String attributes allow us to create attributes containing arbitrary
      // textual values. This is very useful in text-mining applications.
      {"string", domain_t::d_string}

      // {"date", ?}, {"relational", ?}
    };

    using namespace boost::property_tree;

    ptree pt;
    read_xml(filename, pt);

    bool classification(false);

    // Iterate over dataset.header.attributes selection and store all found
    // attributes in the header vector. The get_child() function returns a
    // reference to the child at the specified path; if there is no such child
    // IT THROWS. Property tree iterators are models of BidirectionalIterator.
    BOOST_FOREACH(ptree::value_type dha,
                  pt.get_child("dataset.header.attributes"))
      if (dha.first == "attribute")
      {
        column a;

        // Structure ptree does not have a concept of an attribute, so xml
        // attributes are just sub-elements of a special element <xmlattr>.
        // Note that we don't have to provide the template parameter when it is
        // deduced from the default value.
        a.name = dha.second.get("<xmlattr>.name", "");

        // Via the class="yes" attribute in the attribute specification in the
        // header, one can define which attribute should act as output value.
        a.output = dha.second.get("<xmlattr>.class", "no") == "yes";

        const std::string xml_type(dha.second.get("<xmlattr>.type", ""));

        if (a.output)
          classification = (xml_type == "nominal" || xml_type == "string");

        const std::string category_name(dha.second.get("<xmlattr>.category",
                                                       xml_type));

        // Note the special treatment of the output column of a classification
        // problem: for category_id calculation, we completely ignore the type
        // recorded in the dataset file.
        // The reason is that genetic programming classification algorithms
        // don't manimulate the labels of the output category (they only need
        // the number of classes of the classification problem).
        // So category_id isn't meaningful for the output column of a
        // classification problem.
        if (a.output && classification)
          a.category_id = 0;
        else
        {
          a.category_id = encode(category_name, &categories_map_);

          if (a.category_id >= categories_.size())
          {
            assert(a.category_id == categories_.size());
            categories_.push_back(category{category_name,
                                           from_weka[xml_type],
                                           {}});
          }

          if (xml_type == "nominal")
            try
            {
              BOOST_FOREACH(ptree::value_type l, dha.second.get_child("labels"))
                if (l.first == "label")
                {
                  // Store label1... labelN}
                }
            }
            catch(...)
            {
            }
        }

        header_.push_back(a);
      }

    // XRFF needs informations about the columns.
    if (!header_.size())
      return 0;

    // Picks some data about output(s).
    unsigned output_column, n_output(0);
    for (unsigned i(0); i < header_.size(); ++i)
      if (header_[i].output)
      {
        output_column = i;
        ++n_output;
      }

    // We can manage only one output column.
    if (n_output > 1)
      return 0;

    // If there isn't an explicitly defined output column, we assume it is the
    // last one.
    if (!n_output)
      header_[header_.size()-1].output = true;

    unsigned parsed(0);
    BOOST_FOREACH(ptree::value_type bi, pt.get_child("dataset.body.instances"))
      if (bi.first == "instance")
      {
        value_type instance;

        unsigned index(0);
        for (auto v(bi.second.begin()); v != bi.second.end(); ++v, ++index)
          if (v->first == "value")
            try
            {
              const domain_t domain(
                categories_[header_[index].category_id].domain);

              if (header_[index].output)
              {
                if (classification)
                  instance.output = encode(v->second.data(), &classes_map_);
                else
                  instance.output = convert(v->second.data(), domain);
              }
              else  // input value
                instance.input.push_back(convert(v->second.data(), domain));
            }
            catch(boost::bad_lexical_cast &)
            {
              instance.clear();
              continue;
            }

        if (instance.input.size() + 1 == header_.size())
        {
          const unsigned set(vita::random::between<unsigned>(0,
                                                             datasets_.size()));
          datasets_[set].push_back(instance);
          ++parsed;
        }
      }

    return check() ? parsed : 0;
  }

  ///
  /// \param[in] filename the csv file.
  /// \return number of lines parsed (0 in case of errors).
  ///
  /// We follow the Google Prediction API convention
  /// (http://code.google.com/intl/it/apis/predict/docs/developer-guide.html#data-format):
  /// * NO HEADER ROW is allowed;
  /// * only one example is allowed per line. A single example cannot contain
  ///   newlines and cannot span multiple lines;
  /// * columns are separated by commas. Commas inside a quoted string are not
  ///   column delimiters;
  /// * THE FIRST COLUMN REPRESENTS THE VALUE (numeric or string) for that
  ///   example. If the first column is numetic, this model is a REGRESSION
  ///   model; if the first column is a string, it is a CATEGORIZATION
  ///   (i.e. classification) model. Each column must describe the same kind of
  ///   information for that example;
  /// * the column order of features in the table does not weight the results;
  ///   the first feature is not weighted any more than the last;
  /// * as a best practice, remove punctuation (other than apostrophes) from
  ///   your data. This is because commas, periods, and other punctuation
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
  unsigned data::load_csv(const std::string &filename)
  {
    std::ifstream from(filename.c_str());
    if (!from)
      return 0;

    bool classification(false);

    std::string line;
    unsigned parsed(0);
    while (std::getline(from, line))
    {
      const std::vector<std::string> record(csvline(line));
      const unsigned size(parsed ? header_.size() : record.size());

      if (record.size() == size)
      {
        value_type instance;

        if (!parsed)
          classification = !is_number(record[0]);

        for (unsigned field(0); field < size; ++field)
        {
          // The first line is (also) used to learn data format.
          if (!parsed)
          {
            column a;

            a.name = "";
            a.output = (field == 0);

            const std::string s_domain(is_number(record[field]) ?
                                       "numeric" : "string");
            const domain_t domain(s_domain == "numeric" ? d_double : d_string);

            if (a.output && classification)
              a.category_id = 0;
            else
            {
              a.category_id = encode(s_domain, &categories_map_);
              if (a.category_id >= categories_.size())
              {
                assert(a.category_id == categories_.size());
                categories_.push_back(category{s_domain, domain, {}});
              }
            }

            header_.push_back(a);
          }

          try
          {
            const category_t c(header_[field].category_id);

            if (field == 0)  // output value
            {
              if (classification)
                instance.output = encode(record[field], &classes_map_);
              else
                instance.output = convert(record[field], categories_[c].domain);
            }
            else  // input value
              instance.input.push_back(convert(record[field],
                                               categories_[c].domain));
          }
          catch(boost::bad_lexical_cast &)
          {
            instance.clear();
            continue;
          }
        }

        if (instance.input.size() + 1 == header_.size())
        {
          const unsigned set(vita::random::between<unsigned>(0,
                                                             datasets_.size()));
          datasets_[set].push_back(instance);
          ++parsed;
        }
      }
    }

    return check() ? parsed : 0;
  }

  ///
  /// \param[in] filename name of the file containing the learning collection.
  /// \return number of lines parsed (0 in case of errors).
  ///
  unsigned data::open(const std::string &filename)
  {
    header_.clear();

    const bool xrff(boost::algorithm::iends_with(filename, ".xrff") ||
                    boost::algorithm::iends_with(filename, ".xml"));

    return xrff ? load_xrff(filename) : load_csv(filename);
  }

  ///
  /// \return \c true if the learning collection is empty.
  ///
  bool data::operator!() const
  {
    bool found(false);

    for (auto i(datasets_.begin()); i != datasets_.end() && !found; ++i)
      found = !i->empty();

    return !found;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool data::check() const
  {
    if (datasets_.empty())
      return false;

    const unsigned cl_size(classes());
    // If this is a classification problem then there should be at least two
    //  classes.
    if (cl_size == 1)
      return false;

    const unsigned in_size(datasets_[0].begin()->input.size());

    for (auto l(datasets_.begin()); l != datasets_.end(); ++l)
      for (auto i(l->begin()); i != l->end(); ++i)
      {
        if (i->input.size() != in_size)
          return false;

        if (cl_size && i->label() >= cl_size)
          return false;
      }

    return active_ < datasets_.size();
  }
}  // Namespace vita
