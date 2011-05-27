/**
 *
 *  \file symbol.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "symbol.h"

namespace vita
{

  unsigned symbol::_opc_count(0);

  ///
  /// \return the name of the symbol.
  ///
  std::string
  symbol::display() const
  {
    return _display;
  }

  ///
  /// \param[in] v
  /// \return
  ///
  std::string
  symbol::display(int v) const
  {
    assert(parametric());

    std::ostringstream s;
    s << _display << '_' << v;
    return s.str();
  }

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  bool
  symbol::check() const
  {
    return _display.size() > 0;
  }
  
}  // Namespace vita
