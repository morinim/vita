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
  adf::adf(const individual &ind, const std::vector<symbol_t> &sv, unsigned w)
    : function("ADF", ind.type(), sv, w), core_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \param[in] i the context in which this ADF is evaluated.
  /// \return the output of the ADF.
  ///
  /// adf functions need input parameters from the \a i context.
  ///
  boost::any adf::eval(interpreter *i) const
  {
    return interpreter(core_.code, i)();
  }

  ///
  /// \return the code (\a individual) of the ADF.
  ///
  const individual &adf::get_code() const
  {
    return core_.code;
  }

  ///
  /// \return the name of the ADF.
  ///
  std::string adf::display() const
  {
    std::ostringstream s;
    s << "ADF" << '_' << core_.id;

    return s.str();
  }

  ///
  /// \return \c true.
  ///
  bool adf::auto_defined() const
  {
    return true;
  }

  ///
  /// \return \c true if the \a object passes the internal consistency check.
  ///
  bool adf::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(core_.code); i(); ++i)
      if (i->sym.get() == this)
        return false;

    return core_.check() && function::check();
  }

  ///
  /// \param[in] ind the code for the ADT.
  /// \param[in] w the weight of the ADT.
  ///
  adt::adt(const individual &ind, unsigned w)
    : terminal("ADT", ind.type(), false, false, w), core_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \return the output of the ADT.
  ///
  /// adt hasn't input parameters so the context is not used.
  ///
  boost::any adt::eval(interpreter *) const
  {
    return interpreter(core_.code)();
  }

  ///
  /// \return the code (\a individual) of the ADT.
  ///
  const individual &adt::get_code() const
  {
    return core_.code;
  }

  ///
  /// \return the name of the ADT.
  ///
  std::string adt::display() const
  {
    std::ostringstream s;
    s << "ADT" << '_' << core_.id;

    return s.str();
  }

  ///
  /// \return \c true.
  ///
  bool adt::auto_defined() const
  {
    return true;
  }

  ///
  /// \return \c true if the \a object passes the internal consistency check.
  ///
  bool adt::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(core_.code); i(); ++i)
      if (i->sym.get() == this)
        return false;

    return core_.check() && terminal::check();
  }
}  // Namespace vita
