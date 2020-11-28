/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_de.h"
#include "kernel/cache_hash.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// Constructs a new, random DE individual.
///
/// \param[in] p current problem
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they don't violate the type system's
/// constraints.
///
i_de::i_de(const problem &p) : individual(), genome_(p.sset.categories())
{
  Expects(parameters());

  std::generate(genome_.begin(), genome_.end(),
                [&, n = 0]() mutable
                {
                  return p.sset.roulette_terminal(n++).init();
                });

  Ensures(is_valid());
}

///
/// Inserts into the output stream the graph representation of the individual.
///
/// \param[out] s output stream
///
/// \note
/// The format used to describe the graph is the dot language
/// (http://www.graphviz.org/).
///
void i_de::graphviz(std::ostream &s) const
{
  s << "graph {";

  for (const auto &g : genome_)
    s << "g [label=" << g << ", shape=circle];";

  s << '}';
}

///
/// Prints the genes of the individual.
///
/// \param[in]  de data to be printed
/// \param[out] s  output stream
/// \return        a reference to the output stream
///
/// \relates i_de
///
std::ostream &in_line(const i_de &de, std::ostream &s)
{
  std::copy(de.begin(), de.end(), infix_iterator<i_de::value_type>(s, " "));
  return s;
}

///
/// Differential evolution crossover.
///
/// \param[in] p crossover probability
/// \param[in] f scaling factor range (`environment.de.weight`)
/// \param[in] a first parent
/// \param[in] b second parent
/// \param[in] c third parent (base vector)
/// \return      the offspring (trial vector)
///
/// The offspring, also called trial vector, is generated as follows:
///
///     offspring = crossover(this, c + F * (a - b))
///
/// first the search direction is defined by calculating a *difference vector*
/// between the pair of vectors `a` and `b` (usually choosen at random from the
/// population). This difference vector is scaled by using the *scale factor*
/// `f`. This scaled difference vector is then added to a third vector `c`,
/// called the *base vector*. As a result a new vector is obtained, known as
/// the *mutant vector*. The mutant vector is recombined, based on a used
/// defined parameter, called *crossover probability*, with the target vector
/// `this` (also called *parent vector*).
///
/// This way no separate probability distribution has to be used which makes
/// the scheme completely self-organizing.
///
/// `a` and `b` are used for mutation, `this` and `c` for crossover.
///
i_de i_de::crossover(double p, const range_t<double> &f,
                     const i_de &a, const i_de &b, const i_de &c) const
{
  Expects(0.0 <= p && p <= 1.0);

  const auto ps(parameters());
  Expects(ps == a.parameters());
  Expects(ps == b.parameters());
  Expects(ps == c.parameters());

  // The weighting factor is randomly selected from an interval for each
  // difference vector (a technique called dither). Dither improves convergence
  // behaviour significantly, especially for noisy objective functions.
  const auto rf(random::in(f));

  i_de ret(c);

  for (auto i(decltype(ps){0}); i < ps - 1; ++i)
    if (random::boolean(p))
      ret[i] += rf * (a[i] - b[i]);
    else
      ret[i] = operator[](i);
  ret[ps - 1] += rf * (a[ps - 1] - b[ps - 1]);

  ret.set_older_age(std::max({age(), a.age(), b.age()}));

  ret.signature_.clear();
  Ensures(ret.is_valid());
  return ret;
}

///
/// \return the signature of this individual
///
/// Identical individuals, at genotypic level, have the same signature
///
hash_t i_de::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// Hashes the current individual.
///
/// \return the signature of this individual
///
/// The signature is obtained performing *MurmurHash3* on the individual.
///
hash_t i_de::hash() const
{
  const auto len(genome_.size() * sizeof(genome_[0]));
  return vita::hash::hash128(genome_.data(), len);
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparsion
/// \return        a numeric measurement of the difference between `lhs` and
///                `rhs` (taxicab / L1 distance)
///
double distance(const i_de &lhs, const i_de &rhs)
{
  Expects(lhs.parameters() == rhs.parameters());

  const auto cs(lhs.parameters());
  assert(cs);

  double d(0.0);
  for (unsigned i(0); i < cs; ++i)
    d += std::fabs(lhs[i] - rhs[i]);

  Ensures(d >= 0.0);
  return d;
}

///
/// Sets up the individual with values from a vector.
///
/// \param[in] v input vector (a point in a multidimensional space)
/// \return      a reference to `*this`
///
i_de &i_de::operator=(const std::vector<i_de::value_type> &v)
{
  Expects(v.size() == parameters());

  genome_ = v;

  return *this;
}

///
/// \return `true` if the individual passes the internal consistency check
///
bool i_de::is_valid() const
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
      vitaERROR << "Empty individual must have empty signature";
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
bool i_de::load_impl(std::istream &in, const symbol_set &)
{
  decltype(parameters()) sz;
  if (!(in >> sz))
    return false;

  decltype(genome_) v(sz);
  for (auto &g : v)
    if (!load_float_from_stream(in, &g))
      return false;

  genome_ = v;

  return true;
}

///
/// \param[out] out output stream
/// \return         `true` if the object has been saved correctly
///
bool i_de::save_impl(std::ostream &out) const
{
  out << parameters() << '\n';
  for (const auto &v : genome_)
  {
    save_float_to_stream(out, v);
    out << '\n';
  }

  return out.good();
}

///
/// \param[out] s  output stream
/// \param[in] ind individual to print
/// \return        output stream including `ind`
///
/// \relates i_de
///
std::ostream &operator<<(std::ostream &s, const i_de &ind)
{
  return in_line(ind, s);
}

///
/// This is sweet "syntactic sugar" to manage i_de individuals as real value
/// vectors.
///
/// \return a vector of real values
///
i_de::operator std::vector<i_de::value_type>() const
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
bool operator==(const i_de &lhs, const i_de &rhs)
{
  const bool eq(std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));

  Ensures((lhs.signature() == rhs.signature()) == eq);
  return eq;
}

}  // namespace vita
