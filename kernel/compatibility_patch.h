/**
 *
 *  \file compatibility_patch.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

//#pragma GCC diagnostic ignored "-Wformat"

#if !defined(COMPATIBILITY_PATCH_H)
#define      COMPATIBILITY_PATCH_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#  include <conio.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif

namespace vita
{
#if defined(_MSC_VER)
#  define ROTL64(x, y)  _rotl64(x, y)
#else
   ///
   /// \param[in] x unsigned 64-bit to be rotated.
   /// \param[in] r number of steps.
   /// \return the value corresponding to rotating the bits of \a x \a r-steps
   ///         to the right (r must be between 1 to 31 inclusive).
   ///
   inline std::uint64_t rotl64(std::uint64_t x, std::uint8_t r)
   {
     return (x << r) | (x >> (64 - r));
   }
#  define ROTL64(x, y)  rotl64(x, y)
#endif

  // A way to hide warnings about variables only used in compile time asserts.
  // There are GCC compiler flags that control unused warnings, but I want a
  // selective behaviour (generally it is useful to check for dead code).
#if defined(__GNUC__)
#  define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#  define VARIABLE_IS_NOT_USED
#endif

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32)
  inline bool kbhit()
  {
    // Do not wait at all, not even a microsecond.
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_set readfd;
    FD_ZERO(&readfd);    // initialize readfd
    FD_SET(0, &readfd);  // 0 is the file descriptor for stdin

    // The first parameter is the number of the largest file descriptor to
    // check + 1.
    if (select(1, &readfd, nullptr, nullptr, &tv) == -1)  // an error occured
      return false;

    // read_fd now holds a bit map of files that are readable. We test the
    // entry for the standard input (file 0).
    return FD_ISSET(0, &readfd);

/*
    // An alternative (requires <sys/ioctl.h> and <termios.h> also).

    static const int STDIN(0);
    static bool initialized(false);

    if (!initialized)
    {
      // Use termios to turn off line buffering.
      termios term;
      tcgetattr(STDIN, &term);
      term.c_lflag &= ~ICANON;
      tcsetattr(STDIN, TCSANOW, &term);
      setbuf(stdin, nullptr);
      initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
*/
  }
#endif
}  // namespace vita

#endif  // COMPATIBILITY_PATCH_H
