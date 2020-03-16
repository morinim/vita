/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2020 EOS di Manlio Morini.
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
/// Replaces the first occurrence of a string with another string.
///
/// \param[in] s    input string
/// \param[in] from substring to be searched for
/// \param[in] to   substitute string
/// \return         the modified input
///
std::string replace(std::string s,
                    const std::string &from, const std::string &to)
{
  const auto start_pos(s.find(from));
  if (start_pos != std::string::npos)
    s.replace(start_pos, from.length(), to);

  return s;
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

///
/// Merges two paths.
///
/// \param[in] p1  first part of the path
/// \param[in] p2  second part of the path
/// \param[in] sep path separator character
/// \return        result of path concatenation
///
/// \note
/// Just an approximation of `std::filesystem::path::operator/`. Should be
/// replaced when switching to C++17.
///
std::string merge_path(const std::string &p1, const std::string &p2, char sep)
{
  if (p1.empty()
      || p2.front() == sep)  // absolute (POSIX) path. `p2` replaces `p1`
    return p2;

  // If `p2` is empty append the `sep` char to `p1`

  const auto last_p1(p1.back() == sep ? std::prev(p1.end()) : p1.end());

  return std::string(p1.begin(), last_p1)
         + std::string(1, sep)
         + std::string(p2.begin(), p2.end());
}

///
/// Converts an `value_t` to `double`.
///
/// \param[in] v value that should be converted to `double`
/// \return      the result of the conversion of `v`
///
/// This function is useful for:
/// * debugging purpose;
/// * symbolic regression and classification task (the value returned by
///   the interpeter will be used in a "numeric way").
///
/// \remark If the conversion cannot be performed the function returns `0.0`.
///
template<>
double to<double>(const vita::value_t &v)
{
  // The pointer form of any_cast uses the nullability of pointers (will
  // return a null pointer rather than throw if the cast fails).
  // The alternatives are a.type() == typeid(double)... or try/catch and
  // both seems inferior.
  if (auto *p = std::get_if<D_DOUBLE>(&v))
    return *p;

  if (auto *p = std::get_if<D_INT>(&v))
    return static_cast<D_DOUBLE>(*p);

  return 0.0;
}

///
/// Converts an `value_t` to `std::string`.
///
/// \param[in] v value that should be converted to `std::string`
/// \return      the result of the conversion of `v`
///
/// This function is useful for debugging purpose.
///
template<>
std::string to<std::string>(const vita::value_t &v)
{
  if (auto *p = std::get_if<D_DOUBLE>(&v))
    return std::to_string(*p);

  if (auto *p = std::get_if<D_INT>(&v))
    return std::to_string(*p);

  return std::get<D_STRING>(v);
}

}  // namespace vita
