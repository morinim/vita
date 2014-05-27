/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_ADF_H)
#define      VITA_ADF_H

#include <string>
#include <vector>

#include "kernel/function.h"
#include "kernel/individual/mep.h"
#include "kernel/terminal.h"

namespace vita
{
  template<class T> class interpreter;

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
  /// has-a relationship with it).
  ///
  /// \note
  /// Although the acronym ADF is from Koza's automatically defined functions,
  /// in Vita subroutines are created using the ARL scheme described in
  /// "Discovery of subroutines in genetic programming" - J.P. Rosca and D.H.
  /// Ballard.
  ///
  template<class T>
  class adf_core
  {
  public:
    explicit adf_core(const T &);

    const T &code() const;

    std::string display(const std::string &) const;

    bool debug() const;

  private:  // Private data members
    T        code_;
    opcode_t   id_;

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
    adf(const i_mep &, std::vector<category_t>, unsigned);

    virtual any eval(interpreter<i_mep> *) const override;

    virtual std::string display() const override;

    virtual bool auto_defined() const override;

    virtual bool debug() const override;

    const i_mep &code() const;

  private:
    adf_core<i_mep> core_;
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
    adt(const i_mep &, unsigned);

    virtual any eval(interpreter<i_mep> *) const override;

    virtual std::string display() const override;

    virtual bool auto_defined() const override;

    virtual bool debug() const override;

    const i_mep &code() const;

  private:
    adf_core<i_mep> core_;
  };

#include "kernel/adf_inl.h"
}  // namespace vita

#endif  // Include guard
