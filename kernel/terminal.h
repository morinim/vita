/**
 *
 *  \file terminal.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
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

#if !defined(TERMINAL_H)
#define      TERMINAL_H

#include <string>

#include "kernel/symbol.h"

namespace vita
{
  ///
  /// A terminal (leaf) node in the parse trees representing the programs in the
  /// population. A terminal might be a variable (input to the program), a
  /// constant value or a function taking no arguments (e.g. move-north).
  /// Terminals are symbols with an arity of zero.
  ///
  class terminal : public symbol
  {
  public:
    terminal(const std::string &, category_t, bool = false, bool = false,
             unsigned = default_weight);

    ///
    /// \return \c false (terminals haven't arguments and cannot be
    ///         associative).
    ///
    bool associative() const { return false; }

    ///
    /// \return \c true if the terminal is an input variable.
    ///
    /// An input variable is a feature from the learning domain.
    ///
    bool input() const { return input_; }

    ///
    /// \return \c true if the terminal is parametric.
    ///
    bool parametric() const { return parametric_; }

    ///
    /// \return 0 (0 arguments <=> terminal).
    ///
    unsigned arity() const { return 0; }

    bool check() const;

  private: // Serialization.
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &, unsigned);

  public:   // Public data members.
    static unsigned default_weight;

  private:  // Private data members.
    const bool parametric_;
    const bool      input_;
  };

  ///
  /// \see \c boost::serialization
  ///
  template<class Archive>
  void terminal::serialize(Archive &ar, unsigned)
  {
    ar & boost::serialization::base_object<symbol>(*this);
    ar & default_weight;
    ar & parametric_;
    ar & input_;
  }
}  // namespace vita

#endif  // TERMINAL_H
