/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/ga/i_de.h"
#include "kernel/cache_hash.h"
#include "kernel/gene.h"
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
i_de::i_de(const environment &e) : individual(), genome_(e.sset->categories())
{
  Expects(e.debug(true));
  Expects(e.sset);

  const auto cs(parameters());
  assert(cs);

  for (auto c(decltype(cs){0}); c < cs; ++c)
    genome_[c] = gene(e.sset->roulette_terminal(c)).par;

  Ensures(debug());
}

///
/// \param[out] s output stream.
///
/// The output stream contains a graph, described in dot language
/// (http://www.graphviz.org/), of this individual.
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
  std::copy(de.begin(), de.end(), infix_iterator<decltype(de[0])>(s, " "));
  return s;
}

///
/// \brief Differential evolution crossover.
/// \param[in] p crossover probability.
/// \param[in] f scaling factor range (`environment.de.weight`).
/// \param[in] a first parent.
/// \param[in] b second parent.
/// \param[in] c third parent (base vector).
/// \return the offspring (trial vector).
///
/// The offspring, also called trial vector, is generated as follows:
///
///     offspring = crossover(this, c + F * (a - b))
///
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
i_de i_de::crossover(double p, const double f[2],
                     const i_de &a, const i_de &b, const i_de &c) const
{
  Expects(0.0 <= p && p <= 1.0);
  Expects(a.debug());
  Expects(b.debug());
  Expects(c.debug());

  const auto ps(parameters());
  Expects(ps == a.parameters());
  Expects(ps == b.parameters());
  Expects(ps == c.parameters());

  // The wighting factor is randomly selected from an interval for each
  // difference vector (a technique called dither). Dither improves convergence
  // behaviour significantly, especially for noisy objective functions.
  const auto rf(random::between(f[0], f[1]));

  i_de ret(c);

  for (auto i(decltype(ps){0}); i < ps - 1; ++i)
    if (random::boolean(p))
      ret[i] += rf * (a[i] - b[i]);
    else
      ret[i] = operator[](i);
  ret[ps - 1] += rf * (a[ps - 1] - b[ps - 1]);

  ret.set_older_age(std::max({age(), a.age(), b.age()}));

  ret.signature_.clear();
  Ensures(ret.debug());
  return ret;
}

///
/// \return the signature of this individual.
///
/// Identical individuals at genotypic level have the same signature
///
hash_t i_de::signature() const
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
hash_t i_de::hash() const
{
  // From an individual to a packed byte stream...
  thread_local std::vector<unsigned char> packed;

  packed.clear();
  pack(&packed);

  /// ... and from a packed byte stream to a signature...

  // Length in bytes.
  const auto len(packed.size() * sizeof(packed[0]));  // length in bytes

  return vita::hash::hash128(packed.data(), len);
}

///
/// \param[out] p byte stream compacted version of the gene sequence.
///
void i_de::pack(std::vector<unsigned char> *const p) const
{
  for (const auto &v : genome_)
  {
    auto s(reinterpret_cast<const unsigned char *>(&v));
    for (std::size_t i(0); i < sizeof(v); ++i)
      p->push_back(s[i]);
  }
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return `true` if the two individuals are equal.
///
/// \note
/// Age is not checked.
///
bool operator==(const i_de &lhs, const i_de &rhs)
{
  const bool eq(std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()));

  Ensures((lhs.signature() == rhs.signature()) == eq);
  return eq;
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparsion.
/// \return a numeric measurement of the difference between `lhs` and `rhs`
///         (taxicab / L1 distance).
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
/// \param[in] v input vector (a point in a multidimensional space).
/// \return a reference to `*this`.
///
/// Sets the individuals with values from `v`.
///
i_de &i_de::operator=(const std::vector<double> &v)
{
  Expects(v.size() == parameters());

  genome_ = v;

  return *this;
}

///
/// \return `true` if the individual passes the internal consistency check.
///
bool i_de::debug() const
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

  if (!signature_.empty() && signature_ != hash())
  {
    print.error("Wrong signature: ", signature_, " should be ", hash());
    return false;
  }

  return true;
}

///
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_de::load_impl(std::istream &in, const environment &)
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
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
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
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including `ind`.
///
/// \relates i_de
///
std::ostream &operator<<(std::ostream &s, const i_de &ind)
{
  return in_line(ind, s);
}

///
/// \return a vector of real values.
///
/// This is sweet "syntactic sugar" to manage i_de individuals as real value
/// vectors.
///
i_de::operator std::vector<double>() const
{
  return genome_;
}

}  // namespace vita
