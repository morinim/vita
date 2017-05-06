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

#if !defined(VITA_LOG_H)
#define      VITA_LOG_H

#include <iostream>

#include "kernel/common.h"

namespace vita
{

///
/// \brief A basic console logger.
///
class log
{
public:
  /// \brief The log level.
  ///
  /// * `OFF`     - No output
  /// * `FATAL`   - The program cannot continue
  /// * `ERROR`   - Something really wrong... but you could be lucky
  /// * `WARNING` - I can continue but please have a look
  /// * `OUTPUT`  - Standard program's console output
  /// * `INFO`    - I say something but I don't expect you to listen
  /// * `DEBUG`   - Only interesting for developers
  ///
  /// \remarks
  /// The `DEBUG` log level can be switched on only if the `NDEBUG` macro is
  /// defined.
  enum level : unsigned {L_OFF, L_FATAL, L_ERROR, L_WARNING, L_OUTPUT,
                         L_INFO, L_DEBUG, L_ALL};

  log();

  log &verbosity(level);
  level verbosity() const;

  template<class... Args>
  void fatal(Args &&... args) const
  {
    print_if_(std::cerr, L_FATAL, "FATAL", std::forward<Args>(args)...);
  }

#if defined(NDEBUG)
  template<class... Args> void debug(Args &&...) const {}
#else
  template<class... Args>
  void debug(Args &&... args) const
  {
    print_if_(std::cout, L_DEBUG, "DEBUG", std::forward<Args>(args)...);
  }
#endif

  template<class... Args>
  void error(Args &&... args) const
  {
    print_if_(std::cerr, L_ERROR, "ERROR", std::forward<Args>(args)...);
  }

  template<class... Args>
  void info(Args &&... args) const
  {
    print_if_(std::cout, L_INFO, "INFO", std::forward<Args>(args)...);
  }

  template<class... Args>
  void output(Args &&... args) const
  {
    print_if_(std::cout, L_OUTPUT, "", std::forward<Args>(args)...);
  }

  template<class... Args>
  void warning(Args &&... args) const
  {
    print_if_(std::cout, L_WARNING, "WARNING", std::forward<Args>(args)...);
  }

private:
  template<class... Args>
  void print_if_(std::ostream &out, level l, const std::string &tag,
                 Args &&... args) const
  {
    if (level_ >= l)
    {
      if (!tag.empty())
        out << "[" << tag << "] ";
      to_(out, std::forward<Args>(args)...);
    }
  }

  template<class Head, class... Tail>
  void to_(std::ostream &out, Head &&head, Tail &&... tail) const
  {
    out << std::forward<Head>(head);
    to_(out, std::forward<Tail>(tail)...);
  }
  void to_(std::ostream &out) const { out << '\n'; }

  // Current log level.
  level level_;
};

extern log print;

}  // namespace vita
#endif  // include guard
