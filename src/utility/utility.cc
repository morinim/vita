/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "utility/utility.h"

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

///
/// \return an iterator to the first record of the CSV file.
///
csv_parser::const_iterator csv_parser::begin() const
{
  return *is_ ? const_iterator(is_, filter_hook_, delimiter_, trim_ws_)
              : end();
}

///
/// \return an iterator used as sentry value to stop a cycle.
///
csv_parser::const_iterator csv_parser::end() const
{
  return const_iterator();
}

///
/// \return the next record of the CSV file.
///
void csv_parser::const_iterator::get_input()
{
  try
  {
    if (!ptr_)
      throw get_input_fail::missing_istream;

    do
    {
      std::string line;
      if (!std::getline(*ptr_, line))
        throw get_input_fail::eof;

      value_ = parse_line(line);
    } while (filter_hook_ && !filter_hook_(&value_));
  }
  catch (get_input_fail)
  {
    *this = const_iterator();
  }
}

///
/// \param[in] line line to be parsed.
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

  if (trim_ws_)
    for (auto size(record.size()), i(decltype(size){0}); i < size; ++i)
      trim(record[i]);

  return record;
}


///
/// \param[out] p output xml printer
/// \param[in] e xml element
/// \param[in] s xml element's value
///
/// Specialization for `std::string`.
///
void push_text(tinyxml2::XMLPrinter &p, const std::string &e,
               const std::string &s)
{
  p.OpenElement(e.c_str());
  p.PushText(s.c_str());
  p.CloseElement();
}

///
/// \param[out] p output xml printer
/// \param[in] e xml element
/// \param[in] v xml element's value
///
/// Specialization for `int`.
///
void push_text(tinyxml2::XMLPrinter &p, const std::string &e, int v)
{
  p.OpenElement(e.c_str());
  p.PushText(v);
  p.CloseElement();
}

///
/// \param[out] p output xml printer
/// \param[in] e xml element
/// \param[in] v xml element's value
///
/// Specialization for `double`.
///
void push_text(tinyxml2::XMLPrinter &p, const std::string &e, double v)
{
  p.OpenElement(e.c_str());
  p.PushText(v);
  p.CloseElement();
}

///
/// \param[out] p parent element
/// \param[in] e new xml element
/// \param[in] s new xml element's value
///
/// Specialization for `std::string`.
///
void set_text(tinyxml2::XMLElement *p, const std::string &e,
              const std::string &s)
{
  auto *pe(p->GetDocument()->NewElement(e.c_str()));
  pe->SetText(s.c_str());
  p->InsertEndChild(pe);
}

///
/// \param[out] p parent element
/// \param[in] e new xml element
/// \param[in] v new xml element's value
///
/// Specialization for `int`.
///
void set_text(tinyxml2::XMLElement *p, const std::string &e, int v)
{
  auto *pe(p->GetDocument()->NewElement(e.c_str()));
  pe->SetText(v);
  p->InsertEndChild(pe);
}

///
/// \param[out] p parent element
/// \param[in] e new xml element
/// \param[in] v new xml element's value
///
/// Specialization for `double`.
///
void set_text(tinyxml2::XMLElement *p, const std::string &e, double v)
{
  auto *pe(p->GetDocument()->NewElement(e.c_str()));
  pe->SetText(v);
  p->InsertEndChild(pe);
}

}  // namespace vita
