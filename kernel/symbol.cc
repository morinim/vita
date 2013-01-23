/**
 *
 *  \file symbol.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "symbol.h"

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
  /// This function is called by parametric symbols only. The \a v argument is
  /// used to build a more meaningful name for the symbol (i.e. for a numeric
  /// terminal it's better to print 123 than "NUMBER").
  ///
  std::string symbol::display(int v) const
  {
    assert(parametric());

    return display() + "_" + std::to_string(v);
  }

  ///
  /// \return \c true if symbol was loaded correctly.
  ///
  bool symbol::load(std::istream &in)
  {
    std::getline(in, display_);
    in >> opcode_;
    in >> category_;
    in >> weight;

    const bool ok(in.good());
    if (ok && opc_count_ <= opcode_)
      opc_count_ = opcode_ + 1;

    return ok;
  }

  ///
  /// \return \c true if symbol was saved correctly.
  ///
  bool symbol::save(std::ostream &out) const
  {
    out << display_ << '\n'
        << opcode_ << ' ' << category_ << ' ' << weight << std::endl;

    return out.good();
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool symbol::check() const
  {
    return display_.size() > 0;
  }
}  // Namespace vita
