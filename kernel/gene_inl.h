/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GENE_INL_H)
#define      VITA_GENE_INL_H

template<unsigned K>
constexpr decltype(K) basic_gene<K>::k_args;

///
/// \param[in] t a terminal.
///
/// A new gene built from terminal \a t.
///
/// \note
/// This is usually called for filling the patch section of an individual.
///
template<unsigned K>
basic_gene<K>::basic_gene(symbol *t) : sym(t)
{
  assert(sym->terminal());

  if (sym->parametric())
    par = sym->init();
}

///
/// \param[in] g a reference to a symbol and its arguments.
///
/// With this constructor it is easy to write genome "by hand":
///     std::vector<gene> g(
///     {
///       {{f_add, {1, 2}}},  // [0] ADD 1,2
///       {{    y,   null}},  // [1] Y
///       {{    x,   null}}   // [2] X
///     };
///
template<unsigned K>
basic_gene<K>::basic_gene(
  const std::pair<symbol *, std::vector<index_t>> &g) : sym(g.first)
{
  if (sym->parametric())
    par = sym->init();
  else
  {
    const auto arity(sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
    {
#if !defined(NDEBUG)
      typedef typename std::remove_reference<decltype(args[0])>::type
        ARRAY_ELEM_TYPE;
      assert(g.second[i] <= std::numeric_limits<ARRAY_ELEM_TYPE>::max());
#endif
      args[i] = g.second[i];
    }
  }
}

///
/// \param[in] s a symbol.
/// \param[in] from a starting locus in the genome.
/// \param[in] sup an upper limit in the genome.
///
/// A new gene built from symbol \a s with argument in the [from;sup[ range.
///
/// \note
/// This is usually called for filling the standard section of an individual.
///
template<unsigned K>
basic_gene<K>::basic_gene(symbol *s, index_t from, index_t sup) : sym(s)
{
  assert(from < sup);

  if (sym->parametric())
    par = sym->init();
  else
  {
    const auto arity(sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
    {
      assert(sup <= type_max(args[0]));
      args[i] = random::between(from, sup);
    }
  }
}

///
/// \param[in] g second term of comparison.
/// \return \c true if \c this == \a g
///
template<unsigned K>
bool basic_gene<K>::operator==(const basic_gene<K> &g) const
{
  if (sym != g.sym)
    return false;

  if (sym->parametric())
    return par == g.par;

  const auto arity(sym->arity());
  for (auto i(decltype(arity){0}); i < arity; ++i)
    if (args[i] != g.args[i])
      return false;

  return true;
}
#endif  // Include guard
