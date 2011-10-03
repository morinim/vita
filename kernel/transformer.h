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

  ///
  /// This is a base functor \c class. A \a transformer takes one or more
  /// individuals and creates a new \a individual performing a transformation
  /// (crossover, recombination, mutation...).
  ///
  class transformer
  {
  public:
    virtual individual operator()(const individual &,
                                  const individual &) const = 0;

  protected:
    vita::gene &gene(individual *, unsigned) const;
  };

  ///
  /// Uniform crossover, as the name suggests, is a GP operator inspired by the
  /// GA operator of the same name (G. Syswerda. Uniform crossover in genetic
  /// algorithms - Proceedings of the Third International Conference on Genetic
  /// Algorithms. 1989). GA uniform crossover constructs offspring on a
  /// bitwise basis, copying each allele from each parent with a 50%
  /// probability. Thus the information at each gene location is equally likely
  /// to have come from either parent and on average each parent donates 50%
  /// of its genetic material. The whole operation, of course, relies on the
  /// fact that all the chromosomes in the population are of the same structure
  /// and the same length. GP uniform crossover begins with the observation that
  /// many parse trees are at least partially structurally similar.
  ///
  class uniform_crossover : public transformer
  {
  public:
    individual operator()(const individual &, const individual &) const;
  };

  ///
  /// The oldest homologous crossover in tree-based GP is one-point crossover.
  /// This works by selecting a common crossover point in the parent programs
  /// and then swapping the corresponding subtrees.
  ///
  class one_point_crossover : public transformer
  {
  public:
    individual operator()(const individual &, const individual &) const;
  };

  ///
  /// Two instructions are randomly chosen in each parent as cut points. The
  /// code fragment excised from the first parent is then replaced with the
  /// code fragment excised from the second to generate the child.
  ///
  class two_point_crossover : public transformer
  {
  public:
    individual operator()(const individual &, const individual &) const;
  };
}  // namespace vita

#endif  // TRANSFORMER_H
