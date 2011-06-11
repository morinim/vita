/**
 *
 *  \file fitness.h
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

#if !defined(FITNESS_H)
#define      FITNESS_H

#include "vita.h"

namespace vita
{
  ///
  /// A value assigned to an individual which reflects how well the individual
  /// solves the task.
  ///
  typedef double fitness_t;

  inline double distance(fitness_t f1, fitness_t f2) {return std::fabs(f1-f2);}
}  // namespace vita

#endif  // FITNESS_H
