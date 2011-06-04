/**
 *
 *  \file adf.cc
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

#include "kernel/adf.h"
#include "kernel/individual.h"
#include "kernel/interpreter.h"

namespace vita
{
  static unsigned adf_count_(0);

  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] sv types of the function arguments.
  /// \param[in] w the weight of the ADF.
  ///
  adf::adf(const individual &ind, const std::vector<symbol_t> &sv, unsigned w)
    : function("ADF", ind.type(), sv, w), id_(adf_count_++), code_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \return the code (\a individual) of the ADF.
  ///
  const individual &
  adf::get_code() const
  {
    return code_;
  }

  ///
  /// \param[in] i
  /// \return
  ///
  boost::any
  adf::eval(interpreter *i) const
  {
    return interpreter(code_, i).run();
  }

  ///
  /// \return
  ///
  std::string
  adf::display() const
  {
    std::ostringstream s;
    s << "ADF" << '_' << id_;

    return s.str();
  }

  ///
  /// \return \c true if the \a individual passes the internal consistency
  ///         check.
  ///
  bool
  adf::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(code_); i(); ++i)
      if (i->sym == this)
        return false;

    return code_.eff_size() > 2 && function::check();
  }



  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] w the weight of the ADF.
  ///
  adf0::adf0(const individual &ind, unsigned w)
    : terminal("ADF0", ind.type(), false, false, w), id_(adf_count_++),
      code_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \return the code (\a individual) of the ADF.
  ///
  const individual &
  adf0::get_code() const
  {
    return code_;
  }

  ///
  /// \return
  ///
  boost::any
  adf0::eval(interpreter *) const
  {
    return interpreter(code_).run();
  }

  ///
  /// \return
  ///
  std::string
  adf0::display() const
  {
    std::ostringstream s;
    s << "ADF0" << '_' << id_;

    return s.str();
  }

  ///
  /// \return \c true if the \a individual passes the internal consistency
  ///         check.
  ///
  bool
  adf0::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(code_); i(); ++i)
      if (i->sym == this)
        return false;

    return code_.eff_size() >= 2 && terminal::check();
  }
}  // Namespace vita
