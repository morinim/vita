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
/// \param[in] s1 first term of comparison.
/// \param[in] s2 second term of comparison.
/// \return `true` if all elements in both strings are same (case
///         insensitively).
///
bool iequals(const std::string &s1, const std::string &s2)
{
  return std::equal(
    s1.begin(), s1.end(), s2.begin(), s2.end(),
    [](auto c1, auto c2) { return std::tolower(c1) == std::tolower(c2); });
}

///
/// \param[in] s the input string.
/// \return a copy of `s` with spaces removed on both sides of the string.
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
/// \param[out] p parent element.
/// \param[in] e new xml element.
/// \param[in] s new xml element's value.
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
