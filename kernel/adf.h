/**
 *
 *  \file adf.h
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if !defined(ADF_H)
#define      ADF_H

#include <string>
#include <vector>

#include "kernel/function.h"
#include "kernel/individual.h"
#include "kernel/terminal.h"

namespace vita
{
  class interpreter;

  ///
  /// Human programmers organize sequences of repeated steps into reusable
  /// components such as subroutines, functions and classes. They then
  /// repeatedly invoke these components, typically with different inputs.
  /// Reuse eliminates the need to "reinvent the wheel" every time a particular
  /// sequence of steps is needed. Reuse also makes it possible to exploit a
  /// problem's modularities, symmetries and regularities (thereby potentially
  /// accelerate the problem-solving process). This can be taken further, as
  /// programmers typically organise these components into hierarchies in which
  /// top level components call lower level ones, which call still lower levels.
  /// Although the acronym ADF is from Koza's automatically defined functions,
  /// in Vita subroutines are created using the ARL scheme described in
  /// "Discovery of subroutines in genetic programming" - J.P. Rosca and D.H.
  /// Ballard.
  ///
  class adf_core
  {
    friend class adf;
    friend class adt;

    explicit adf_core(const individual &);

    bool check() const;

    const unsigned id;
    individual   code;

    static unsigned adf_count;
  };

  ///
  /// Subroutine with arguments.
  ///
  class adf : public function
  {
  public:
    adf(const individual &, const std::vector<category_t> &, unsigned);

    boost::any eval(interpreter *) const;

    const individual &get_code() const;
    std::string display() const;

    bool auto_defined() const;

    bool check() const;

  private:
    adf_core core_;
  };

  ///
  /// Subroutines WITHOUT arguments (see "An Analysis of Automatic Subroutine
  /// Discovery in Genetic Programming" - A.Dessi', A.Giani, A.Starita>).
  ///
  class adt : public terminal
  {
  public:
    adt(const individual &, unsigned);

    boost::any eval(interpreter *) const;

    const individual &get_code() const;
    std::string display() const;

    bool auto_defined() const;

    bool check() const;

  private:
    adf_core core_;
  };
}  // namespace vita

#endif  // ADF_H
