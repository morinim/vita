/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2017 EOS di Manlio Morini.
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
/// \return       `true` if all elements in both strings are same (case
///               insensitively)
///
bool iequals(const std::string &lhs, const std::string &rhs)
{
  return std::equal(
    lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
    [](auto c1, auto c2) { return std::tolower(c1) == std::tolower(c2); });
}

///
/// \param[in] s the input string
/// \return      a copy of `s` with spaces removed on both sides of the string
///
/// \see http://stackoverflow.com/a/24425221/3235496
///
std::string trim(const std::string &s)
{
  auto front = std::find_if_not(s.begin(), s.end(),
                                [](auto c) { return std::isspace(c); });

  // The search is limited in the reverse direction to the last non-space value
  // found in the search in the forward direction.
  auto back = std::find_if_not(s.rbegin(), std::make_reverse_iterator(front),
                               [](auto c) { return std::isspace(c); }).base();

  return {front, back};
}

///
/// Replaces all occurrences of a string with another string.
///
/// \param[in] s    input string
/// \param[in] from substring to be searched for
/// \param[in] to   substitute string
/// \return         the modified input
///
std::string replace_all(std::string s,
                        const std::string &from, const std::string &to)
{
  if (!from.empty())
  {
    std::size_t start(0);
    while ((start = s.find(from, start)) != std::string::npos)
    {
      s.replace(start, from.length(), to);
      start += to.length();  // in case `to` contains `from`, like replacing
                             // "x" with "yx"
    }
  }

  return s;

  // With std::regex it'd be something like:
  //     s = std::regex_replace(s, std::regex(from), to);
  // (possibly escaping special characters in the `from` string)
}

///
/// A convenient arrangement for inserting strings into `XMLDocument`.
///
/// \param[out] p parent element
/// \param[in]  e new xml element
/// \param[in]  s new xml element's value
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

}  // namespace vita
