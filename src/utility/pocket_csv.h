/**
 *  \file
 *  \remark This file is part of POCKET_CSV.
 *
 *  \copyright Copyright (C) 2016, 2022 Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(POCKET_CSV_PARSER_H)
#define      POCKET_CSV_PARSER_H

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>

namespace pocket_csv
{

///
/// Information about the CSV dialect.
///
/// *CSV is a textbook example of how not to design a textual file format*.
/// The Art of Unix Programming, Raymond (2003).
///
struct dialect
{
  /// A one-character string used to separate fields. When `0` triggers the
  /// sniffer.
  char delimiter = 0;
  /// When `true` skips leading and trailing spaces adjacent to commas.
  bool trim_ws = false;
  /// When `HAS_HEADER` assumes a header row is present. When `GUESS_HEADER`
  /// triggers the sniffer.
  enum header_e {GUESS_HEADER = -1, NO_HEADER = 0, HAS_HEADER = 1} has_header
    = GUESS_HEADER;
  /// Controls if quotes should be keep by the reader.
  /// - `KEEP_QUOTES`. Always keep the quotes;
  /// - `REMOVE_QUOTES`. Never keep quotes.
  /// It defaults to `REMOVE_QUOTES`.
  enum quoting_e {KEEP_QUOTES, REMOVE_QUOTES} quoting = REMOVE_QUOTES;
};  // class dialect

///
/// Simple parser for CSV files.
///
/// \warning The class doesn't support multi-line fields.
///
class parser
{
public:
  using record_t = std::vector<std::string>;
  using filter_hook_t = std::function<bool (record_t &)>;

  explicit parser(std::istream &);
  parser(std::istream &, const dialect &);

  [[nodiscard]] const dialect &active_dialect() const;

  parser &delimiter(char) &;
  parser delimiter(char) &&;

  parser &quoting(dialect::quoting_e) &;
  parser quoting(dialect::quoting_e) &&;

  parser &trim_ws(bool) &;
  parser trim_ws(bool) &&;

  parser &filter_hook(filter_hook_t) &;
  parser filter_hook(filter_hook_t) &&;

  class const_iterator;
  [[nodiscard]] const_iterator begin() const;
  [[nodiscard]] const_iterator end() const;

private:
  // DO NOT move this. `is` must be initialized before other data members.
  std::istream *is_;

  filter_hook_t filter_hook_;
  dialect dialect_;
};  // class parser

///
/// A forward iterator for CSV records.
///
class parser::const_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = parser::record_t;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  const_iterator(std::istream *is = nullptr,
                 parser::filter_hook_t f = nullptr,
                 const dialect &d = {})
    : ptr_(is), filter_hook_(f), dialect_(d), value_()
  {
    if (ptr_)
      get_input();
  }

  /// \return an iterator pointing to the next record of the CSV file
  const_iterator &operator++()
  {
    get_input();
    return *this;
  }

  /// \return reference to the current record of the CSV file
  const_reference operator*() const { return value_; }

  /// \return pointer to the current record of the CSV file
  const_pointer operator->() const { return &operator*(); }

  /// \param[in] lhs first term of comparison
  /// \param[in] rhs second term of comparison
  /// \return        `true` if iterators point to the same line
  friend bool operator==(const const_iterator &lhs, const const_iterator &rhs)
  {
    return lhs.ptr_ == rhs.ptr_ && lhs.value_ == rhs.value_
           && (!lhs.ptr_ || lhs.ptr_->tellg() == rhs.ptr_->tellg());
  }

  friend bool operator!=(const const_iterator &lhs, const const_iterator &rhs)
  {
    return !(lhs == rhs);
  }

private:
  value_type parse_line(const std::string &);
  void get_input();

  // Data members MUST BE INITIALIZED IN THE CORRECT ORDER:
  // * `value_` is initialized via the `get_input` member function;
  // * `get_input` works only if `_ptr` is already initialized.
  // So it's important that `value_` is the last value to be initialized.
  std::istream *ptr_;
  parser::filter_hook_t filter_hook_;
  dialect dialect_;
  value_type value_;
};  // class parser::const_iterator

namespace detail
{

enum column_tag {none_tag = 0, skip_tag = -1,
                 number_tag = -2, string_tag = -3};

struct char_stat
{
  char_stat(unsigned cf = 0, unsigned w = 0) : char_freq(cf), weight(w) {}

  unsigned char_freq;
  unsigned weight;
};

///
/// \param[in] s the input string
/// \return      a copy of `s` with spaces removed on both sides of the string
///
/// \see http://stackoverflow.com/a/24425221/3235496
///
[[nodiscard]] inline std::string trim(const std::string &s)
{
  const auto front(std::find_if_not(
               s.begin(), s.end(),
               [](auto c) { return std::isspace(c); }));

  // The search is limited in the reverse direction to the last non-space value
  // found in the search in the forward direction.
  const auto back(std::find_if_not(
                    s.rbegin(), std::make_reverse_iterator(front),
                    [](auto c) { return std::isspace(c); }).base());

  return {front, back};
}

///
/// \param[in] s the string to be tested
/// \return      `true` if `s` contains a number
///
[[nodiscard]] inline bool is_number(std::string s)
{
  s = trim(s);

  char *end;
  strtod(s.c_str(), &end);  // if no conversion can be performed, `end` is set
                            // to `s.c_str()`
  return end != s.c_str() && *end == '\0';
}

///
/// Calculates the mode of a sequence of natural numbers.
///
/// \param[in] v a sequence of natural number
/// \return    a vector of {mode, counter} pairs (the input sequence
///            may have more than one mode)
///
/// \warning
/// Assumes a sorted input vector.
///
[[nodiscard]] inline std::vector<char_stat> mode(const std::vector<unsigned> &v)
{
  assert(std::is_sorted(v.begin(), v.end()));

  if (v.empty())
    return {};

  auto current(*v.begin());
  unsigned count(1), max_count(1);

  std::vector<char_stat> ret({{current, 1}});

  for (auto i(std::next(v.begin())); i != v.end(); ++i)
  {
    if (*i == current)
      ++count;
    else
    {
      count = 1;
      current = *i;
    }

    if (count > max_count)
    {
      max_count = count;
      ret = {{current, max_count}};
    }
    else if (count == max_count)
      ret.emplace_back(current, max_count);
  }

  return ret;
}

[[nodiscard]] inline int find_column_tag(const std::string &s)
{
  const auto ts(detail::trim(s));

  if (ts.empty())
    return none_tag;
  if (detail::is_number(ts))
    return number_tag;

  return static_cast<int>(s.length());
}

[[nodiscard]] inline bool capitalized(std::string s)
{
  s = detail::trim(s);

  return !s.empty() && std::isupper(s.front())
         && std::all_of(std::next(s.begin()), s.end(),
                        [](auto c)
                        {
                          return std::isprint(c)
                                 && (!std::isalpha(c) || std::islower(c));
                        });
}

[[nodiscard]] inline bool lower_case(const std::string &s)
{
  return std::all_of(s.begin(), s.end(),
                     [](auto c)
                     {
                       return !std::isalpha(c) || std::islower(c);
                     });
}

[[nodiscard]] inline bool upper_case(const std::string &s)
{
  return std::all_of(s.begin(), s.end(),
                     [](auto c)
                     {
                       return !std::isalpha(c) || std::isupper(c);
                     });
}

[[nodiscard]] inline dialect::header_e has_header(std::istream &is,
                                                  std::size_t lines,
                                                  char delim)
{
  dialect d;
  d.delimiter = delim;
  d.has_header = dialect::HAS_HEADER;  // assume first row is header (1)
  pocket_csv::parser parser(is, d);

  // Quoting allows to correctly identify a column with header `"1980"` (e.g. a
  // specific year. Notice the double quotes) and values `2012`, `2000`...
  // (the values observed during 1980).
  parser.quoting(pocket_csv::dialect::KEEP_QUOTES);
  const auto header(*parser.begin());  // assume first row is header (2)
  parser.quoting(pocket_csv::dialect::REMOVE_QUOTES);

  const auto columns(header.size());
  std::vector<int> column_types(columns, none_tag);

  unsigned checked(0);
  for (auto it(std::next(parser.begin())); it != parser.end(); ++it)
    // Skip rows that have irregular number of columns
    if (const auto row = *it; row.size() == columns)
    {
      for (std::size_t field(0); field < columns; ++field)
      {
        if (column_types[field] == skip_tag)  // inconsistent column
          continue;
        if (trim(row[field]).empty())         // missing values
          continue;

        const auto this_tag(find_column_tag(row[field]));
        if (column_types[field] == this_tag)  // matching column type
          continue;

        if (capitalized(header[field]) && lower_case(row[field]))
          column_types[field] = string_tag;
        else if (upper_case(header[field]) && !upper_case(row[field]))
          column_types[field] = string_tag;
        else if (column_types[field] == none_tag)
          column_types[field] = this_tag;
        else
          column_types[field] = skip_tag;  // type is inconsistent, remove
      }                                    // column from consideration

      if (checked++ > lines)
        break;
    }

  // Finally, compare results against first row and "vote" on whether it's a
  // header.
  int vote_header(0);

  for (std::size_t field(0); field < columns; ++field)
    switch (column_types[field])
    {
    case none_tag:
      if (header[field].length())
        ++vote_header;
      else
        --vote_header;
      break;

    case skip_tag:
      break;

    case number_tag:
      if (!is_number(header[field]))
        ++vote_header;
      else
        --vote_header;
      break;

    case string_tag:  // variable length strings
      ++vote_header;
      break;

    default:  // column containing fixed length strings
      assert(column_types[field] > 0);
      if (const auto length = static_cast<std::size_t>(column_types[field]);
          header[field].length() != length)
        ++vote_header;
      else
        --vote_header;
    }

  is.clear();
  is.seekg(0, std::ios::beg);  // back to the start!

  return vote_header > 0 ? dialect::HAS_HEADER : dialect::NO_HEADER;
}

[[nodiscard]] inline char guess_delimiter(std::istream &is, std::size_t lines)
{
  const std::vector preferred = {',', ';', '\t', ':', '|'};

  // `count[c]` is a vector with information about character `c`. It grows
  // one element every time a new input line is read.
  // `count[c][l]` contains the number of times character `c` appears in line
  // `l`.
  std::map<char, std::vector<unsigned>> count;

  std::size_t scanned(0);

  for (std::string line; std::getline(is, line) && lines;)
  {
    if (trim(line).empty())
      continue;

    // A new non-empty line. Initially every character has a `0` counter.
    for (auto c : preferred)
      count[c].push_back(0u);

    for (auto c : line)
      if (std::find(preferred.begin(), preferred.end(), c) != preferred.end())
        ++count[c].back();

    --lines;
    ++scanned;
  }

  if (count.empty())  // empty input file
    return 0;

  // `mode_weight[c]` stores a couple of values specifying:
  // 1. how many time character `c` usually repeats in a line of the CSV file;
  // 2. a weight (the effective number of lines condition 1 is verified).
  std::map<char, char_stat> mode_weight;

  for (auto &[c, cf] : count)
  {
    std::sort(cf.begin(), cf.end());

    const auto mf(mode(cf));

    if (mf.empty() || mf.size() > 1 || mf.front().char_freq == 0)
      mode_weight[c] = {0u, 0u};
    else
      mode_weight[c] = mf.front();
  }

  const auto res(std::max_element(mode_weight.begin(), mode_weight.end(),
                                  [](const auto &l, const auto &r)
                                  {
                                    return l.second.weight < r.second.weight;
                                  }));

  if (res->second.char_freq == 0)
    return '\n';

  // Delimiter must consistently appear in the input lines.
  if (3 * res->second.weight < 2 * scanned)
    return 0;

  return res->first;
}

}  // detail namespace

///
/// *Sniffs* the format of a CSV file (delimiter, headers).
///
/// \param[in] is stream containing CSV data
/// \return       a `dialect` object
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
/// ---
///
/// The delimiter *should* occur the same number of times on each row. However,
/// due to malformed data, it may not. We don't want an all or nothing
/// approach, so we allow for small variations in this number:
///
/// 1. build a table of the frequency of usual delimiters (comma, tab, colon,
///    semicolon, vertical bar) on every line;
/// 2. build a table of frequencies of this frequency (meta-frequency?), e.g.
///    'x occurred 5 times in 10 rows, 6 times in 1000 rows, 7 times in 2
///    rows';
/// 3. use the mode of the meta-frequency to determine the *expected* frequency
///    for that character;
/// 4. find out how often the character actually meets that goal;
/// 5. the character that best meets its goal is the delimiter.
///
/// \note
/// Somewhat inspired by the dialect sniffer developed by Clifford Wells for
/// his Python-DSV package (Wells, 2002) ehich was incorporated into Python
/// v2.3.
///
[[nodiscard]] inline dialect sniffer(std::istream &is)
{
  const std::size_t lines(20);

  dialect d;

  d.delimiter = detail::guess_delimiter(is, lines);
  d.has_header = detail::has_header(is, lines, d.delimiter);

  return d;
}

///
/// Initializes the parser trying to sniff the CSV format.
///
/// \param[in] is input stream containing CSV data
///
inline parser::parser(std::istream &is) : parser(is, {})
{
  dialect_ = sniffer(is);
}

///
/// Initializes theparser trying to sniff the CSV format.
///
/// \param[in] is input stream containing CSV data
/// \param[in] d  dialect used for CSV data
///
inline parser::parser(std::istream &is, const dialect &d)
  : is_(&is), filter_hook_(nullptr), dialect_(d)
{
}

///
/// \return a constant reference to the active CSV dialect
///
inline const pocket_csv::dialect &parser::active_dialect() const
{
  return dialect_;
}

///
/// \param[in] delim separator character for fields
/// \return          a reference to `this` object (fluent interface)
///
inline parser &parser::delimiter(char delim) &
{
  dialect_.delimiter = delim;
  return *this;
}
inline parser parser::delimiter(char delim) &&
{
  dialect_.delimiter = delim;
  return *this;
}

///
/// \param[in] q quoting style (see `pocket_csv::dialect`)
/// \return      a reference to `this` object (fluent interface)
///
inline parser &parser::quoting(dialect::quoting_e q) &
{
  dialect_.quoting = q;
  return *this;
}

inline parser parser::quoting(dialect::quoting_e q) &&
{
  dialect_.quoting = q;
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
inline parser &parser::trim_ws(bool t) &
{
  dialect_.trim_ws = t;
  return *this;
}
inline parser parser::trim_ws(bool t) &&
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
/// the fact that a `parser' is a sort of Python generator and tends to be used
/// in for-loops.
/// Users often write:
///
///     for (auto record : parser(f).filter_hook(filter)) { ... }
///
/// but that's broken (it only works if `filter_hook` returns by value).
/// `parser` is a lighweight object and this shouldn't have an impact on
/// performance.
///
/// \see <http://stackoverflow.com/q/10593686/3235496>.
///
inline parser &parser::filter_hook(filter_hook_t filter) &
{
  filter_hook_ = filter;
  return *this;
}
inline parser parser::filter_hook(filter_hook_t filter) &&
{
  filter_hook_ = filter;
  return *this;
}

///
/// \return an iterator to the first record of the CSV file
///
inline parser::const_iterator parser::begin() const
{
  assert(is_);

  is_->clear();
  is_->seekg(0, std::ios::beg);  // back to the start!

  return *is_ ? const_iterator(is_, filter_hook_, dialect_)
              : end();
}

///
/// \return an iterator used as sentry value to stop a cycle
///
inline parser::const_iterator parser::end() const
{
  return const_iterator();
}

///
/// \return the next record of the CSV file
///
inline void parser::const_iterator::get_input()
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
    while (detail::trim(line).empty());

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
inline parser::const_iterator::value_type parser::const_iterator::parse_line(
  const std::string &line)
{
  value_type record;  // the return value

  const char quote('"');

  bool inquotes(false);
  std::string curstring;

  const auto &add_field([&record, this](const std::string &field)
                        {
                          record.push_back(dialect_.trim_ws
                                           ? detail::trim(field) : field);
                        });

  const auto length(line.length());
  for (std::size_t pos(0); pos < length && line[pos]; ++pos)
  {
    const auto c(line[pos]);

    if (!inquotes && detail::trim(curstring).empty()
        && c == quote)  // begin quote char
    {
      if (dialect_.quoting == dialect::KEEP_QUOTES)
        curstring.push_back(c);

      inquotes = true;
    }
    else if (inquotes && c == quote)
    {
      if (pos + 1 < length && line[pos + 1] == quote)  // quote char
      {
        // Encountered 2 double quotes in a row (resolves to 1 double quote).
        curstring.push_back(c);
        ++pos;
      }
      else  // end quote char
      {
        if (dialect_.quoting == dialect::KEEP_QUOTES)
          curstring.push_back(c);

        inquotes = false;
      }
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
  }

  assert(!inquotes);

  add_field(curstring);

  return record;
}

}  // namespace pocket_csv

#endif  // include guard
