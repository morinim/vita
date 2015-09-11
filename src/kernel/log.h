/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LOG_H)
#define      VITA_LOG_H

#include <iostream>

#include "kernel/vita.h"

namespace vita
{

///
/// \brief A basic console logger
///
class log
{
public:
  /// \brief The log level
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
  enum level : unsigned {OFF, FATAL, ERROR, WARNING, OUTPUT, INFO, DEBUG, ALL};

  log();

  log &verbosity(level);
  level verbosity() const;

  template<class... Args>
  void fatal(Args &&... args) const
  {
    if (level_ >= FATAL)
    {
      std::cerr << "[FATAL] ";
      to_<std::cerr>(std::forward<Args>(args)...);
    }
  }

#if defined(NDEBUG)
  template<class... Args> void debug(Args &&...) const {}
#else
  template<class... Args>
  void debug(Args &&... args) const
  {
    if (level_ >= DEBUG)
    {
      std::cout << "[DEBUG] ";
      to_<std::cout>(std::forward<Args>(args)...);
    }
  }
#endif

  template<class... Args>
  void error(Args &&... args) const
  {
    if (level_ >= ERROR)
    {
      std::cerr << "[ERROR] ";
      to_<std::cerr>(std::forward<Args>(args)...);
    }
  }

  template<class... Args>
  void info(Args &&... args) const
  {
    if (level_ >= INFO)
    {
      std::cout << "[INFO] ";
      to_<std::cout>(std::forward<Args>(args)...);
    }
  }

  template<class... Args>
  void output(Args &&... args) const
  {
    if (level_ >= OUTPUT)
      to_<std::cout>(std::forward<Args>(args)...);
  }

  template<class... Args>
  void warning(Args &&... args) const
  {
    if (level_ >= WARNING)
    {
      std::cout << "[WARNING] ";
      to_<std::cout>(std::forward<Args>(args)...);
    }
  }

private:
  template<std::ostream &Out, class Head, class... Tail>
  void to_(Head &&head, Tail &&... tail) const
  {
    Out << std::forward<Head>(head);
    to_<Out>(std::forward<Tail>(tail)...);
  }
  template<std::ostream &Out>
  void to_() const { Out << '\n'; }

private:  // Private data member
  // Current log level.
  level level_;
};

extern log print;
}  // namespace vita
#endif  // Include guard
