/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "utility/csv_parser.h"
#include "utility/utility.h"

namespace vita
{

///
/// \return an iterator to the first record of the CSV file
///
csv_parser::const_iterator csv_parser::begin() const
{
  return *is_ ? const_iterator(is_, filter_hook_, delimiter_, trim_ws_)
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
    record.push_back(trim_ws_ ? trim(field) : field);
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
    else if (!inquotes && c == delimiter_)  // end of field
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
