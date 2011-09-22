/**
 *
 *  \file transformer.cc
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

#include "kernel/transformer.h"
#include "kernel/individual.h"
#include "kernel/random.h"

namespace vita
{
  inline
  vita::gene &transformer::gene(individual *i, unsigned l) const
  {
    assert(i->check());
    assert(l < i->size());

    return i->code_[l];
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the transformed \a individual.
  ///
  /// The i-th locus of the offspring has a 50% probability to be filled with
  /// the i-th gene of \a p1 and 50% with i-th gene of \a p2. Parents must have
  /// the same size.
  ///
  individual uniform_crossover::operator()(const individual &p1,
                                           const individual &p2) const
  {
    assert(p1.check());
    assert(p2.check());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());

    individual offspring(p1);
    for (unsigned i(0); i < cs; ++i)
      if (!random::boolean())
        gene(&offspring, i) = p2[i];

    assert(offspring.check());
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return a the transformed \a individual.
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a single locus
  /// (crossover point). The offspring is created with genes from the choosen
  /// parent up to the crossover point and genes from the other parent beyond
  /// that point. Parents must have the same size.
  ///
  individual one_point_crossover::operator()(const individual &p1,
                                             const individual &p2) const
  {
    assert(p1.check());
    assert(p2.check());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());

    const unsigned cut(random::between<unsigned>(0, cs-1));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (unsigned i(cut); i < cs; ++i)
      gene(&offspring, i) = (*parents[!base])[i];

    assert(offspring.check());
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return a the transformed \a individual.
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a two loci
  /// (crossover points). The offspring is created with genes from the choosen
  /// parent before the first crossover point and after the second crossover
  /// point; genes between crossover points are taken from the other parent.
  /// Parents must have the same size.
  ///
  individual two_point_crossover::operator()(const individual &p1,
                                             const individual &p2) const
  {
    assert(p1.check());
    assert(p2.check());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());

    const unsigned cut1(random::between<unsigned>(0, cs-1));
    const unsigned cut2(random::between<unsigned>(cut1+1, cs));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (unsigned i(cut1); i < cut2; ++i)
      gene(&offspring, i) = (*parents[!base])[i];

    assert(offspring.check());
    return offspring;
  }
}  // Namespace vita
