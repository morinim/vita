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

#if !defined(VITA_ADF_H)
#define      VITA_ADF_H

#include <atomic>
#include <string>

#include "kernel/vitafwd.h"
#include "kernel/gp/function.h"
#include "kernel/gp/mep/i_mep.h"
#include "kernel/gp/terminal.h"

namespace vita
{
///
/// The core of vita::adt and vita::adf.
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
/// *has-a* relationship).
///
/// \note
/// Although the acronym ADF is from Koza's automatically defined functions,
/// in Vita subroutines are created using the ARL scheme described in
/// [Discovery of Subroutines in Genetic Programming](https://github.com/morinim/vita/wiki/bibliography#15).
///
template<class T>
class adf_core
{
public:
  explicit adf_core(const T &);

  const T &code() const;

  std::string name(const std::string &) const;

  bool is_valid() const;

private:
  T      code_;
  opcode_t id_;

  static opcode_t adf_count()
  {
    // Generates identifiers for ADF functions in a threadsafe manner.
    static std::atomic<opcode_t> counter(0);
    return counter++;
  }
};

///
/// Subroutine with arguments.
///
class adf : public function
{
public:
  adf(const i_mep &, cvect);

  bool auto_defined() const final;

  value_t eval(core_interpreter *) const override;

  std::string name() const override;

  bool is_valid() const override;

  const i_mep &code() const;

private:
  adf_core<i_mep> core_;
};

///
/// Subroutines WITHOUT arguments.
///
/// \see
/// "An Analysis of Automatic Subroutine Discovery in Genetic Programming" -
/// A.Dessi', A.Giani, A.Starita.
///
class adt : public terminal
{
public:
  explicit adt(const i_mep &);

  bool auto_defined() const final;

  value_t eval(core_interpreter *) const override;

  std::string name() const override;

  bool is_valid() const override;

  const i_mep &code() const;

private:
  adf_core<i_mep> core_;
};

}  // namespace vita

#endif  // include guard
