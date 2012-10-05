/**
 *
 *  \file timer.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(TIMER_H)
#define      TIMER_H

#include <chrono>

namespace vita
{
  ///
  /// We always run into the task of measuring the time between two points.
  /// This would be easy to do with C++11 (but quite verbose).
  ///
  /// The vita::timer class cuts down the verbose syntax needed to measure
  /// elapsed time. It is similar to boost::cpu_timer but tailored to our
  /// needs (so less general).
  ///
  /// The simplest and most common use is:
  /// \code{.cc}
  ///   #include <kernel/timer.h>
  ///   #include <cmath>
  ///
  ///   int main()
  ///   {
  ///     vita::timer t;
  ///
  ///     for (long i(0); i < 100000000; ++i)
  ///       std::sqrt(123.456L); // burn some time
  ///
  ///     std::cout << "Elapsed (milliseconds): " << t.elapsed() << std::endl
  ///
  ///     return 0;
  ///   }
  /// \endcode
  ///
  /// \remark
  /// The original idea is of Kjellkod (http://kjellkod.wordpress.com).
  ///
  class timer
  {
  public:
    timer() : start_(std::chrono::steady_clock::now()) {}

    void restart() { start_ = std::chrono::steady_clock::now(); }

    ///
    /// \return time elapsed in milliseconds (as would be measured by a clock
    ///         on the wall. It's NOT the processor time).
    ///
    double elapsed() const
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_).count();
    }

  private:
    std::chrono::steady_clock::time_point start_;
  };
}  // namespace vita

#endif  // TIMER_H
