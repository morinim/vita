/**
 *
 *  \file classifier.h
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

#if !defined(CLASSIFIER_H)
#define      CLASSIFIER_H

#include <string>

#include "kernel/data.h"
#include "kernel/individual.h"

namespace vita
{
  class classifier
  {
  public:
    explicit classifier(const individual &ind) : ind_(ind)
    { assert(ind.check()); }

    virtual std::string operator()(const data::value_type &) const = 0;

  protected:
    individual ind_;
  };
}  // namespace vita

#endif  // CLASSIFIER_H