/**
 *
 *  \file data.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "data.h"
#include "random.h"
#include "symbol.h"

namespace vita
{
  ///
  /// New empty data instance.
  ///
  data::data()
  {
    clear();

    assert(debug());
  }

  ///
  /// \param[in] filename nome of the file containing the learning collection.
  /// \param[in] verbosity verbosity level (see environment::verbosity for
  ///            further details).
  /// New data instance containing the learning collection from \a filename.
  ///
  data::data(const std::string &filename, unsigned verbosity)
  {
    assert(filename != "");

    clear();
    open(filename, verbosity);

    assert(debug());
  }

  ///
  /// Resets the object.
  ///
  void data::clear()
  {
    categories_map_.clear();
    classes_map_.clear();

    header_.clear();
    categories_.clear();

    for (size_t i(0); i <= k_max_dataset; ++i)
    {
      dataset_[i].clear();
      end_[i] = dataset_[i].end();
    }

    active_dataset_ = training;

    assert(debug());
  }

  ///
  /// \param[in] d the active dataset.
  ///
  /// We can choose the data we want to operate on (training / validation /
  /// test set).
  ///
  void data::dataset(dataset_t d)
  {
    active_dataset_ = d;
  }

  ///
  /// \return the type (training, validation, test) of the active dataset.
  ///
  data::dataset_t data::dataset() const
  {
    return active_dataset_;
  }

  ///
  /// \param[in] n number of elements for the slice.
  ///
  /// Cuts a 'slice' of \a n elements in the active dataset. Future calls to
  /// data::end() will refer to the active slice (a subset of the dataset).
  /// To reset the slice call data::slice with argument 0.
  ///
  void data::slice(size_t n)
  {
    end_[dataset()] = (n == 0 || n >= size()) ?
      dataset_[dataset()].end() : std::next(begin(), n);
  }

  ///
  /// \return reference to the first element of the active dataset.
  ///
  data::iterator data::begin()
  {
    return dataset_[dataset()].begin();
  }

  ///
  /// \return a constant reference to the first element of the active dataset.
  ///
  data::const_iterator data::cbegin() const
  {
    return dataset_[dataset()].cbegin();
  }

  ///
  /// \return a constant reference to the last+1 (sentry) element of the active
  ///         dataset.
  ///
  data::iterator data::end() const
  {
    return end_[dataset()];
  }

  ///
  /// \param[in] d a dataset (training / validation / test set).
  /// \return the size of the dataset \a d.
  ///
  /// \note
  /// Please note that the result is independent of the active slice.
  ///
  size_t data::size(dataset_t d) const
  {
    return dataset_[d].size();
  }

  ///
  /// \return the size of the active dataset.
  ///
  size_t data::size() const
  {
    return size(dataset());
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
  const data::category &data::get_category(category_t i) const
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
    assert(i < columns());
    return header_[i];
  }

  ///
  /// \param[in] f the comparer used for sorting.
  ///
  /// Sorts the active slice in the current dataset (slice is preserved).
  ///
  void data::sort(std::function<bool (const example &, const example &)> f)
  {
    const dataset_t d(dataset());
    const size_t partition_size(std::distance(begin(), end()));

    dataset_[d].sort(f);

    slice(partition_size);
  }

  ///
  /// \param[in] r this is the \a size_of_validation_set / \a size_of_dataset
  ///              ratio.
  ///
  /// Splits the dataset in two subsets (training set, validation set)
  /// according to the \a r ratio.
  ///
  void data::divide(double r)
  {
    assert(0.0 <= r && r <= 1.0);

    // Validation set items are moved to the training set.
    while (!dataset_[validation].empty())
    {
      dataset_[training].push_back(dataset_[validation].front());
      dataset_[validation].pop_front();
    }

    if (r > 0.0)
    {
      // The requested validation examples are selected (the algorithm hint is
      // due to Kyle Cronin)...
      //
      // > Iterate through and for each element make the probability of
      // >  selection = (number needed)/(number left)
      // >
      // > So if you had 40 items, the first would have a 5/40 chance of being
      // > selected. If it is, the next has a 4/39 chance, otherwise it has a
      // > 5/39 chance. By the time you get to the end you will have your 5
      // > items, and often you'll have all of them before that".
      size_t available(dataset_[training].size());

      const size_t k(available * r);
      assert(k <= available);

      size_t needed(k);

      auto iter(dataset_[training].begin());
      while (dataset_[validation].size() < k)
      {
        if (random::boolean(static_cast<double>(needed) / available))
        {   // selected
          dataset_[validation].push_back(*iter);
          iter = dataset_[training].erase(iter);
          --needed;
        }
        else  // not selected
          ++iter;

        --available;
      }

      assert(!needed);
    }
  }

  ///
  /// \return number of categories of the problem (>= 1).
  ///
  /// \attention
  /// please note that the value categories() returns may differ from the
  /// intuitive number of categories of the dataset (it can be 1 unit smaller).
  /// For instance consider the simple Iris classification problem:
  ///
  ///     ...
  ///     <attribute class="yes" name="class" type="nominal">
  ///       <labels>
  ///         <label>Iris-setosa</label> ... <label>Iris-virginica</label>
  ///       </labels>
  ///     </attribute>
  ///     <attribute name="sepallength" type="numeric" />
  ///     ...
  /// It has a nominal attribute to describe output classes and four numeric
  /// attributes as inputs. So there are two distinct attribute types
  /// (nominal and numeric), i.e. two categories.
  /// But... categories() would return 1.
  /// This happens because the genetic programming algorithm for classification
  /// we use (based on a discriminant function) doesn't manipulate (skips) the
  /// output category (it only uses the number of output classes).
  ///
  size_t data::categories() const
  {
    return categories_.size();
  }

  ///
  /// \return number of columns of the dataset.
  ///
  /// \note data class supports just one output for every instance, so, if
  /// the dataset is not empty: \code variables() + 1 == columns() \endcode.
  ///
  size_t data::columns() const
  {
    assert(dataset_[dataset()].empty() || variables() + 1 == header_.size());

    return header_.size();
  }

  ///
  /// \return number of classes of the problem (== 0 for a symbolic regression
  ///         problem, > 1 for a classification problem).
  ///
  size_t data::classes() const
  {
    return classes_map_.size();
  }

  ///
  /// \return input vector dimension.
  ///
  /// \note data class supports just one output for every instance, so, if
  /// the dataset is not empty, \code variables() + 1 == columns() \endcode.
  ///
  size_t data::variables() const
  {
    const size_t n(dataset_[dataset()].empty() ? 0 : cbegin()->input.size());

    assert(dataset_[dataset()].empty() || n + 1 == header_.size());

    return n;
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
      const size_t n(map->size());
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
    for (const auto &p : classes_map_)
      if (p.second == i)
        return p.first;

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
  /// and efficient for parsing, but it is not as easily applied.
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

      if (!inquotes && curstring.length() == 0 && c == '"') // begin quote char
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
      for (size_t i(0); i < record.size(); ++i)
        boost::trim(record[i]);

    return record;
  }

  ///
  /// \param[in] s the string to be converted.
  /// \param[in] d what type should \a s be converted in?
  /// \return the converted data.
  ///
  /// convert("123.1", sym_double) == double(123.1)
  ///
  data::example::value_t data::convert(const std::string &s, domain_t d)
  {
    switch (d)
    {
    case d_bool:  return   data::example::value_t(boost::lexical_cast<bool>(s));
    case d_int:   return    data::example::value_t(boost::lexical_cast<int>(s));
    case d_double:return data::example::value_t(boost::lexical_cast<double>(s));
    case d_string:return                              data::example::value_t(s);
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

  ///
  /// \param[in] c1 a category.
  /// \param[in] c2 a category.
  ///
  /// Swap catagories \a c1 and \a c2, updating the \a header_ and
  /// \a categories_ vector.
  ///
  void data::swap_category(category_t c1, category_t c2)
  {
    assert(c1 < columns());
    assert(c2 < columns());

    std::swap(categories_[c1], categories_[c2]);

    for (size_t i(0); i < columns(); ++i)
      if (header_[i].category_id == c1)
        header_[i].category_id = c2;
      else if (header_[i].category_id == c2)
        header_[i].category_id = c1;
  }

  ///
  /// \param[in] filename the xrff file.
  /// \return number of lines parsed (0 in case of errors).
  ///
  /// Loads the content of \a filename into the active dataset.
  ///
  /// \note
  /// An XRFF (eXtensible attribute-Relation File Format) file describes a list
  /// of instances sharing a set of attributes.
  /// The original format is defined in http://weka.wikispaces.com/XRFF, we
  /// extend it with an additional (non-standard) feature: attribute category.
  ///
  ///     <attribute name="vehicle length" type="numeric" category="length" /
  ///     <attribute name="vehicle width" type="numeric" category="length" />
  ///     <attribute name="vehicle weight" type="numeric" category="weight" />
  ///
  /// This feature is used to constrain the search (Strongly Typed Genetic
  /// Programming).
  ///
  /// \post
  /// \li \a header_[0] is the output column (it contains informations about
  ///     problem's output);
  /// \li \a category(0) is the output category (for symbolic regresssion
  ///     problems it is the output type of the xrff file, for classification
  ///     problems it is the \a numeric type).
  ///
  /// \warning
  /// To date:
  /// * we don't support compressed XRFF files;
  /// * XRFF files cannot be uset to load test set (problems with missing
  ///   output column and possible column category redefinition).
  ///
  /// \note
  /// Test set can have an empty output value.
  ///
  size_t data::load_xrff(const std::string &filename)
  {
    assert(dataset() == training);

    using namespace boost::property_tree;

    ptree pt;
    read_xml(filename, pt);

    size_t n_output(0);
    bool classification(false);

    // Iterate over dataset.header.attributes selection and store all found
    // attributes in the header vector. The get_child() function returns a
    // reference to the child at the specified path; if there is no such child
    // IT THROWS.
    for (ptree::value_type dha : pt.get_child("dataset.header.attributes"))
      if (dha.first == "attribute")
      {
        bool output(false);

        column a;

        // In a ptree xml-attributes are just sub-elements of a special element
        // <xmlattr>.
        // Note that we don't have to provide the template parameter when it is
        // deduced from the default value.
        a.name = dha.second.get("<xmlattr>.name", "");

        // One can define which attribute should act as output value via the
        // class="yes" attribute in the attribute specification in the header.
        output = dha.second.get("<xmlattr>.class", "no") == "yes";

        std::string xml_type(dha.second.get("<xmlattr>.type", ""));

        std::string category_name(dha.second.get("<xmlattr>.category",
                                                 xml_type));

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

        a.category_id = encode(category_name, &categories_map_);

        if (a.category_id >= categories_.size())
        {
          assert(a.category_id == categories_.size());
          categories_.push_back(category{category_name, from_weka(xml_type),
                                         {}});
        }

        if (xml_type == "nominal")
          try
          {
            // Store label1... labelN.
            for (ptree::value_type l : dha.second.get_child("labels"))
              if (l.first == "label")
                categories_[a.category_id].labels.insert(l.second.data());
          }
          catch(...)
          {
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
    // last one (and it is the first element of the header_ vector).
    if (n_output == 0)
    {
      header_.insert(header_.begin(), header_.back());
      header_.pop_back();
    }

    // Category 0 is the output category.
    swap_category(category_t(0), header_[0].category_id);

    size_t parsed(0);
    for (ptree::value_type bi : pt.get_child("dataset.body.instances"))
      if (bi.first == "instance")
      {
        example instance;

        unsigned index(0);
        for (auto v(bi.second.begin()); v != bi.second.end(); ++v, ++index)
          if (v->first == "value")
            try
            {
              const domain_t domain(
                categories_[header_[index].category_id].domain);

              const std::string value(v->second.data());

              if (index == 0)  // output value
              {
                // Strings could be used as label for classes, but integers
                // are simpler and faster to manage (arrays instead of maps).
                if (classification)
                  instance.output = static_cast<int>(encode(value,
                                                            &classes_map_));
                else
                  instance.output = convert(value, domain);
              }
              else  // input value
                instance.input.push_back(convert(value, domain));
            }
            catch(boost::bad_lexical_cast &)
            {
              instance.clear();
              continue;
            }

        if (instance.input.size() + 1 == columns())
        {
          dataset_[dataset()].push_back(instance);
          ++parsed;
        }
      }

    return debug() ? parsed : 0;
  }

  ///
  /// \param[in] filename the csv file.
  /// \param[in] verbosity verbosity level (see environment::verbosity for
  ///            further details).
  /// \return number of lines parsed (0 in case of errors).
  ///
  /// We follow the Google Prediction API convention
  /// (https://developers.google.com/prediction/docs/developer-guide?hl=it#data-format):
  /// * NO HEADER ROW is allowed;
  /// * only one example is allowed per line. A single example cannot contain
  ///   newlines and cannot span multiple lines;
  /// * columns are separated by commas. Commas inside a quoted string are not
  ///   column delimiters;
  /// * THE FIRST COLUMN REPRESENTS THE VALUE (numeric or string) for that
  ///   example. If the first column is numeric, this model is a REGRESSION
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
  /// \note
  /// Test set can have an empty output value.
  ///
  size_t data::load_csv(const std::string &filename, unsigned verbosity)
  {
    std::ifstream from(filename.c_str());
    if (!from)
      return 0;

    bool classification(false);

    std::string line;
    while (std::getline(from, line))
    {
      const std::vector<std::string> record(csvline(line));
      const size_t size(columns() ? columns() : record.size());

      if (record.size() == size)
      {
        example instance;

        if (!dataset_[dataset()].size())  // No line parsed
          classification = !is_number(record[0]);

        for (size_t field(0); field < size; ++field)
        {
          // The first line is (also) used to learn data format.
          if (columns() != size)
          {
            assert(!dataset_[dataset()].size());

            column a;
            a.name = "";

            std::string s_domain(is_number(record[field])
                                 ? "numeric"
                                 : "string" +
                                   boost::lexical_cast<std::string>(field));
            // For classification problems we use discriminant functions, so the
            // actual output type is always numeric.
            if (field == 0 && classification)
              s_domain = "numeric";

            const domain_t domain(s_domain == "numeric" ? d_double : d_string);

            a.category_id = encode(s_domain, &categories_map_);
            if (a.category_id >= categories_.size())
            {
              assert(a.category_id == categories_.size());
              categories_.push_back(category{s_domain, domain, {}});
            }

            header_.push_back(a);
          }

          try
          {
            const category_t c(header_[field].category_id);

            const std::string value(record[field]);

            if (field == 0)  // output value
            {
              if (value.empty())
              {
                assert(dataset() == test);
                // For test set the output class/value could be missing (e.g.
                // for Kaggle.com competition test set).
              }
              else
              {
                if (classification)
                  instance.output = static_cast<int>(encode(value,
                                                            &classes_map_));
                else
                  instance.output = convert(value, categories_[c].domain);
              }
            }
            else  // input value
            {
              instance.input.push_back(convert(value, categories_[c].domain));
              if (categories_[c].domain == d_string)
                categories_[c].labels.insert(value);
            }
          }
          catch(boost::bad_lexical_cast &)
          {
            instance.clear();
            continue;
          }
        }

        if (instance.input.size() + 1 == columns())
          dataset_[dataset()].push_back(instance);
        else if (verbosity >= 2)
          std::cout << k_s_warning << " [" << line << "] skipped" << std::endl;
      }
    }

    return debug() ? dataset_[dataset()].size() : 0;
  }

  ///
  /// \param[in] f name of the file containing the data set.
  /// \param[in] verbosity verbosity level (see environment::verbosity for
  ///            further details).
  /// \return number of lines parsed (0 in case of errors).
  ///
  /// Loads the content of \a f into the active dataset.
  ///
  /// \warning
  /// * Training/valutation set must be loaded before test set.
  /// * Before changing problem the data object should be clear.
  ///
  /// So:
  ///
  ///     dataset(training);
  ///     open("training.csv");
  ///     dataset(test);
  ///     open("test.csv");
  ///     ...
  ///     clear();
  ///     dataset(training);
  ///     open("training2.csv");
  ///     ...
  ///
  /// \note
  /// Test set can have an empty output value.
  ///
  size_t data::open(const std::string &f, unsigned verbosity)
  {
    const bool xrff(boost::algorithm::iends_with(f, ".xrff") ||
                    boost::algorithm::iends_with(f, ".xml"));

    return xrff ? load_xrff(f) : load_csv(f, verbosity);
  }

  ///
  /// \return \c true if the current dataset is empty.
  ///
  bool data::operator!() const
  {
    return size() == 0;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool data::debug() const
  {
    const size_t cl_size(classes());
    // If this is a classification problem then there should be at least two
    // classes.
    if (cl_size == 1)
      return false;

    for (const auto &d : dataset_)
      if (!d.empty() && &d != &dataset_[test])
      {
        const size_t in_size(d.begin()->input.size());

        for (const auto &e : d)
        {
          if (e.input.size() != in_size)
            return false;

          if (cl_size && e.label() >= cl_size)
            return false;
        }
      }

    return true;
  }
}  // Namespace vita
