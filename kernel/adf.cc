/**
 *
 *  \file adf.cc
 *
 *  \author Manlio Morini
 *  \date 2011/05/27
 *
 *  This file is part of VITA
 *
 */

#include "adf.h"
#include "individual.h"
#include "interpreter.h"

namespace vita
{

  static unsigned _adf_count(0);

  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] sv types of the function arguments.
  /// \param[in] w the weight of the ADF.
  ///
  adf::adf(const individual &ind, const std::vector<symbol_t> &sv, unsigned w)
    : function("ADF",ind.type(),sv,w), _id(_adf_count++), _code(ind)
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
    return _code;
  }

  ///
  /// \param[in] i
  /// \return
  ///
  boost::any
  adf::eval(interpreter &i) const
  {  
    return interpreter(_code,&i).run();
  }

  ///
  /// \return
  ///
  std::string
  adf::display() const
  {
    std::ostringstream s;
    s << "ADF" << '_' << _id;

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
    for (individual::const_iterator i(_code); i(); ++i)
      if (i->sym == this)
        return false;
    
    return _code.eff_size() > 2 && function::check();
  }



  ///
  /// \param[in] ind the code for the ADF.
  /// \param[in] w the weight of the ADF.
  ///
  adf0::adf0(const individual &ind, unsigned w)
    : terminal("ADF0",ind.type(),false,false,w), _id(_adf_count++), _code(ind)
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
    return _code;
  }

  ///
  /// \return
  ///
  boost::any
  adf0::eval(interpreter &) const
  {  
    return interpreter(_code).run();
  }

  ///
  /// \return
  ///
  std::string
  adf0::display() const
  {
    std::ostringstream s;
    s << "ADF0" << '_' << _id;

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
    for (individual::const_iterator i(_code); i(); ++i)
      if (i->sym == this)
        return false;
    
    return _code.eff_size() >= 2 && terminal::check();
  }

}  // Namespace vita
