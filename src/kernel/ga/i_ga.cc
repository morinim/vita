/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014, 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_ga.h"
#include "kernel/ttable_hash.h"

namespace vita
{
///
/// \param[in] e base environment.
/// \param[in] ss a symbol set.
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they do not violate the type system's
/// constraints.
///
i_ga::i_ga(const environment &e, const symbol_set &ss)
  : individual(e, ss), genome_(ss.categories())
{
  assert(e.debug(true, true));

  assert(parameters());

  const auto cs(ss.categories());
  assert(cs);

  for (auto c(decltype(cs){0}); c < cs; ++c)
    genome_[c] = gene(ss.roulette_terminal(c));

  assert(debug(true));
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
/// \param[out] s output stream
///
/// Prints genes of the individual.
///
std::ostream &i_ga::in_line(std::ostream &s) const
{
  std::copy(genome_.begin(), genome_.end(), infix_iterator<gene>(s, " "));
  return s;
}

///
/// \param[out] s output stream
///
/// Do you remember C=64 list? :-)
///
///     01 123.37
///     02 13
///     03 99.99
///     ...
///
std::ostream &i_ga::list(std::ostream &s) const
{
  const auto cs(sset_->categories());
  const auto w(1 + static_cast<int>(std::log10(cs)));

  unsigned i(0);
  for (const auto &g : genome_)
  {
    assert(i == g.sym->category());

    s << '[' << std::setfill('0') << std::setw(w) << i++ << "] " << g
      << '\n';
  }

  return s;
}

///
/// \param[out] s output stream.
///
std::ostream &i_ga::tree(std::ostream &s) const
{
  return in_line(s);
}

///
/// \param[in] p probability of gene mutation.
/// \return number of mutations performed.
///
/// A new individual is created mutating `this`.
///
/// \note
/// This function is included for compatibility with GP recombination
/// strategies. Typical differential evolution GA algorithm won't use
/// this method.
///
unsigned i_ga::mutation(double p)
{
  assert(0.0 <= p);
  assert(p <= 1.0);

  unsigned n(0);

  const auto ps(parameters());
  for (category_t c(0); c < ps; ++c)
    if (random::boolean(p))
    {
      ++n;

      genome_[c] = gene(sset_->roulette_terminal(c));
    }

  signature_ = hash();

  assert(debug());
  return n;
}

///
/// \brief Two points crossover
/// \param[in] rhs the second parent.
/// \return the result of the crossover (we only generate a single
///         offspring).
///
/// We randomly select two loci (common crossover points). The offspring is
/// created with genes from the `rhs` parent before the first crossover
/// point and after the second crossover point; genes between crossover
/// points are taken from the other `*this` parent.
///
/// \note
/// - Parents must have the same size.
/// - This function is included for compatibility with GP recombination
///   strategies. Typical differential evolution GA algorithm won't use
///   this method.
///
i_ga i_ga::crossover(i_ga rhs) const
{
  assert(rhs.debug());

  const auto ps(parameters());
  assert(ps == rhs.parameters());

  const auto cut1(random::sup(ps - 1));
  const auto cut2(random::between(cut1 + 1, ps));

  for (auto i(cut1); i < cut2; ++i)
    rhs.genome_[i] = genome_[i];

  rhs.set_older_age(age());

  rhs.signature_ = rhs.hash();
  assert(rhs.debug());
  return rhs;
}

///
/// \brief Differential evolution crossover
/// \param[in] a first parent.
/// \param[in] b second parent.
/// \param[in] c third parent.
/// \return the offspring.
///
///     offspring = crossover(this, c + F * (a - b))
///
/// The offspring, also called trial vector, is generated as follows:
/// first the search direction is defined by calculating a
/// _difference vector_ between the pair of vectors `a` and `b` (usually
/// choosen at random from the population). This difference vector is scaled by
/// using the _scale factor_ `f`. This scaled difference vector is then added
/// to a third vector `c`, called the _base vector_. As a result a new vector
/// is obtained, known as the _mutant vector_. The mutant vector is recombined,
/// based on a used defined parameter, called _crossover probability_, with the
/// target vector `this` (also called _parent vector_).
///
/// This way no separate probability distribution has to be used which makes
/// the scheme completely self-organizing.
///
/// `a` and `b` are used for mutation, `this` and `c` for crossover.
///
i_ga i_ga::crossover(const i_ga &a, const i_ga &b, i_ga c) const
{
  assert(a.debug());
  assert(b.debug());
  assert(c.debug());

  const auto ps(parameters());
  assert(ps == a.parameters());
  assert(ps == b.parameters());
  assert(ps == c.parameters());

  const auto p_cross(env_->p_cross);
  assert(0.0 <= p_cross);
  assert(p_cross <= 1.0);

  const auto &f(env_->de.weight);  // scaling factor range

  for (auto i(decltype(ps){0}); i < ps; ++i)
    if (random::boolean(p_cross))
      c[i] += random::between(f[0], f[1]) * (a[i] - b[i]);
    else
      c[i] = operator[](i);

  c.set_older_age(std::max({age(), a.age(), b.age()}));

  c.signature_.clear();
  assert(c.debug());
  return c;
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

    assert(g.sym->parametric());
    const auto param(g.par);

    auto s2 = reinterpret_cast<const unsigned char *>(&param);
    for (std::size_t i(0); i < sizeof(param); ++i)
      p->push_back(s2[i]);
  }
}

///
/// \param[in] x second term of comparison.
/// \return `true` if the two individuals are equal (symbol by symbol,
///         including introns).
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
  const auto cs(sset_->categories());

