/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/utility.h"

namespace vita
{
///
/// \param[in] s1 first term of comparison
/// \param[in] s2 second term of comparison
/// \return `true` if all elements in both strings are same (case
///         insensitively).
///
bool iequals(const std::string &s1, const std::string &s2)
{
  return std::equal(
    s1.begin(), s1.end(), s2.begin(),
    [](int c1, int c2) { return std::tolower(c1) == std::tolower(c2); });
}

///
/// \param[in] line line to be parsed.
/// \param[in] delimiter separator character for fields.
/// \param[in] trimws if `true` trims leading and trailing spaces adjacent to
///                   commas (this practice is contentious and in fact is
///                   specifically prohibited by RFC 4180, which states,
///                   "Spaces are considered part of a field and should not be
///                   ignored.").
/// \return a vector where each element is a field of the CSV line.
///
/// This function parses a line of data by a delimiter. If you pass in a
/// comma as your delimiter it will parse out a Comma Separated Value (CSV)
/// file. If you pass in a `\t` char it will parse out a tab delimited file
/// (.txt or .tsv). CSV files often have commas in the actual data, but
/// accounts for this by surrounding the data in quotes. This also means the
/// quotes need to be parsed out, this function accounts for that as well.
///
/// \note
/// This is a slightly modified version of the function at
/// <http://www.zedwood.com/article/112/cpp-csv-parser>.
///
/// \note
/// Escaped List Separator class from Boost C++ libraries is also very nice
/// and efficient for parsing, but it isn't as easily applied.
///
std::vector<std::string> parse_csvline(const std::string &line, char delimiter,
                                       bool trimws)
{
  std::vector<std::string> record;  // the return value

  const char quote('"');

  bool inquotes(false);
  std::string curstring;

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
    else if (!inquotes && c == delimiter)  // end of field
    {
      record.push_back(curstring);
      curstring = "";
    }
    else if (!inquotes && (c == '\r' || c == '\n'))
      break;
    else
      curstring.push_back(c);

    ++pos;
  }

  assert(!inquotes);

  record.push_back(curstring);

  if (trimws)
    for (auto size(record.size()), i(decltype(size){0}); i < size; ++i)
      trim(record[i]);

  return record;
}

///
/// \param[in] s the input string.
/// \return a copy of `s` with spaces removed on both sides of the string.
///
std::string trim(const std::string &s)
{
  auto ws_front = std::find_if_not(s.begin(), s.end(),
                                   [](int c) {return std::isspace(c); });
  auto ws_back = std::find_if_not(s.rbegin(), s.rend(),
                                  [](int c){return std::isspace(c); }).base();

  return ws_back <= ws_front ? std::string() : std::string(ws_front, ws_back);
}

}  // namespace vita
