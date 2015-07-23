/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2012-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  Original idea by Jean Daniel Michaud (http://code.google.com/p/clipo/).
 *  Code used under MPL2 licence with author's permission.
 */

#if !defined(VITA_BOOST_COMMAND_LINE_INTERPRETER)
#define      VITA_BOOST_COMMAND_LINE_INTERPRETER

#include <vector>
#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include "kernel/vita.h"

namespace vita {
namespace cli {

typedef boost::program_options::options_description commands_description;

///
/// This is the the boost::split_winmain function developed by Vladimir Prus.
///
static std::vector<std::string> split_command_line(const std::string &input)
{
  std::vector<std::string> result;

  std::string::const_iterator i(input.cbegin());
  const std::string::const_iterator e(input.cend());
  while (i != e && std::isspace(*i))
    ++i;

  if (i != e)
  {
    std::string current;
    bool inside_quoted(false);
    unsigned backslash_count(0);

    for (; i != e; ++i)
      if (*i == '"')
      {
        // '"' preceded by even number (n) of backslashes generates
        // n/2 backslashes and is a quoted block delimiter
        if (backslash_count % 2 == 0)
        {
          current.append(backslash_count / 2, '\\');
          inside_quoted = !inside_quoted;
          // '"' preceded by odd number (n) of backslashes generates
          // (n-1)/2 backslashes and is literal quote.
        }
        else
        {
          current.append(backslash_count / 2, '\\');
          current += '"';
        }

        backslash_count = 0;
      }
      else if (*i == '\\')
        ++backslash_count;
      else
      {
        // Not quote or backslash. All accumulated backslashes should be
        // added
        if (backslash_count)
        {
          current.append(backslash_count, '\\');
          backslash_count = 0;
        }

        if (isspace(*i) && !inside_quoted)
        {
          // Space outside quoted section terminate the current argument
          result.push_back(current);
          current.resize(0);
          for (; i != e && isspace(*i); ++i)
          {}

          --i;
        }
        else
          current += *i;
      }

    // If we have trailing backslashes, add them
    if (backslash_count)
      current.append(backslash_count, '\\');

    // If we have non-empty 'current' or we're still in quoted
    // section (even if 'current' is empty), add the last token.
    if (!current.empty() || inside_quoted)
      result.push_back(current);
  }

  return result;
}

class command_line_interpreter
{
public:
  explicit command_line_interpreter(const commands_description &desc)
    : desc_(&desc) {}

  command_line_interpreter(const commands_description &desc,
                           const std::string &prompt)
    : desc_(&desc), prompt_(prompt) {}

  void interpret(std::istream &input_stream)
  {
    std::cout << prompt_ << std::flush;

    std::string command;
    while (std::getline(input_stream, command, '\n'))
    {
      handle_read_line(command);
      std::cout << prompt_ << std::flush;
    }
  }

private:
  void handle_read_line(const std::string &line)
  {
    // huu, ugly...
    std::vector<std::string> args(split_command_line(std::string("--") + line));

    try
    {
      boost::program_options::variables_map vm;
      boost::program_options::store(
        boost::program_options::command_line_parser(args).options(*desc_).
        run(), vm);
      boost::program_options::notify(vm);
    }
    catch (boost::program_options::ambiguous_option &e)
    {
      std::cerr << vita::k_s_error << " Ambiguous command '"
                << boost::program_options::strip_prefixes(e.get_option_name())
                << "'\n";
    }
    catch (boost::program_options::invalid_command_line_syntax &e)
    {
      std::cerr << vita::k_s_error << " The required argument for option '"
                << boost::program_options::strip_prefixes(e.get_option_name())
                << "' is missing\n";
    }
    catch (boost::program_options::unknown_option &e)
    {
      std::cerr << vita::k_s_error << " Unknown command '"
                << boost::program_options::strip_prefixes(e.get_option_name())
                << "'\n";
    }
    catch (boost::program_options::error &e)
    {
      std::cerr << vita::k_s_error << ' ' << e.what() << '\n';
    }
  }

private:
  const commands_description *desc_;
  std::string               prompt_;
};

}  // namespace cli
}  // namespace vita

#endif // Include guard
