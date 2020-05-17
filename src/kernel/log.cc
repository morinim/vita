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

#include <fstream>
#include <iomanip>
#include <iostream>

#include "kernel/log.h"

#define HAS_UNCAUGHT_EXCEPTIONS 1
#include "third_party/date/date.h"

namespace vita
{

log::level log::reporting_level = log::lALL;
std::unique_ptr<std::ostream> log::stream = nullptr;

///
/// Creates a `log` object.
///
log::log() : os(), level_(lOUTPUT) {}

///
/// Sets the logging level of a message.
///
/// \param[in] l logging level of the following message
///
/// The following code:
///
///     log().get(level) << "Hello " << username;
///
/// creates a `log` object with the `level` logging level, fetches its
/// `std::stringstream` object, formats and accumulates the user-supplied data
/// and, finally:
/// - prints the resulting string on `std::cout`;
/// - persists the resulting string into the log file (if specified).
///
std::ostringstream &log::get(level l)
{
  level_ = std::min(lFATAL, l);
  return os;
}

log::~log()
{
  static const std::string tags[] =
  {
    "ALL", "DEBUG", "INFO", "", "WARNING", "ERROR", "FATAL", ""
  };

  if (stream)  // `stream`, if available, gets all the messages
  {
    const auto tp(std::chrono::system_clock::now());
    const date::year_month_day d(date::floor<date::days>(tp));

    *stream << date::format("%T", d) << '\t' << tags[level_] << '\t'
            << os.str() << std::endl;
  }

  if (level_ >= reporting_level)  // `cout` is selective
  {
    std::string tag;
    if (level_ != lOUTPUT)
      tag = "[" + tags[level_] + "] ";

    std::cout << tag << os.str() << std::endl;
  }
}

///
/// Sets the `log::stream` variable with a convenient value.
///
/// \param[in] base base filepath of the log (e.g. `/home/doe/app`)
///
/// Given the `/home/doe/app` argument associates the `log::stream` variable
/// with the `app_123_18_30_00.log` file (the numbers represents the current:
/// day of the year, hours, minutes, seconds) in the `/home/doe/` directory.
///
void log::setup_stream(const std::string &base)
{
  const auto tp(std::chrono::system_clock::now());
  const date::year_month_day d(date::floor<date::days>(tp));

  std::ostringstream fn;
  fn << base << date::format("_%j_%H_%M_%S", d) << ".log";

  stream = std::make_unique<std::ofstream>(fn.str());
}

}  // namespace vita
