/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_ga.h"
#include "kernel/cache_hash.h"
#include "kernel/log.h"

namespace vita
{
///
/// \param[in] e base environment.
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they do not violate the type system's
/// constraints.
///
i_ga::i_ga(const environment &e) : individual(), genome_(e.sset->categories())
{
  Expects(e.debug(true));
  Expects(e.sset);

  const auto cs(parameters());
  assert(cs);

  for (auto c(decltype(cs){0}); c < cs; ++c)
    genome_[c] = gene(e.sset->roulette_terminal(c));

  Ensures(debug());
}

///
/// \param[out] s output stream.
///
/// The output stream contains a graph, described in dot language
/// (http://www.graphviz.org/), of this individual.
///
void i_ga::graphviz(std::ostream &s) const
{
  s << "graph {";

  for (const auto &g : genome_)
    s << "g [label=" << g << ", shape=circle];";

  s << '}';
}

///
/// Prints the genes of the individual.
///
/// \param[in]  ga data to be printed
/// \param[out] s  output stream
/// \return        a reference to the output stream
///
/// \relates i_ga
///
std::ostream &in_line(const i_ga &ga, std::ostream &s)
{
  std::copy(ga.begin(), ga.end(), infix_iterator<gene>(s, " "));
  return s;
}

///
/// \brief A new individual is created mutating `this`
///
/// \param[in] p probability of gene mutation.
/// \param[in] env the current environment.
/// \return number of mutations performed.
///
/// \note
/// This function is included for compatibility with GP recombination
/// strategies. Typical differential evolution GA algorithm won't use
/// this method.
///
unsigned i_ga::mutation(double p, const environment &env)
{
  Expects(0.0 <= p && p <= 1.0);

  unsigned n(0);

  const auto ps(parameters());
  for (category_t c(0); c < ps; ++c)
    if (random::boolean(p))
    {
      const gene g(env.sset->roulette_terminal(c));

      if (g != genome_[c])
      {
        ++n;
        genome_[c] = g;
      }
    }

  // Here we assume that a micromutation of a terminal isn't significative.
  // It can happen (the probability is very very low but...) that
  //
  // {1.0, 0, 0} --MUTATION--> {0.999999999999999999999999999999999999, 0, 0}
  //
  // The individuals are considered equal (the comparison between parametric
  // terminals are based on the `almost_equal` function) so this doesn't count
  // as mutation.
  if (n)
    signature_ = hash();

  Ensures(debug());
  return n;
}

///
/// \brief Two points crossover.
/// \param[in] lhs first parent.
/// \param[in] rhs second parent.
/// \return the result of the crossover (we only generate a single offspring).
///
/// We randomly select two loci (common crossover points). The offspring is
/// created with genes from the `rhs` parent before the first crossover
/// point and after the second crossover point; genes between crossover
/// points are taken from the `lhs` parent.
///
/// \note
/// - Parents must have the same size.
/// - The function is included for compatibility with GP recombination
///   strategies. Typical differential evolution GA algorithm won't use
///   this method.
///
i_ga crossover(const i_ga &lhs, const i_ga &rhs)
{
  Expects(lhs.debug());
  Expects(rhs.debug());
  Expects(lhs.parameters() == rhs.parameters());

  const auto ps(lhs.parameters());
  const auto cut1(random::sup(ps - 1));
  const auto cut2(random::between(cut1 + 1, ps));

  i_ga ret(rhs);
  for (auto i(cut1); i < cut2; ++i)
    ret.genome_[i] = lhs[i];

  ret.set_older_age(lhs.age());
  ret.signature_ = ret.hash();

  Ensures(ret.debug());
  return ret;
}

///
/// \return the signature of this individual.
///
/// Identical individuals at genotypic level have the same signature
///
hash_t i_ga::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// \return the signature of this individual.
///
/// Converts this individual in a packed byte level representation and performs
/// the _MurmurHash3_ algorithm on it.
///
hash_t i_ga::hash() const
{
  // From an individual to a packed byte stream...
  thread_local std::vector<unsigned char> packed;

  packed.clear();
  pack(&packed);

  /// ... and from a packed byte stream to a signature...

  // Length in bytes.
  const auto len(static_cast<unsigned>(packed.size() * sizeof(packed[0])));

  return vita::hash(packed.data(), len, 1973);
}

///
/// \param[out] p byte stream compacted version of the gene sequence.
///
void i_ga::pack(std::vector<unsigned char> *const p) const
{
  for (const auto &g : genome_)
  {
    // Although 16 bit are enough to contain opcodes and parameters, they are
    // usually stored in unsigned variables (i.e. 32 or 64 bit) for
    // performance reasons.
    // Anyway before hashing opcodes/parameters we convert them to 16 bit
    // types to avoid hashing more than necessary.
    const auto opcode(static_cast<std::uint16_t>(g.sym->opcode()));
    assert(g.sym->opcode() <= std::numeric_limits<opcode_t>::max());

    // DO NOT CHANGE reinterpret_cast type to std::uint8_t since even if
    // std::uint8_t has the exact same size and representation as
    // unsigned char, if the implementation made it a distinct, non-character
    // type, the aliasing rules would not apply to it
    // (see http://stackoverflow.com/q/16138237/3235496)
    const auto *const s1 = reinterpret_cast<const unsigned char *>(&opcode);
    for (std::size_t i(0); i < sizeof(opcode); ++i)
      p->push_back(s1[i]);

    assert(terminal::cast(g.sym)->parametric());
    const auto param(g.par);

    auto s2 = reinterpret_cast<const unsigned char *>(&param);
    for (std::size_t i(0); i < sizeof(param); ++i)
      p->push_back(s2[i]);
  }
}

///
/// \param[in] x second term of comparison.
/// \return `true` if the two individuals are equal.
///
/// \note
/// Age is not checked.
///
bool i_ga::operator==(const i_ga &x) const
{
  const bool eq(genome_ == x.genome_);

  assert(signature_.empty() != x.signature_.empty() ||
         (signature_ == x.signature_) == eq);

  return eq;
}

///
/// \param[in] ind an individual to compare with `this`.
/// \return a numeric measurement of the difference between `ind` and `this`
///         (the number of different genes between individuals).
///
unsigned i_ga::distance(const i_ga &ind) const
{
  const auto cs(parameters());

  unsigned d(0);
  for (auto i(decltype(cs){0}); i < cs; ++i)
    if (genome_[i] != ind.genome_[i])
      ++d;

  return d;
}

///
/// \return `true` if the individual passes the internal consistency check.
///
bool i_ga::debug() const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      print.error("Inconsistent internal status for empty individual");
      return false;
    }

    if (!signature_.empty())
    {
      print.error("Empty individual must empty signature");
      return false;
    }

    return true;
  }

  const auto ps(parameters());

  for (auto i(decltype(ps){0}); i < ps; ++i)
  {
    if (!genome_[i].sym)
    {
      print.error("Empty symbol pointer at position ", i);
      return false;
    }

    if (!genome_[i].sym->terminal())
    {
      print.error("Not-terminal symbol at position ", i);
      return false;
    }

    if (genome_[i].sym->category() != i)
    {
      print.error("Wrong category: ", i,
                  genome_[i].sym->name(), " -> ",
                  genome_[i].sym->category(), " should be ", i);
      return false;
    }
  }

  if (!signature_.empty() && signature_ != hash())
  {
    print.error("Wrong signature: ", signature_, " should be ", hash());
    return false;
  }

  return true;
}

///
/// \param[in] in input stream.
/// \param[in] e environment used to build the individual.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_ga::load_impl(std::istream &in, const environment &e)
{
  decltype(parameters()) sz;
  if (!(in >> sz))
    return false;

  decltype(genome_) v(sz);
  for (auto &g : v)
  {
    opcode_t opcode;
    if (!(in >> opcode))
      return false;

    gene temp;
    temp.sym = e.sset->decode(opcode);
    if (!temp.sym)
      return false;

    if (!(in >> temp.par))
      return false;

    g = temp;
  }

  genome_ = v;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
///
bool i_ga::save_impl(std::ostream &out) const
{
  out << parameters() << '\n';
  for (const auto &g : genome_)
    out << g.sym->opcode() << ' ' << g.par << '\n';

  return out.good();
}

///
/// \param[out] s  output stream
/// \param[in] ind individual to print.
/// \return        output stream including `ind`
///
/// \relates i_ga
///
std::ostream &operator<<(std::ostream &s, const i_ga &ind)
{
  return in_line(ind, s);
}

}  // namespace vita
