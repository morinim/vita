/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <map>

#include "kernel/i_mep.h"
#include "kernel/adf.h"
#include "kernel/argument.h"
#include "kernel/random.h"
#include "kernel/ttable_hash.h"

namespace vita
{
///
/// \param[in] e base environment.
///
/// The process that generates the initial, random expressions has to be
/// implemented so as to ensure that they do not violate the type system's
/// constraints.
///
i_mep::i_mep(const environment &e)
  : individual(), genome_(e.code_length, e.sset->categories()), best_{0, 0}
{
  assert(e.sset);

  assert(size());
  assert(e.patch_length);
  assert(size() > e.patch_length);
  assert(categories());

  const index_t i_sup(size()), patch(i_sup - e.patch_length);
  const category_t c_sup(categories());

  // STANDARD SECTION. Filling the genome with random symbols.
  for (index_t i(0); i < patch; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(e.sset->roulette(c), i + 1, size());

  // PATCH SUBSECTION. Placing terminals for satisfying constraints on types.
  for (index_t i(patch); i < i_sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(e.sset->roulette_terminal(c));

  assert(debug(true));
}

///
/// \param[in] e base environment.
/// \param[in] gv vector of genes.
///
/// Create a new individual obtained containing the genes of `gv`.
/// This is useful for debugging purpouse (i.e. setup ad-hoc individuals).
///
i_mep::i_mep(const environment &e, const std::vector<gene> &gv) : i_mep(e)
{
  assert(e.sset);

  index_t i(0);
  for (const auto &g : gv)
    set({i++, g.sym->category()}, g);

  assert(debug());
}


///
/// \return the effective size of the individual.
/// \see size()
///
/// \note
/// eff_size() can be greater than size() when `category() > 1`. For instance
/// consider the following individual:
///
///     [0, 1] FIFL 1 2 2 3
///     [1, 0] "car"
///     [2, 0] "plane"
///     [2, 1] 10
///     [3, 1] 20
///
/// `size() == 4` but `eff_size() == 5`.
///
unsigned i_mep::eff_size() const
{
  return static_cast<unsigned>(std::distance(begin(), end()));
}

///
/// \param[in] l locus of the genome.
/// \return an individual obtained from `this` choosing the gene sequence
///         starting at `l`.
///
/// This function is often used along with the \ref blocks function.
///
i_mep i_mep::get_block(const locus &l) const
{
  i_mep ret(*this);
  ret.best_ = l;
  ret.signature_.clear();

  assert(ret.debug());
  return ret;
}

///
/// \brief A new individual is created mutating `this`
///
/// \param[in] p probability of gene mutation.
/// \param[in] sset a symbol set.
/// \return number of mutations performed.
///
unsigned i_mep::mutation(double p, const symbol_set &sset)
{
  assert(0.0 <= p && p <= 1.0);

  unsigned n(0);

  const auto sup(size() - 1);

  for (const auto &l : *this)  // Here mutation affects only exons
    if (random::boolean(p))
    {
      ++n;

      const auto i(l.index);
      const auto c(l.category);

      if (i < sup)
        set(l, gene(sset.roulette(c), i + 1, size()));
      else
        set(l, gene(sset.roulette_terminal(c)));
    }

/*
  // MUTATION OF THE ENTIRE GENOME (EXONS + INTRONS).
  const category_t c_sup(categories());

  for (index_t i(0); i < sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
      if (random::boolean(p))
      {
        ++n;

        set({i, c}, gene(sset->roulette(c), i + 1, size()));
      }

  for (category_t c(0); c < c_sup; ++c)
    if (random::boolean(p))
    {
      ++n;

      set({sup, c}, gene(sset->roulette_terminal(c)));
    }
*/

  assert(debug());
  return n;
}

///
/// \return a list of loci referring to active symbols.
///
/// The function calculates a set of indexes to blocks contained in `this`
/// individual.
///
/// Indexes can be used by the i_mep::get_block function.
///
/// \note
/// A block is a subset of the active code composed of, at least, a function.
///
std::vector<locus> i_mep::blocks() const
{
  std::vector<locus> bl;

  for (const auto &l : *this)
    if (genome_(l).sym->arity())
      bl.push_back(l);

  return bl;
}

///
/// \param[in] l locus where replacement takes place.
/// \param[in] g new gene for replacement.
/// \return a new individual with gene at locus `l` replaced by `g`.
///
/// Create a new individual obtained from `this` replacing the original
/// symbol at locus `l` with `g`.
///
/// \note
/// i_mep::replace method is similar to i_mep::set but the former
/// creates a new individual while the latter modify `this`.
///
i_mep i_mep::replace(const locus &l, const gene &g) const
{
  i_mep ret(*this);

  ret.set(l, g);

  assert(ret.debug());
  return ret;
}

///
/// \param[in] g new gene for replacement.
/// \return a new individual with gene at locus `best_` replaced by `g`.
///
/// Create a new individual obtained from `this` replacing the original
/// symbol at locus `best_` with `g`.
///
i_mep i_mep::replace(const gene &g) const
{
  return replace(best_, g);
}

///
/// \param[in] sset a symbol set.
/// \param[in] index index of a symbol in the individual.
/// \return a new individual obtained from `this` inserting a random terminal
///         at index `index`.
///
i_mep i_mep::destroy_block(index_t index, const symbol_set &sset) const
{
  assert(index < size());

  i_mep ret(*this);
  const category_t c_sup(categories());
  for (category_t c(0); c < c_sup; ++c)
    ret.set({index, c}, gene(sset.roulette_terminal(c)));

  assert(ret.debug());
  return ret;
}

///
/// \param[in] sset a symbol set.
/// \param[in] max_args maximum number of arguments for the ADF.
/// \return the generalized individual and a set of loci (ADF arguments
///         positions).
///
/// Changes up to `max_args` terminals (exactly `max_args` when available)
/// of `this` individual with formal arguments, thus producing the body for a
/// ADF.
///
std::pair<i_mep, std::vector<locus>> i_mep::generalize(
  unsigned max_args, const symbol_set &sset) const
{
  assert(max_args && max_args <= gene::k_args);

  std::vector<locus> terminals;

  // Step 1: mark the active terminal symbols.
  for (const auto &l : *this)
    if (genome_(l).sym->terminal())
      terminals.push_back(l);

  const auto t_size(static_cast<unsigned>(terminals.size()));

  // Step 2: shuffle the terminals and pick elements 0..n-1.
  const auto n(std::min<unsigned>(max_args, t_size));
  assert(n);

  if (n < size())
    for (auto j(decltype(n){0}); j < n; ++j)
    {
      const auto r(random::between(j, t_size));

      std::swap(terminals[j], terminals[r]);
    }

  // Step 3: randomly substitute n terminals with function arguments.
  i_mep ret(*this);
  for (auto j(decltype(n){0}); j < n; ++j)
    ret.genome_(terminals[j]).sym = sset.arg(j);
  ret.signature_.clear();

  assert(ret.debug());

  return {ret, std::vector<locus>(terminals.begin(), terminals.begin() + n)};
}

///
/// \return the category of the individual.
///
category_t i_mep::category() const
{
  return best_.category;
}

///
/// \param[in] x second term of comparison.
/// \return `true` if the two individuals are equal (symbol by symbol,
///         including introns).
///
/// \note Age is not checked.
///
bool i_mep::operator==(const i_mep &x) const
{
  const bool eq(genome_ == x.genome_);

  assert(!eq ||
         signature_.empty() != x.signature_.empty() ||
         signature_ == x.signature_);

  return eq;
}

///
/// \param[in] lhs first term of comparison.
/// \param[in] rhs second term of comparison.
/// \return a numeric measurement of the difference between `lhs` and
///         `rhs` (the number of different genes between individuals).
///
/// \relates i_mep
///
unsigned distance(const i_mep &lhs, const i_mep &rhs)
{
  assert(lhs.size() == rhs.size());
  assert(lhs.categories() == rhs.categories());

  const index_t i_sup(lhs.size());
  const category_t c_sup(lhs.categories());

  unsigned d(0);
  for (index_t i(0); i < i_sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
    {
      const locus l{i, c};
      if (lhs[l] != rhs[l])
        ++d;
    }

  return d;
}

///
/// \param[in] l locus in this individual.
/// \param[out] p byte stream compacted version of the gene sequence
///               starting at locus `l`.
///
/// This function map syntactically distinct (but logically equivalent)
/// individuals to the same byte stream. This is a very interesting
/// property, useful for individual comparison / information retrieval.
///
void i_mep::pack(const locus &l, std::vector<unsigned char> *const p) const
{
  const gene &g(genome_(l));

  // Although 16 bit are enough to contain opcodes and parameters, they are
  // usually stored in unsigned variables (i.e. 32 or 64 bit) for performance
  // reasons.
  // Anyway before hashing opcodes/parameters we convert them to 16 bit types
  // to avoid hashing more than necessary.
  const auto opcode(static_cast<std::uint16_t>(g.sym->opcode()));
  assert(g.sym->opcode() <= std::numeric_limits<decltype(opcode)>::max());

  // DO NOT CHANGE reinterpret_cast type to std::uint8_t since even if
  // std::uint8_t has the exact same size and representation as
  // unsigned char, if the implementation made it a distinct, non-character
  // type, the aliasing rules would not apply to it
  // (see http://stackoverflow.com/q/16138237/3235496)
  auto s1 = reinterpret_cast<const unsigned char *>(&opcode);
  for (std::size_t i(0); i < sizeof(opcode); ++i)
    p->push_back(s1[i]);

  if (g.sym->parametric())
  {
    const auto param(g.par);

    auto s2 = reinterpret_cast<const unsigned char *>(&param);
    for (std::size_t i(0); i < sizeof(param); ++i)
      p->push_back(s2[i]);
  }
  else
  {
    const auto arity(g.sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
      pack(g.arg_locus(i), p);
  }
}

///
/// \return the signature of this individual.
///
/// Converts this individual in a packed byte level representation and
/// performs the MurmurHash3 algorithm on it.
///
hash_t i_mep::hash() const
{
  // Calling hash() for an empty individual is UB. We could trade speed for
  // robustness adding:
  //     if (empty())  return hash_t();
  assert(size());

  // From an individual to a packed byte stream...
  thread_local std::vector<unsigned char> packed;

  packed.clear();
  pack(best_, &packed);

  /// ... and from a packed byte stream to a signature...
  const auto len(static_cast<unsigned>(
                   packed.size() * sizeof(packed[0])));  // Length in bytes

  return vita::hash(packed.data(), len, 1973);
}

///
/// \return the signature of this individual.
///
/// Signature maps syntactically distinct (but logically equivalent)
/// individuals to the same value.
///
/// In other words identical individuals at genotypic level have the same
/// signature; different individuals at the genotipic level may be mapped
/// to the same signature since the value of terminals is considered and not
/// the index.
///
/// This is a very interesting  property, useful for individual comparison,
/// information retrieval, entropy calculation...
///
hash_t i_mep::signature() const
{
  if (signature_.empty())
    signature_ = hash();

  return signature_;
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the individual passes the internal consistency check.
///
bool i_mep::debug(bool verbose) const
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

    if (best_ != locus::npos())
    {
      if (verbose)
        std::cerr << k_s_debug
                  << " Empty individual must have undefined best locus.\n";

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

  for (index_t i(0); i < size(); ++i)
    for (category_t c(0); c < categories(); ++c)
    {
      const locus l{i, c};

      if (!genome_(l).sym)
      {
        if (verbose)
          std::cerr << k_s_debug << " Empty symbol pointer at locus " << l
                    << ".\n";
        return false;
      }

      // Correspondence between arity of the symbol and numbers of parameters.
      const auto arity(genome_(l).sym->arity());
      if (genome_(l).args.size() != arity)
      {
        if (verbose)
          std::cerr << k_s_debug
                    << "Arity and actual number of parameters don't match.\n";
        return false;
      }

      // Checking arguments' addresses.
      for (const auto &arg : genome_(l).args)
      {
        // Arguments' addresses must be smaller than the size of the genome.
        if (arg >= size())
        {
          if (verbose)
            std::cerr << k_s_debug << " Argument is out of range.\n";
          return false;
        }

        // Function address must be smaller than its arguments' addresses.
        if (arg <= i)
        {
          if (verbose)
            std::cerr << k_s_debug << " Wrong reference in locus " << l
                      << ".\n";
          return false;
        }
      }
    }

  for (category_t c(0); c < categories(); ++c)
    if (!genome_(genome_.rows() - 1, c).sym->terminal())
    {
      if (verbose)
        std::cerr << k_s_debug << " Last symbol of type " << c
                  << " in the genome isn't a terminal.\n";
      return false;
    }

  // Type checking.
  for (index_t i(0); i < size(); ++i)
    for (category_t c(0); c < categories(); ++c)
    {
      const locus l{i, c};

      if (genome_(l).sym->category() != c)
      {
        if (verbose)
          std::cerr << k_s_debug << " Wrong category: " << l
                    << genome_(l).sym->display() << " -> "
                    << genome_(l).sym->category() << " should be " << c
                    << '\n';
        return false;
      }
    }

  if (best_.index >= size())
  {
    if (verbose)
      std::cerr << k_s_debug << " Incorrect index for first active symbol.\n";
    return false;
  }
  if (best_.category >= categories())
  {
    if (verbose)
      std::cerr << k_s_debug
                << " Incorrect category for first active symbol.\n";
    return false;
  }

  if (categories() == 1 && eff_size() > size())
  {
    if (verbose)
      std::cerr << k_s_debug
                << "eff_size() cannot be greater than size() in single "
                   "category individuals.\n";
    return false;
  }

  return signature_.empty() || signature_ == hash();
}

///
/// \param[out] s output stream.
/// \param[in] id used for subgraph plot (usually this is an empty string).
///
/// The output stream contains a graph of this individual described in dot
/// language (see http://www.graphviz.org/).
///
void i_mep::graphviz(std::ostream &s, const std::string &id) const
{
  if (id.empty())
    s << "graph";
  else
    s << "subgraph " << id;
  s << " {";

  for (const auto &l : *this)
  {
    const gene &g(genome_(l));

    s << 'g' << l.index << '_' << l.category << " [label=" << g
      << ", shape=" << (g.sym->arity() ? "box" : "circle") << "];";

    for (unsigned j(0); j < g.sym->arity(); ++j)
      s << 'g' << l.index << '_' << l.category << " -- g" << g.args[j]
        << '_' << function::cast(g.sym)->arg_category(j) << ';';
  }

  s << '}';
}

///
/// \param[out] s output stream
///
/// The individual is printed on a single line with symbols separated by
/// spaces. Not at all human readable, but a compact representation for
/// import / export.
///
/// \note
/// Prints active genes visiting the genome in pre-order.
///
std::ostream &i_mep::in_line(std::ostream &s) const
{
  std::function<void (locus)> in_line_(
    [&](locus l)
    {
      const gene &g(genome_(l));

      if (l != best_)
        s << ' ';
      s << g;

      const auto arity(g.sym->arity());
      for (auto i(decltype(arity){0}); i < arity; ++i)
        in_line_(g.arg_locus(i));
    });

  in_line_(best_);
  return s;
}

///
/// \param[out] s output stream.
/// \param[in] short_form if `true` prints a shorter and more human-readable
///                       form of the genome.
///
/// Do you remember C=64 list? :-)
///
/// 10 PRINT "HOME"
/// 20 PRINT "SWEET"
/// 30 GOTO 10
///
std::ostream &i_mep::list(std::ostream &s, bool short_form) const
{
  SAVE_FLAGS(s);

  const auto w1(1 + static_cast<int>(std::log10(size() - 1)));
  const auto w2(1 + static_cast<int>(std::log10(categories())));

  for (const auto &l : *this)
  {
    const gene &g(genome_(l));

    if (short_form && g.sym->terminal() && l != best_)
      continue;

    s << '[' << std::setfill('0') << std::setw(w1) << l.index;

    if (categories() > 1)
      s << ',' << std::setw(w2) << l.category;

    s << "] " << g;

    const auto arity(g.sym->arity());
    for (auto j(decltype(arity){0}); j < arity; ++j)
    {
      s << ' ';

      const auto arg_j(g.arg_locus(j));

      if (short_form && genome_(arg_j).sym->terminal())
        s << genome_(arg_j);
      else
      {
        s << '[' << std::setw(w1) << arg_j.index;
        if (categories() > 1)
          s << ',' << std::setw(w2) << arg_j.category;
        s << ']';
      }
    }

    s << '\n';
  }

  return s;
}

///
/// \param[out] s output stream.
///
std::ostream &i_mep::tree(std::ostream &s) const
{
  std::function<void (locus, unsigned, locus)> tree_(
    [&](locus child, unsigned indent, locus parent)
    {
      const gene &g(genome_(child));

      if (child == parent ||
          !genome_(parent).sym->associative() ||
          genome_(parent).sym != g.sym)
      {
        s << std::string(indent, ' ') << g << '\n';
        indent += 2;
      }

      const auto arity(g.sym->arity());
      for (auto i(decltype(arity){0}); i < arity; ++i)
        tree_(g.arg_locus(i), indent, child);
    });

  tree_(best_, 0, best_);
  return s;
}

///
/// \param[out] s output stream.
///
/// Print the complete content of this individual.
///
std::ostream &i_mep::dump(std::ostream &s) const
{
  SAVE_FLAGS(s);

  const auto w1(1 + static_cast<int>(std::log10(size() - 1)));
  const auto w2(1 + static_cast<int>(std::log10(categories())));

  for (index_t i(0); i < size(); ++i)
    for (category_t c(0); c < categories(); ++c)
    {
      const gene &g(genome_(i, c));

      s << '[' << std::setfill('0') << std::setw(w1) << i;

      if (categories() > 1)
        s << ',' << std::setw(w2) << c;

      s  << "] " << g;

      const auto arity(g.sym->arity());
      for (auto j(decltype(arity){0}); j < arity; ++j)
      {
        const auto arg_j(g.arg_locus(j));

        s << " [" << std::setw(w1) << arg_j.index;
        if (categories() > 1)
          s << ',' << std::setw(w2) << arg_j.category;
        s << ']';
      }

      s << '\n';
    }

  return s;
}

///
/// \param[out] s output stream.
/// \param[in] ind individual to print.
/// \return output stream including `ind`.
///
std::ostream &operator<<(std::ostream &s, const i_mep &ind)
{
  return ind.list(s, true);
}

///
/// \param[in] e environment used to build the individual.
/// \param[in] in input stream.
/// \return `true` if the object has been loaded correctly.
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_mep::load_nvi(std::istream &in, const environment &e)
{
  unsigned rows, cols;
  if (!(in >> rows >> cols))
    return false;

  // The matrix class has a basic support for serialization but we cannot
  // take advantage of it here: the gene class needs a special management
  // (among other things it needs access to the symbol_set to decode the
  // symbols).
  decltype(genome_) genome(rows, cols);
  for (auto &g : genome)
  {
    opcode_t opcode;
    if (!(in >> opcode))
      return false;

    gene temp;

    temp.sym = e.sset->decode(opcode);
    if (!temp.sym)
      return false;

    if (temp.sym->parametric())
      if (!(in >> temp.par))
        return false;

    const auto arity(temp.sym->arity());
    if (arity)
    {
      temp.args.resize(arity);

      for (unsigned i(0); i < arity; ++i)
        if (!(in >> temp.args[i]))
          return false;
    }

    g = temp;
  }

  auto best(locus::npos());

  if (rows && !(in >> best.index >> best.category))
      return false;

  best_ = best;
  genome_ = genome;

  return true;
}

///
/// \param[out] out output stream.
/// \return `true` if the object has been saved correctly.
///
bool i_mep::save_nvi(std::ostream &out) const
{
  out << genome_.rows() << ' ' << genome_.cols() << '\n';
  for (const auto &g : genome_)
  {
    out << g.sym->opcode();

    if (g.sym->parametric())
      out << ' ' << g.par;

    const auto arity(g.sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
      out << ' ' << g.args[i];

    out << '\n';
  }

  if (!empty())
    out << best_.index << ' ' << best_.category << '\n';

  return out.good();
}

///
/// \brief A sort of "common subexpression elimination" optimization
///
/// The function doesn't rely on the meaning of the symbols, just on the
/// genome layout.
///
i_mep i_mep::compress() const
{
  i_mep ret(*this);

  // std::map needs a compare function and there isn't a predefined less
  // operator for gene class.
  struct cmp_gene
  {
    bool operator()(const gene &a, const gene &b) const
    {
      if (a.sym->opcode() < b.sym->opcode())
        return true;

      if (a.sym->opcode() == b.sym->opcode())
      {
        if (a.sym->terminal())
          return a.sym->parametric() ? a.par < b.par : false;

        const auto arity(a.sym->arity());
        for (auto i(decltype(arity){0}); i < arity; ++i)
          if (a.args[i] < b.args[i])
            return true;
      }

      return false;
    }
  };

  std::map<gene, locus, cmp_gene> new_locus;

  using ARRAY_ELEM_TYPE =
    typename std::remove_reference<decltype(gene::args[0])>::type;

  for (index_t i(size()); i > 0; --i)
    for (category_t c(0); c < ret.genome_.cols(); ++c)
    {
      gene &g(ret.genome_(i - 1, c));

      const auto arity(g.sym->arity());
      for (auto p(decltype(arity){0}); p < arity; ++p)
      {
        const auto where(new_locus.find(ret[g.arg_locus(p)]));
        if (where != new_locus.end())
        {
          assert(where->second.index <=
                 std::numeric_limits<ARRAY_ELEM_TYPE>::max());

          g.args[p] = static_cast<ARRAY_ELEM_TYPE>(where->second.index);
        }
      }

      new_locus.insert({g, {i - 1, c}});
    }

  return ret;
}

#if defined(UNIFORM_CROSSOVER)
///
/// \brief Uniform Crossover
/// \param[in] rhs the second parent.
/// \return the result of the crossover (we only generate a single offspring).
///
/// The i-th locus of the offspring has a 50% probability to be filled with
/// the i-th gene of `this` and 50% with i-th gene of `rhs`. Parents must
/// have the same size.
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
i_mep i_mep::crossover(i_mep rhs) const
{
  assert(rhs.debug());
  assert(size() == rhs.size());

  for (auto &l : rhs)
    if (random::boolean())
      rhs.set(l, operator[](l));

  rhs.set_older_age(age());

  assert(rhs.debug(true));
  return rhs;
}
#elif defined(ONE_POINT_CROSSOVER)
///
/// \brief Tree Crossover
/// \param[in] rhs the second parent.
/// \return the result of the crossover (we only generate a single offspring).
///
/// We randomly select a parent (between `this` and `rhs`) and a single locus
/// (common crossover point). The offspring is created with genes from the
/// chosen parent up to the crossover point and genes from the other parent
/// beyond that point.
/// One-point crossover is the oldest homologous crossover in tree-based GP.
///
/// \note
/// Parents must have the same size.
///
i_mep i_mep::crossover(i_mep rhs) const
{
  assert(rhs.debug());
  assert(size() == rhs.size());

  const auto i_sup(size());
  const auto c_sup(categories());

  const auto cut(random::between<index_t>(1, i_sup - 1));

  if (random::boolean())
    for (index_t i(cut); i < i_sup; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        rhs.set(l, operator[](l));
      }
  else
    for (index_t i(0); i < cut; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        rhs.set(l, operator[](l));
      }

  rhs.set_older_age(age());

  assert(rhs.debug());
  return rhs;
}
#elif defined(TREE_CROSSOVER)
///
/// \brief Two Point Crossover
/// \param[in] rhs the second parent.
/// \return the result of the crossover (we only generate a single offspring).
///
/// This crossover insert a complete tree from one parent into the other ones.
/// This is somewhat less disruptive than other forms of crossover since
/// an entire tree is copied (not just a part). Anyway, due to size
/// constraints, existing genes in the target individual could be overwritten.
///
/// \note
/// Parents must have the same size.
///
i_mep i_mep::crossover(i_mep rhs) const
{
  assert(rhs.debug());
  assert(size() == rhs.size());

  for (auto it(std::next(begin(), eff_size())); it != end(); ++it)
    rhs.set(*it, operator[](*it));

  rhs.set_older_age(age());

  assert(rhs.debug());
  return rhs;
}
#else  // TWO_POINT_CROSSOVER (default)
///
/// \brief Two Point Crossover
/// \param[in] rhs the second parent.
/// \return the result of the crossover (we only generate a single offspring).
///
/// We randomly select a parent (between `this` and `rhs`) and a two loci
/// (common crossover points). The offspring is created with genes from the
/// chosen parent before the first crossover point and after the second
/// crossover point; genes between crossover points are taken from the other
/// parent.
///
/// \note
/// Parents must have the same size.
///
i_mep i_mep::crossover(i_mep rhs) const
{
  assert(rhs.debug());
  assert(size() == rhs.size());

  const auto i_sup(size());
  const auto c_sup(categories());

  const auto cut1(random::sup(i_sup - 1));
  const auto cut2(random::between(cut1 + 1, i_sup));

  if (random::boolean())
  {
    for (index_t i(cut1); i < cut2; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        rhs.set(l, operator[](l));
      }
  }
  else
  {
    for (index_t i(0); i < cut1; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        rhs.set(l, operator[](l));
      }

    for (index_t i(cut2); i < i_sup; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        rhs.set(l, operator[](l));
      }
  }

  rhs.set_older_age(age());

  assert(rhs.debug());
  return rhs;
}
#endif
}  // namespace vita
