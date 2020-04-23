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

#include "utility/csv_parser.h"
#include "utility/utility.h"

namespace
{

enum column_tag {none_tag = 0, skip_tag = -1,
                 number_tag = -2, string_tag = -3};

int find_column_tag(const std::string &s)
{
  const auto ts(vita::trim(s));

  if (ts.empty())
    return none_tag;
  if (vita::is_number(ts))
    return number_tag;

  return s.length();
}

}  // unnamed namespace

namespace vita
{

///
/// *Sniffs* the format of a CSV file (delimiter, headers).
///
/// \param[in] is stream containing CSV data
/// \return       a csv_dialect object
///
/// For detecting the **header** creates a dictionary of types of data in each
/// column. If any column is of a single type (say, integers), *except* for the
/// first row, then the first row is presumed to be labels. If the type cannot
/// be determined, it's assumed to be a string in which case the length of the
/// string is the determining factor: if all of the rows except for the first
/// are the same length, it's a header.
/// Finally, a 'vote' is taken at the end for each column, adding or
/// subtracting from the likelihood of the first row being a header.
///
csv_dialect csv_sniffer(std::istream &is)
{
  csv_dialect ret;

  csv_parser parser(is, {});

  const auto header(*parser.begin());  // assume first row is header
  const auto columns(header.size());
  std::vector<int> column_types(columns, none_tag);

  unsigned checked(0);

  for (auto it(std::next(parser.begin())); it != parser.end(); ++it)
    // Skip rows that have irregular number of columns
    if (const auto row = *it; row.size() == columns)
    {
      for (std::size_t field(0); field < columns; ++field)
        if (column_types[field] != skip_tag  // inconsistent column
            && !trim(row[field]).empty())   // missing values don't contribute
        {
          const auto this_tag(find_column_tag(row[field]));

          if (column_types[field] != this_tag)
          {
            if (column_types[field] == none_tag)
              column_types[field] = this_tag;
            else
              column_types[field] = skip_tag;  // type is inconsistent, remove
          }                                    // column from consideration
        }

      if (checked++ > 20)
        break;
    }

  // Finally, compare results against first row and "vote" on whether it's a
  // header.
  int has_header(0);

  for (std::size_t field(0); field < columns; ++field)
    switch (column_types[field])
    {
    case none_tag:
      if (header[field].length())
        ++has_header;
      else
        --has_header;
      break;

    case skip_tag:
      break;

    case number_tag:
      if (!is_number(header[field]))
        ++has_header;
      else
        --has_header;
      break;

    default:  // column containing fixed length strings
      assert(column_types[field] > 0);
      if (const auto length = static_cast<std::size_t>(column_types[field]);
          header[field].length() != length)
        ++has_header;
      else
        --has_header;
    }

  ret.has_header = has_header > 0;

  // is.clear() is not required anymore (C++11)
  is.seekg(0, std::ios::beg);  // back to the start!

  return ret;
}

///
/// Initializes theparser trying to sniff the CSV format.
///
/// \param[in] is input stream containing CSV data
///
csv_parser::csv_parser(std::istream &is) : csv_parser(is, {})
{
  dialect_ = csv_sniffer(is);
}

///
/// Initializes theparser trying to sniff the CSV format.
///
/// \param[in] is input stream containing CSV data
/// \param[in] d  dialect used for CSV data
///
csv_parser::csv_parser(std::istream &is, const csv_dialect &d)
  : is_(&is), filter_hook_(nullptr), dialect_(d)
{
}

///
/// \return a constant reference to the active CSV dialect
///
const csv_dialect &csv_parser::dialect() const
{
  return dialect_;
}

///
/// \param[in] delim separator character for fields
/// \return          a reference to `this` object (fluent interface)
///
csv_parser &csv_parser::delimiter(char delim) &
{
  dialect_.delimiter = delim;
  return *this;
}
csv_parser csv_parser::delimiter(char delim) &&
{
  dialect_.delimiter = delim;
  return *this;
}

///
/// \param[in] t if `true` trims leading and trailing spaces adjacent to
///              commas
/// \return      a reference to `this` object (fluent interface)
///
/// \remark
/// Trimming spaces is contentious and in fact the practice is specifically
/// prohibited by RFC 4180, which states: *spaces are considered part of a
/// field and should not be ignored*.
///
csv_parser &csv_parser::trim_ws(bool t) &
{
  dialect_.trim_ws = t;
  return *this;
}
csv_parser csv_parser::trim_ws(bool t) &&
{
  dialect_.trim_ws = t;
  return *this;
}

///
/// \param[in] filter a filter function for CSV records
/// \return           a reference to `this` object (fluent interface)
///
/// \note A filter function returns `true` for records to be keep.
///
/// \warning
/// Usually, in C++, a fluent interface returns a **reference**.
/// Here we return a **copy** of `this` object. The design decision is due to
/// the fact that a `csv_parser' is a sort of Python generator and tends to
/// be used in for-loops.
/// Users often write:
///
///     for (auto record : csv_parser(f).filter_hook(filter)) { ... }
///
/// but that's broken (it only works if `filter_hook` returns by value).
/// `csv_parser` is a lighweight parser and this shouldn't be a performance
/// concern.
///
/// \see <http://stackoverflow.com/q/10593686/3235496>.
///
csv_parser &csv_parser::filter_hook(filter_hook_t filter) &
{
  filter_hook_ = filter;
  return *this;
}
csv_parser csv_parser::filter_hook(filter_hook_t filter) &&
{
  filter_hook_ = filter;
  return *this;
}

///
/// \return an iterator to the first record of the CSV file
///
csv_parser::const_iterator csv_parser::begin() const
{
  return *is_ ? const_iterator(is_, filter_hook_, dialect_)
              : end();
}

///
/// \return an iterator used as sentry value to stop a cycle
///
csv_parser::const_iterator csv_parser::end() const
{
  return const_iterator();
}

///
/// \return the next record of the CSV file
///
void csv_parser::const_iterator::get_input()
{
  if (!ptr_)
  {
    *this = const_iterator();
    return;
  }

  do
  {
    std::string line;

    do  // gets the first non-empty line
      if (!std::getline(*ptr_, line))
      {
        *this = const_iterator();
        return;
      }
    while (trim(line).empty());

    value_ = parse_line(line);
  } while (filter_hook_ && !filter_hook_(value_));
}

///
/// This function parses a line of data by a delimiter.
///
/// \param[in] line line to be parsed
/// \return         a vector where each element is a field of the CSV line
///
/// If you pass in a comma as your delimiter it will parse out a
/// Comma Separated Value (*CSV*) file. If you pass in a `\t` char it will
/// parse out a tab delimited file (`.txt` or `.tsv`). CSV files often have
/// commas in the actual data, but accounts for this by surrounding the data in
/// quotes. This also means the quotes need to be parsed out, this function
/// accounts for that as well.
///
/// \note
/// This is a slightly modified version of the function at
/// <http://www.zedwood.com/article/112/cpp-csv-parser>. A simpler
/// implementation is <http://stackoverflow.com/a/1120224/3235496> (but it
/// *doesn't escape comma and newline*).
///
/// \note
/// Escaped List Separator class from Boost C++ libraries is also very nice
/// and efficient for parsing, but it isn't as easily applied.
///
csv_parser::const_iterator::value_type csv_parser::const_iterator::parse_line(
  const std::string &line)
{
  value_type record;  // the return value

  const char quote('"');

  bool inquotes(false);
  std::string curstring;

  const auto &add_field = [&](const std::string &field) -> void
  {
    record.push_back(dialect_.trim_ws ? trim(field) : field);
  };

  for (auto length(line.length()), pos(decltype(length){0});
       pos < length && line[pos];)
  {
    const char c(line[pos]);

    if (!inquotes && !curstring.length() && c == quote)  // begin quote char
      inquotes = true;
    else if (inquotes && c == quote)
    {
      if (pos + 1 < length && line[pos + 1] == quote)  // quote char
      {
        // Encountered 2 double quotes in a row (resolves to 1 double quote).
        curstring.push_back(c);
        ++pos;
      }
      else  // end quote char
        inquotes = false;
    }
    else if (!inquotes && c == dialect_.delimiter)  // end of field
    {
      add_field(curstring);
      curstring = "";
    }
    else if (!inquotes && (c == '\r' || c == '\n'))
      break;
    else
      curstring.push_back(c);

    ++pos;
  }

  assert(!inquotes);

  add_field(curstring);

  return record;
}

}  // namespace vita
