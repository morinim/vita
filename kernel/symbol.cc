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

#include <boost/lexical_cast.hpp>

#include "kernel/symbol.h"

namespace vita
{
  opcode_t symbol::opc_count_(0);

  ///
  /// \return the name of the symbol.
  ///
  std::string symbol::display() const
  {
    return display_;
  }

  ///
  /// \param[in] v
  /// \return a string representing the symbol.
  ///
  /// This function is called for parametric symbols only. The \a v argument is
  /// used to build a more meaningful name for the symbol (i.e. for a numeric
  /// terminal it's better to print 123 than "NUMBER").
  ///
  std::string symbol::display(int v) const
  {
    assert(parametric());

    return display() + "_" + boost::lexical_cast<std::string>(v);
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol::debug() const
  {
    return display().size() > 0;
  }
}  // namespace vita
