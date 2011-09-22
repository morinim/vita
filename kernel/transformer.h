/**
 *
 *  \file transformer.h
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

#if !defined(TRANSFORMER_H)
#define      TRANSFORMER_H

#include "kernel/gene.h"

namespace vita
{
  class individual;

  class transformer
  {
  public:
    virtual individual operator()(const individual &,
                                  const individual &) const = 0;

  protected:
    vita::gene &gene(individual *, unsigned) const;
  };

  class uniform_crossover : public transformer
  {
  public:
    virtual individual operator()(const individual &, const individual &) const;
  };

  class one_point_crossover : public transformer
  {
  public:
    virtual individual operator()(const individual &, const individual &) const;
  };

  class two_point_crossover : public transformer
  {
  public:
    virtual individual operator()(const individual &, const individual &) const;
  };
}  // namespace vita

#endif  // TRANSFORMER_H
