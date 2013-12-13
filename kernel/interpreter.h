/**
 *
 *  \file kernel/interpreter.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(INTERPRETER_H)
#define      INTERPRETER_H

#include <boost/optional.hpp>

#include "kernel/matrix.h"

namespace vita
{
  class individual;
  template<class T> class team;

  ///
  /// \tparam T the type of individual used.
  ///
  /// This class specifies the minimum interface of an interpreter.
  ///
  template<class T>
  class basic_interpreter
  {
  public:
    explicit basic_interpreter(const T &, basic_interpreter<T> * = nullptr);

    virtual any run() = 0;

    virtual bool debug() const { return true; }

  protected:
    const T &prg_;

    basic_interpreter<T> *const context_;
  };

  ///
  /// \tparam T the type of individual used.
  ///
  /// The interpreter class "executes" an individual (a program) in its
  /// environment.
  ///
  template<class T> class interpreter;

  template<>
  class interpreter<individual> : public basic_interpreter<individual>
  {
  public:
    explicit interpreter(const individual &,
                         interpreter<individual> * = nullptr);

    virtual any run() override;

    any fetch_param();
    any fetch_arg(unsigned);
    any fetch_adf_arg(unsigned);

    virtual bool debug() const override;

  private:  // Private methods.
    any run_locus(const locus &);

  private:
    // Instruction pointer.
    locus ip_;

    mutable matrix<boost::optional<any>> cache_;
  };

  ///
  /// \tparam T the type of individual used.
  ///
  /// This class executes a team of individuals of type \a T.
  ///
  template<class T>
  class interpreter<team<T>> : public basic_interpreter<team<T>>
  {
  public:
    explicit interpreter(const team<T> &, interpreter<team<T>> * = nullptr);

    virtual any run() override;

    virtual bool debug() const override;
  };

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///

#include "kernel/interpreter_inl.h"
}  // namespace vita

#endif  // INTERPRETER_H
