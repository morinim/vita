/**
 *
 *  \file factory.h
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
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

#if !defined(PRIMITIVE_FACTORY_H)
#define      PRIMITIVE_FACTORY_H

#include <boost/any.hpp>

#include "kernel/vita.h"
#include "kernel/primitive/double_pri.h"

namespace vita
{
  class variable : public terminal
  {
  public:
    explicit variable(const std::string &name, category_t t = 0)
      : terminal(name, t, true) {}

    boost::any eval(vita::interpreter *) const { return val; }

    boost::any val;
  };
}  // namespace vita

#endif  // PRIMITIVE_FACTORY_H
