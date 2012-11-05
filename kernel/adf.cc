/**
 *
 *  \file adf.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <boost/lexical_cast.hpp>

#include "adf.h"
#include "interpreter.h"

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
  /// \param[in] sv categories of the function arguments.
  /// \param[in] w the weight of the ADF.
  ///
  adf::adf(const individual &ind, const std::vector<category_t> &sv, unsigned w)
    : function("ADF", ind.category(), sv, w), core_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \param[in] i the context in which this ADF is evaluated.
  /// \return the output of the ADF.
  ///
  /// Adf functions need input parameters from the a context (contrary to
  /// adt::eval).
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
    return "ADF_" + boost::lexical_cast<std::string>(core_.id);
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
    : terminal("ADT", ind.category(), false, false, w), core_(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  ///
  /// \return the output of the ADT.
  ///
  /// Adt hasn't input parameters so the context is ignored (contrary to
  /// adf::eval).
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
