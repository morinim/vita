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

#if !defined(VITA_MEP_INTERPRETER_H)
#define      VITA_MEP_INTERPRETER_H

#include <boost/optional.hpp>

#include "kernel/core_interpreter.h"
#include "kernel/matrix.h"
#include "kernel/vitafwd.h"

namespace vita
{
  ///
  /// \brief A template specialization for interpreter<T> class
  ///
  /// \note
  /// We don't have a generic interpreter<T> implementation (e.g. see the
  /// lambda_f source code) because interpreter and individual are strongly
  /// coupled: the interpreter must be build around the peculiarities of
  /// the specific individual class.
  ///
  template<>
  class interpreter<i_mep> : public core_interpreter
  {
  public:
    explicit interpreter(const i_mep &, interpreter<i_mep> * = nullptr);

    any fetch_param();
    any fetch_arg(unsigned);
    any fetch_adf_arg(unsigned);

  private:  // Methods of the non-virtual interface
    virtual any run_nvi() override;
    virtual bool debug_nvi() const override;

  private:  // Private support methods
    any run_locus(const locus &);

  private:  // Private data members
    const i_mep &prg_;

    mutable matrix<boost::optional<any>> cache_;

    // Instruction pointer.
    locus ip_;

    interpreter<i_mep> *const context_;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
}  // namespace vita

#endif  // Include guard
