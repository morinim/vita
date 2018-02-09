/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2018 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <chrono>
#include <iomanip>
#include <iostream>

#include "kernel/log.h"

namespace vita
{

log::level log::reporting_level = log::ALL;
std::ostream *log::file = nullptr;

///
/// Creates a `log` object.
///
log::log() : os(), level_(OUTPUT) {}

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
  level_ = l;
  return os;
}

log::~log()
{
  static const std::string tags[] =
  {
    "ALL", "DEBUG", "INFO", "", "WARNING", "ERROR", "FATAL", ""
  };

  if (file)
  {
    const auto now(std::chrono::system_clock::now());
    const auto tt(std::chrono::system_clock::to_time_t(now));
    const auto lt(*std::localtime(&tt));

    *file << "[" + tags[level_] + "] " << std::put_time(&lt, "%T") << ' '
          << os.str() << std::endl;
  }

  if (level_ >= reporting_level)
  {
    std::string tag;
    if (level_ != OUTPUT)
      tag = "[" + tags[level_] + "] ";

    std::cout << tag << os.str() << std::endl;
  }
}

}  // namespace vita
