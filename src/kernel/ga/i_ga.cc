/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_ga.h"
#include "kernel/cache_hash.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// Constructs a new, random GA individual.
///
/// \param[in] p current problem
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they do not violate the type system's
/// constraints.
///
i_ga::i_ga(const problem &p) : individual(), genome_(p.sset.categories())
{
  Expects(parameters());

  std::generate(genome_.begin(), genome_.end(),
                [&, n = 0]() mutable
                {
                  return static_cast<value_type>(
                    p.sset.roulette_terminal(n++).init());
                });

  Ensures(is_valid());
}

///
/// Produces a dot-language representation of this individual.
///
/// \param[out] s output stream
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
  std::copy(ga.begin(), ga.end(), infix_iterator<i_ga::value_type>(s, " "));
  return s;
}

///
/// Mutates the current individual.
///
/// \param[in] pgm probability of gene mutation
/// \param[in] prb the current problem
/// \return        number of mutations performed
///
/// \relates i_ga
///
unsigned i_ga::mutation(double pgm, const problem &prb)
{
  Expects(0.0 <= pgm);
  Expects(pgm <= 1.0);

  unsigned n(0);

  const auto ps(parameters());
  for (category_t c(0); c < ps; ++c)
    if (random::boolean(pgm))
      if (const auto g =
          static_cast<value_type>(prb.sset.roulette_terminal(c).init());
          g != genome_[c])
      {
        ++n;
        genome_[c] = g;
      }

  if (n)
    signature_ = hash();

  Ensures(is_valid());
  return n;
}

///
/// Two points crossover.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        crossover children (we only generate a single offspring)
///
/// We randomly select two loci (common crossover points). The offspring is
/// created with genes from the `rhs` parent before the first crossover
/// point and after the second crossover point; genes between crossover
/// points are taken from the `lhs` parent.
///
/// \note Parents must have the same size.
///
/// \relates i_ga
///
i_ga crossover(const i_ga &lhs, const i_ga &rhs)
{
  Expects(lhs.parameters() == rhs.parameters());

  const auto ps(lhs.parameters());
  const auto cut1(random::sup(ps - 1));
  const auto cut2(random::between(cut1 + 1, ps));

  i_ga ret(rhs);
  for (auto i(cut1); i < cut2; ++i)
    ret.genome_[i] = lhs[i];  // not using `operator[](unsigned)` to avoid
                              // multiple signature resets.
  ret.set_older_age(lhs.age());
  ret.signature_ = ret.hash();

  Ensures(ret.is_valid());
  return ret;
}

///
/// The signature (hash value) of this individual.
///
/// \return the signature of this individual
///
/// Identical individuals at genotypic level have the same signature. The
/// signature is calculated just at the first call and then stored inside the
/// individual.
///
hash_t i_ga::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// Hashes the current individual.
///
/// \return the hash value of the individual
///
/// Converts this individual in a packed representation (raw sequence of bytes)
/// and performs the *MurmurHash3* algorithm on it.
///
hash_t i_ga::hash() const
{
  // Length in bytes.
  const auto len(genome_.size() * sizeof(genome_[0]));  // length in bytes
  return vita::hash::hash128(genome_.data(), len);
}

///
/// \param[in] x second term of comparison
/// \return      `true` if the two individuals are equal
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
/// \param[in] ind an individual to compare with `this`
/// \return        a numeric measurement of the difference between `ind` and
///                `this` (the number of different genes between individuals)
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
/// \return `true` if the individual passes the internal consistency check
///
bool i_ga::is_valid() const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      vitaERROR << "Inconsistent internal status for empty individual";
      return false;
    }

    if (!signature_.empty())
    {
      vitaERROR << "Empty individual must empty signature";
      return false;
    }

    return true;
  }

  if (!signature_.empty() && signature_ != hash())
  {
    vitaERROR << "Wrong signature: " << signature_ << " should be " << hash();
    return false;
  }

  return true;
}

///
/// \param[in] in input stream
/// \return       `true` if the object has been loaded correctly
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_ga::load_impl(std::istream &in, const symbol_set &)
{
  decltype(parameters()) sz;
  if (!(in >> sz))
    return false;

  decltype(genome_) v(sz);
  for (auto &g : v)
    if (!(in >> g))
      return false;

  genome_ = v;

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if the object has been saved correctly
///
bool i_ga::save_impl(std::ostream &out) const
{
  out << parameters() << '\n';
  for (const auto &g : genome_)
    out << g << '\n';

  return out.good();
}

///
/// \param[out] s  output stream
/// \param[in] ind individual to print
/// \return        output stream including `ind`
///
/// \relates i_ga
///
std::ostream &operator<<(std::ostream &s, const i_ga &ind)
{
  return in_line(ind, s);
}

///
/// This is sweet "syntactic sugar" to manage i_ga individuals as integer value
/// vectors.
///
/// \return a vector of integer values
///
i_ga::operator std::vector<i_ga::value_type>() const
{
  return genome_;
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparison
/// \return        `true` if the two individuals are equal
///
/// \note Age is not checked.
///
bool operator==(const i_ga &lhs, const i_ga &rhs)
{
  const bool eq(std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));

  Ensures((lhs.signature() == rhs.signature()) == eq);
  return eq;
}

}  // namespace vita
