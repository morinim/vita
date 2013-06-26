/**
 *
 *  \file adf.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
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
  /// \brief The core of vita::adt and vita::adf
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
  ///
  /// adf_core is the core of vita::adt and vita::adf classes (they are in a
  /// HAS-A relationship with it).
  ///
  /// \note
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

    std::string display(const std::string &) const;

    bool debug() const;

  private: // Data members
    opcode_t     id;
    individual code;

    static opcode_t adf_count()
    {
      static opcode_t counter(0);
      return counter++;
    }
  };

  ///
  /// \brief Subroutine with arguments
  ///
  class adf : public function
  {
  public:
    adf(const individual &, const std::vector<category_t> &, unsigned);

    virtual any eval(interpreter *) const override;

    const individual &get_code() const;
    virtual std::string display() const override;

    virtual bool auto_defined() const override;

    virtual bool debug() const override;

  private:  // Private data members.
    adf_core core_;
  };

  ///
  /// \brief Subroutines WITHOUT arguments
  ///
  /// \see
  /// "An Analysis of Automatic Subroutine Discovery in Genetic Programming" -
  /// A.Dessi', A.Giani, A.Starita.
  ///
  class adt : public terminal
  {
  public:
    adt(const individual &, unsigned);

    virtual any eval(interpreter *) const override;

    const individual &get_code() const;
    virtual std::string display() const override;

    virtual bool auto_defined() const override;

    virtual bool debug() const override;

  private:  // Private data members.
    adf_core core_;
  };
}  // namespace vita

#endif  // ADF_H
