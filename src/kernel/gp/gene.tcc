/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GENE_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_GENE_TCC)
#define      VITA_GENE_TCC

///
/// A new gene built from terminal `t`.
///
/// \param[in] t a terminal
///
/// \note
/// This is usually called for filling the patch section of an individual.
///
template<unsigned K>
basic_gene<K>::basic_gene(const terminal &t) : sym(&t), par(), args(0)
{
  init_if_parametric();
}

///
/// Possibly inits the parameter.
///
template<unsigned K>
void basic_gene<K>::init_if_parametric()
{
  Expects(sym->terminal());

  const auto t(terminal::cast(sym));
  if (t->parametric())
    par = t->init();
}

///
/// Utility constructor to input hard-coded genomes.
///
/// \param[in] g a reference to a symbol and its arguments
///
/// A constructor that makes it easy to write genome "by hand":
///     std::vector<gene> g(
///     {
///       {{f_add, {1, 2}}},  // [0] ADD 1,2
///       {{    y,     {}}},  // [1] Y
///       {{    x,     {}}}   // [2] X
///     };
///
template<unsigned K>
basic_gene<K>::basic_gene(const std::pair<symbol *, std::vector<index_t>> &g)
  : sym(g.first), args(g.first->arity())
{
  if (sym->arity())
  {
    std::transform(g.second.begin(), g.second.end(), args.begin(),
                   [](index_t i)
                   {
                     Expects(i <= std::numeric_limits<packed_index_t>::max());
                     return static_cast<packed_index_t>(i);
                   });
  }
  else
    init_if_parametric();
}

///
/// A new gene built from symbol `s` with argument in the `[from;sup[` range.
///
/// \param[in] s    a symbol
/// \param[in] from a starting index in the genome
/// \param[in] sup  an upper limit in the genome
///
/// \note
/// This is usually called for filling the standard section of an individual.
///
template<unsigned K>
basic_gene<K>::basic_gene(const symbol &s, index_t from, index_t sup)
  : sym(&s), args(s.arity())
{
  Expects(from < sup);

  if (s.arity())
  {
    assert(sup <= std::numeric_limits<packed_index_t>::max());

    std::generate(args.begin(), args.end(),
                  [from, sup]()
                  {
                    return static_cast<packed_index_t>(random::between(from,
                                                                       sup));
                  });
  }
  else
    init_if_parametric();
}

///
/// \return the list of loci associated with the arguments of the current
///         gene.
///
template<unsigned K>
small_vector<locus, K> basic_gene<K>::arguments() const
{
  small_vector<locus, K> ret(sym->arity());

  std::generate(ret.begin(), ret.end(),
                [this, i = 0u]() mutable
                {
                  return locus_of_argument(i++);
                });

  return ret;
}

///
/// \param[in] i ordinal of an argument
/// \return      the locus that `i`-th argument of the current symbol refers to
///
template<unsigned K>
locus basic_gene<K>::locus_of_argument(std::size_t i) const
{
  Expects(i < sym->arity());

  return {args[i], function::cast(sym)->arg_category(i)};
}

///
/// \param[in] g1 first term of comparison
/// \param[in] g2 second term of comparison
/// \return       `true` if `g1 == g2`
///
template<unsigned K>
bool operator==(const basic_gene<K> &g1, const basic_gene<K> &g2)
{
  if (g1.sym != g2.sym)
    return false;

  assert(g1.sym->arity() == g2.sym->arity());

  if (g1.sym->arity())
    return std::equal(g1.args.begin(), g1.args.end(),
                      g2.args.begin(), g2.args.end());

  assert(g1.sym->terminal());
  const auto t(terminal::cast(g1.sym));

  return !t->parametric() || almost_equal(g1.par, g2.par);
}

///
/// \param[in] g1 first term of comparison
/// \param[in] g2 second term of comparison
/// \return       `true` if `g1 != g2`
///
template<unsigned K>
bool operator!=(const basic_gene<K> &g1, const basic_gene<K> &g2)
{
  return !(g1 == g2);
}

///
/// \param[out] s output stream
/// \param[in]  g gene to print
/// \return       output stream including `g`
///
template<unsigned K>
std::ostream &operator<<(std::ostream &s, const basic_gene<K> &g)
{
  const auto *sym(g.sym);

  if (sym->terminal())
  {
    const auto t(terminal::cast(g.sym));
    if (t->parametric())
      return s << t->display(g.par);
  }

  return s << sym->name();
}

#endif  // include guard
