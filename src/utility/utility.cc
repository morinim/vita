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
    s1.begin(), s1.end(), s2.begin(), s2.end(),
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

}  // namespace vita
