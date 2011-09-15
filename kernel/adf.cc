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
  unsigned adf_core::adf_count(0);

  ///
  /// \param[in] ind individual whose code is used as ADF.
  ///
  adf_core::adf_core(const individual &ind) : id(adf_count++), code(ind)
  {
  }

  ///
  /// \return \c true if the \a object passes the internal consistency check.
  ///
  bool adf_core::check() const
  {
    return code.eff_size() >= 2;
  }




  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] sv types of the function arguments.
  /// \param[in] w the weight of the ADF.
  ///
  adf_n::adf_n(const individual &ind, const std::vector<symbol_t> &sv,
               unsigned w)
    : function("ADFn", ind.type(), sv, w), adf_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \param[in] i the context in which this ADF is evaluated.
  /// \return the output of the ADF.
  ///
  /// adf_n functions need input parameters from the \a i context.
  ///
  boost::any adf_n::eval(interpreter *i) const
  {
    return interpreter(adf_.code, i)();
  }

  ///
  /// \return the code (\a individual) of the ADF.
  ///
  const individual &adf_n::get_code() const
  {
    return adf_.code;
  }

  ///
  /// \return the name of the ADF.
  ///
  std::string adf_n::display() const
  {
    std::ostringstream s;
    s << "ADFn" << '_' << adf_.id;

    return s.str();
  }

  ///
  /// \return \c true if the \a object passes the internal consistency check.
  ///
  bool adf_n::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(adf_.code); i(); ++i)
      if (i->sym.get() == this)
        return false;

    return adf_.check() && function::check();
  }



  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] w the weight of the ADF.
  ///
  adf_0::adf_0(const individual &ind, unsigned w)
    : terminal("ADF0", ind.type(), false, false, w), adf_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \return the output of the ADF.
  ///
  /// adf_0 functions haven't input parameters so the context is not used.
  ///
  boost::any adf_0::eval(interpreter *) const
  {
    return interpreter(adf_.code)();
  }

  ///
  /// \return the code (\a individual) of the ADF.
  ///
  const individual &adf_0::get_code() const
  {
    return adf_.code;
  }

  ///
  /// \return the name of the ADF.
  ///
  std::string adf_0::display() const
  {
    std::ostringstream s;
    s << "ADF0" << '_' << adf_.id;

    return s.str();
  }

  ///
  /// \return \c true if the \a object passes the internal consistency check.
  ///
  bool adf_0::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(adf_.code); i(); ++i)
      if (i->sym.get() == this)
        return false;

    return adf_.check() && terminal::check();
  }
}  // Namespace vita
