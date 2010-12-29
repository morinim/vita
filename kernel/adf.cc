/**
 *
 *  \file adf.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "adf.h"
#include "individual.h"
#include "interpreter.h"

namespace vita
{

  unsigned adf::_adf_count(0);

  ///
  /// \param ind[in]
  /// \param sv[in]
  /// \param w[in]
  ///
  adf::adf(const individual &ind, const std::vector<symbol_t> &sv, unsigned w)
    : function("ADF",ind.type(),sv,w), _id(_adf_count++), _code(ind)
  {
    assert(ind.check() && ind.eff_size() >= 2);

    assert(check());
  }

  /**
   * get_code
   * \return
   */
  const individual &
  adf::get_code() const
  {
    return _code;
  }

  /**
   * eval
   * \param i[in]
   * \return
   */
  boost::any
  adf::eval(interpreter &i) const
  {  
    return interpreter(_code,&i).run();
  }

  /**
   * display
   * \return
   */
  std::string
  adf::display() const
  {
    std::ostringstream s;
    s << "ADF" << '_' << _id;

    return s.str();
  }

  /**
   * check
   * \return true if the individual passes the internal consistency check.
   */
  bool
  adf::check() const
  {
    // No recursive calls.
    for (individual::const_iterator i(_code); i(); ++i)
      if (i->sym == this)
        return false;
    
    return _code.eff_size() > 2 && function::check();
  }

}  // Namespace vita
