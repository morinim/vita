/**
 *
 *  \file adf.h
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

#if !defined(ADF_H)
#define      ADF_H

#include <string>
#include <vector>

#include "kernel/vita.h"
#include "kernel/function.h"
#include "kernel/individual.h"
#include "kernel/terminal.h"

namespace vita
{
  class interpreter;

  class adf : public function
  {
  public:
    adf(const individual &, const std::vector<symbol_t> &, unsigned);

    boost::any eval(interpreter *) const;

    std::string display() const;

    const individual &get_code() const;

    bool check() const;

  private:
    const unsigned id_;
    individual   code_;
  };

  class adf0 : public terminal
  {
  public:
    adf0(const individual &, unsigned);

    boost::any eval(interpreter *) const;

    std::string display() const;

    const individual &get_code() const;

    bool check() const;

  private:
    const unsigned id_;
    individual   code_;
  };

}  // namespace vita

#endif  // ADF_H
