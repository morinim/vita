/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_COMPATIBILITY_PATCH_H)
#define      VITA_COMPATIBILITY_PATCH_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
// The `windows.h` header file (or more correctly, `windef.h` that it includes
// in turn) has macros for `min` and `max` that are interfering with
// `std::min`/`max` and `std::numeric_limits<T>::min`/`max`.
// The `NOMINMAX` macro suppresses the `min` and `max` definitions in
// `Windef.h`. The `undef` limits potential side effects.
#  define NOMINMAX
#  include <conio.h>
#  include <windows.h>
#  undef NOMINMAX
#else
#  include <iostream>
#  include <termios.h>
#  include <unistd.h>
#endif

namespace vita
{

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32)
///
/// \param[in] enter if `true` sets the terminal raw mode, else restore the
///                  default terminal mode
///
/// The raw mode discipline performs no line editing and the control
/// sequences for both line editing functions and the various special
/// characters ("interrupt", "quit", and flow control) are treated as normal
/// character input. Applications programs reading from the terminal receive
/// characters immediately and receive the entire character stream unaltered,
/// just as it came from the terminal device itself.
///
inline void term_raw_mode(bool enter)
{
  static termios oldt, newt;

  if (enter)
  {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~static_cast<unsigned>(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

///
/// \return `true` if the user press a key (`false` otherwise)
///
inline bool kbhit()
{
  // Do not wait at all, not even a microsecond.
  timeval tv;
  tv.tv_sec  = 0;
  tv.tv_usec = 0;

  fd_set readfd;
  FD_ZERO(&readfd);  // initialize `readfd`
  FD_SET(STDIN_FILENO, &readfd);

  // The first parameter is the number of the largest file descriptor to
  // check + 1.
  if (select(STDIN_FILENO + 1, &readfd, nullptr, nullptr, &tv) == -1)
    return false;  // an error occurred
  // `read_fd` now holds a bit map of files that are readable. We test the
  // entry for the standard input (file 0).
  return FD_ISSET(STDIN_FILENO, &readfd);
}

inline bool keypressed(int k) { return kbhit() && std::cin.get() == k; }

#else

inline void term_raw_mode(bool) {}
inline bool keypressed(int k) { return _kbhit() && _getch() == k; }
#endif

}  // namespace vita

#endif  // include guard