  unsigned d(0);
  for (auto i(decltype(cs){0}); i < cs; ++i)
    if (genome_[i] != ind.genome_[i])
      ++d;

  return d;
}

///
/// \param[in] v input vector (a point in a multidimensional space).
/// \return a reference to `*this`.
///
/// Sets the individuals with values from `v`.
///
i_ga &i_ga::operator=(const std::vector<gene::param_type> &v)
{
  const auto ps(parameters());
  assert(v.size() == ps);

  for (auto i(decltype(ps){0}); i < ps; ++i)
    operator[](i) = v[i];

  return *this;
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the individual passes the internal consistency check.
///
bool i_ga::debug(bool verbose) const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " Inconsistent internal status for empty individual.\n";

      return false;
    }

    if (!signature_.empty())
    {
      if (verbose)
        std::cerr << k_s_debug << " Empty individual must empty signature.\n";

      return false;
    }

    return true;
  }

  const auto ps(parameters());

  for (auto i(decltype(ps){0}); i < ps; ++i)
  {
    if (!genome_[i].sym)
    {
      if (verbose)
        std::cerr << k_s_debug << " Empty symbol pointer at position " << i
                  << ".\n";
      return false;
    }

    if (!genome_[i].sym->terminal())
    {
      if (verbose)
        std::cerr << k_s_debug << " Not-terminal symbol at position " << i
                  << ".\n";

      return false;
    }

    if (genome_[i].sym->category() != i)
    {
      if (verbose)
        std::cerr << k_s_debug << " Wrong category: " << i
                  << genome_[i].sym->display() << " -> "
                  << genome_[i].sym->category() << " should be " << i
                  << '\n';
      return false;
    }
  }

  if (!signature_.empty() && signature_ != hash())
  {
    if (verbose)
      std::cerr << k_s_debug << " Wrong signature: " << signature_
                << " should be " << hash() << '\n';
    return false;
  }

  return env_->debug(verbose, true);
}

///
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_ga::load_nvi(std::istream &in)
{
  decltype(genome_)::size_type sz;
  if (!(in >> sz) || !sz)
    return false;

  decltype(genome_) v(sz);
  for (auto &e : v)
  {
    opcode_t opcode;
    if (!(in >> opcode))
      return false;

    gene g;
    g.sym = sset_->decode(opcode);
    if (!g.sym)
      return false;

    if (!(in >> g.par))
      return false;

    e = g;
  }

  genome_ = v;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
///
bool i_ga::save_nvi(std::ostream &out) const
{
  out << parameters() << '\n';
  for (const auto &g : genome_)
    out << g.sym->opcode() << ' ' << g.par << '\n';

  return out.good();
}

///
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including `ind`.
///
std::ostream &operator<<(std::ostream &s, const i_ga &ind)
{
  return ind.in_line(s);
}

///
/// \return a vector of real values.
///
/// This is sweet "syntactic sugar" to manage i_ga individuals as real value
/// vectors.
///
i_ga::operator std::vector<double>() const
{
  const auto ps(parameters());
  std::vector<double> v(ps);

  for (auto i(decltype(ps){0}); i < ps; ++i)
    v[i] = operator[](i);

  return v;
}

}  // namespace vita
