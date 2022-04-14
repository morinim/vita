/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2022 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <algorithm>
#include <functional>
#include <map>

#include "kernel/gp/mep/i_mep.h"
#include "kernel/cache_hash.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// Generates the initial, random expressions that make up an individual.
///
/// \param[in] p base problem
///
/// The constructor is implemented so as to ensure that there is no violation
/// of the type system's constraints.
///
i_mep::i_mep(const problem &p)
  : individual(), genome_(p.env.mep.code_length, p.sset.categories()),
    best_{0, 0}, active_crossover_type_(random::sup(NUM_CROSSOVERS))
{
  Expects(size());
  Expects(p.env.mep.patch_length);
  Expects(size() > p.env.mep.patch_length);
  Expects(categories());

  const index_t i_sup(size()), patch(i_sup - p.env.mep.patch_length);
  const category_t c_sup(categories());

  // STANDARD SECTION. Filling the genome with random symbols.
  for (index_t i(0); i < patch; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(p.sset.roulette(c), i + 1, i_sup);

  // PATCH SUBSECTION. Placing terminals for satisfying constraints on types.
  for (index_t i(patch); i < i_sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(p.sset.roulette_terminal(c));

  Ensures(is_valid());
}

///
/// Creates a new individual containing genes from `gv`.
///
/// \param[in] gv vector of genes
///
/// This is useful for debugging purpose (i.e. setup *ad-hoc* individuals).
///
i_mep::i_mep(const std::vector<gene> &gv)
  : individual(),
    genome_(gv.size(),
            std::max_element(std::begin(gv), std::end(gv),
                             [](gene g1, gene g2)
                             {
                               return g1.sym->category() < g2.sym->category();
                             })->sym->category() + 1),
    best_{0, 0},
    active_crossover_type_(random::sup(NUM_CROSSOVERS))
{
  index_t i(0);

  for (const auto &g : gv)
    genome_(i++, g.sym->category()) = g;

  Ensures(is_valid());
}


///
/// Number of active symbols.
///
/// \return number of active symbols
///
/// \see size()
///
/// When `category() > 1`, active_symbols() can be greater than size(). For
/// instance consider the following individual:
///
///     [0, 1] FIFL 1 2 2 3
///     [1, 0] "car"
///     [2, 0] "plane"
///     [2, 1] 10
///     [3, 1] 20
///
/// `size() == 4` (four slots / rows) and `active_symbols() == 5`.
///
unsigned i_mep::active_symbols() const
{
  return static_cast<unsigned>(std::distance(begin(), end()));
}

///
/// \param[in] l locus of the genome
/// \return      an individual obtained from `this` choosing the gene sequence
///              starting at `l`
///
/// \note This function is often used along with the i_mep::blocks function.
///
i_mep i_mep::get_block(const locus &l) const
{
  i_mep ret(*this);

  if (ret.best_ != l)
  {
    ret.best_ = l;
    ret.signature_.clear();
  }

  Ensures(ret.is_valid());
  return ret;
}

///
/// A new individual is created mutating `this`.
///
/// \param[in] pgm probability of gene mutation
/// \param[in] prb the current problem
/// \return        number of mutations performed
///
unsigned i_mep::mutation(double pgm, const problem &prb)
{
  Expects(0.0 <= pgm && pgm <= 1.0);

  unsigned n(0);

  const auto i_size(size());
  const auto patch(i_size - prb.env.mep.patch_length);

  for (auto i(begin()); i != end(); ++i)  // here mutation affects only exons
    if (random::boolean(pgm))
    {
      const auto ix(i.locus().index);
      const auto ct(i.locus().category);

      const gene g(ix < patch ? gene(prb.sset.roulette(ct), ix + 1, i_size)
                              : gene(prb.sset.roulette_terminal(ct)));

      if (*i != g)
      {
        ++n;
        *i = g;
      }
    }

  if (n)
    signature_.clear();

  Ensures(is_valid());
  return n;
}

///
/// Calculates a set of loci referring to blocks contained in the individual.
///
/// \return a set of loci
///
/// A locus obtained with this function is usually an argument for the
/// i_mep::get_block function.
///
/// \note
/// A block is a subset of the active code composed of, at least, a function.
///
std::set<locus> i_mep::blocks() const
{
  std::set<locus> bl;

  for (auto i(begin()); i != end(); ++i)
    if (i->sym->arity())
      bl.insert(i.locus());

  return bl;
}

///
/// Create a new individual obtained from `this` replacing the original
/// symbol at locus `l` with `g`.
///
/// \param[in] l locus where replacement takes place
/// \param[in] g new gene for replacement
/// \return      a new individual with gene at locus `l` replaced by `g`
///
i_mep i_mep::replace(const locus &l, const gene &g) const
{
  i_mep ret(*this);

  ret.genome_(l) = g;
  ret.signature_.clear();

  Ensures(ret.is_valid());
  return ret;
}

///
/// Creates a new individual obtained from `this` replacing the original
/// symbol at locus `best()` with `g`.
///
/// \param[in] g new gene for replacement
/// \return      a new individual with gene at locus `best()` replaced by `g`
///
i_mep i_mep::replace(const gene &g) const
{
  return replace(best(), g);
}

///
/// \param[in] sset  a symbol set
/// \param[in] index index of a symbol in the individual
/// \return          a new individual obtained from `this` inserting a random
///                  terminal at index `index`
///
i_mep i_mep::destroy_block(index_t index, const symbol_set &sset) const
{
  Expects(index < size());

  i_mep ret(*this);
  const category_t c_sup(categories());
  for (category_t c(0); c < c_sup; ++c)
    ret.genome_(index, c) = gene(sset.roulette_terminal(c));

  ret.signature_.clear();

  Ensures(ret.is_valid());
  return ret;
}

///
/// \return the category of the individual
///
category_t i_mep::category() const
{
  return best().category;
}

///
/// \param[in] x second term of comparison.
/// \return      `true` if the two individuals are equal (symbol by symbol,
///                     including introns)
///
/// \note Age is not checked.
///
bool i_mep::operator==(const i_mep &x) const
{
  const bool eq(genome_ == x.genome_);

  Ensures(!eq
          || signature_.empty() != x.signature_.empty()
          || signature_ == x.signature_);

  return eq;
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparison
/// \return      `true` if the two individuals aren't equal
///
bool operator!=(const i_mep &lhs, const i_mep &rhs)
{
  return !(lhs == rhs);
}

///
/// \param[in] lhs first term of comparison
/// \param[in] rhs second term of comparison
/// \return        a numeric measurement of the difference between `lhs` and
///                `rhs` (the number of different genes between individuals)
///
/// \relates i_mep
///
unsigned distance(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.size() == rhs.size());
  Expects(lhs.categories() == rhs.categories());

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
/// Maps syntactically distinct (but logically equivalent) individuals to the
/// same byte stream.
///
/// \param[in]  l locus in this individual
/// \param[out] p byte stream compacted version of the gene sequence
///               starting at locus `l`
///
/// Useful for individual comparison / information retrieval.
///
void i_mep::pack(const locus &l, std::vector<std::byte> *p) const
{
  const gene &g(genome_(l));

  // Although 16 bit are enough to contain opcodes and parameters, they are
  // usually stored in unsigned variables (i.e. 32 or 64 bit) for performance
  // reasons.
  // Anyway before hashing opcodes/parameters we convert them to 16 bit types
  // to avoid hashing more than necessary.
  const auto opcode(static_cast<std::uint16_t>(g.sym->opcode()));
  assert(g.sym->opcode() <= std::numeric_limits<decltype(opcode)>::max());

  auto s1 = reinterpret_cast<const std::byte *>(&opcode);
  for (std::size_t i(0); i < sizeof(opcode); ++i)
    p->push_back(s1[i]);

  auto arity(g.sym->arity());
  if (arity)
    for (const auto &al : g.arguments())
      pack(al, p);
  else
  {
    if (terminal::cast(g.sym)->parametric())
    {
      const auto param(g.par);

      auto s2 = reinterpret_cast<const std::byte *>(&param);
      for (std::size_t i(0); i < sizeof(param); ++i)
        p->push_back(s2[i]);
    }
  }
}

///
/// Converts this individual in a packed byte level representation and
/// performs the hash algorithm on it.
///
/// \return the signature of this individual
///
hash_t i_mep::hash() const
{
  Expects(size());
  // if (empty())
  //   return hash_t();

  // From an individual to a packed byte stream...
  thread_local std::vector<std::byte> packed;

  packed.clear();
  pack(best(), &packed);

  /// ... and from a packed byte stream to a signature...
  const auto len(packed.size() * sizeof(packed[0]));  // length in bytes

  return vita::hash::hash128(packed.data(), len);
}

///
/// Signature maps syntactically distinct (but logically equivalent)
/// individuals to the same value.
///
/// \return the signature of this individual.
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
/// \return `true` if the individual passes the internal consistency check
///
bool i_mep::is_valid() const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      vitaERROR << "Inconsistent internal status for empty individual";
      return false;
    }

    if (best() != locus::npos())
    {
      vitaERROR << "Empty individual must have undefined best locus";
      return false;
    }

    if (!signature_.empty())
    {
      vitaERROR << "Empty individual and non-empty signature";
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
        vitaERROR << "Empty symbol pointer at locus " << l;
        return false;
      }

      // Correspondence between arity of the symbol and numbers of parameters.
      const auto arity(genome_(l).sym->arity());
      if (genome_(l).args.size() != arity)
      {
        vitaERROR << "Arity and actual arguments don't match";
        return false;
      }

      // Checking arguments' addresses.
      for (const auto &arg : genome_(l).args)
      {
        // Arguments' addresses must be smaller than the size of the genome.
        if (arg >= size())
        {
          vitaERROR << "Argument is out of range";
          return false;
        }

        // Function address must be smaller than its arguments' addresses.
        if (arg <= i)
        {
          vitaERROR << "Wrong reference in locus " << l;
          return false;
        }
      }
    }

  for (category_t c(0); c < categories(); ++c)
    if (!genome_(genome_.rows() - 1, c).sym->terminal())
    {
      vitaERROR << "Last symbol of type " << c
                        << " in the genome isn't a terminal";
      return false;
    }

  // Type checking.
  for (index_t i(0); i < size(); ++i)
    for (category_t c(0); c < categories(); ++c)
    {
      const locus l{i, c};

      if (genome_(l).sym->category() != c)
      {
        vitaERROR << "Wrong category: " << l << genome_(l).sym->name()
                  << " -> " << genome_(l).sym->category()
                  << " should be " << c;
        return false;
      }
    }

  if (best().index >= size())
  {
    vitaERROR << "Incorrect index for first active symbol";
    return false;
  }
  if (best().category >= categories())
  {
    vitaERROR << "Incorrect category for first active symbol";
    return false;
  }

  if (categories() == 1 && active_symbols() > size())
  {
    vitaERROR << "`active_symbols()` cannot be greater than `size()` "
                 "in single-category individuals";
    return false;
  }

  return signature_.empty() || signature_ == hash();
}

///
/// \param[in] p  active symbol set
/// \param[in] in input stream
/// \return       `true` if the object has been loaded correctly
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_mep::load_impl(std::istream &in, const symbol_set &ss)
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

    temp.sym = ss.decode(opcode);
    if (!temp.sym)
      return false;

    if (temp.sym->terminal() && terminal::cast(temp.sym)->parametric())
      if (!(in >> temp.par))
        return false;

    auto arity(temp.sym->arity());
    if (arity)
    {
      temp.args.resize(arity);

      for (auto &arg : temp.args)
        if (!(in >> arg))
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
/// \param[out] out output stream
/// \return         `true` if the object has been saved correctly
///
bool i_mep::save_impl(std::ostream &out) const
{
  out << genome_.rows() << ' ' << genome_.cols() << '\n';
  for (const auto &g : genome_)
  {
    out << g.sym->opcode();

    if (g.sym->terminal() && terminal::cast(g.sym)->parametric())
      out << ' ' << g.par;

    const auto arity(g.sym->arity());
    for (auto i(decltype(arity){0}); i < arity; ++i)
      out << ' ' << g.args[i];

    out << '\n';
  }

  if (!empty())
    out << best().index << ' ' << best().category << '\n';

  return out.good();
}

///
/// A sort of "common subexpression elimination" optimization.
///
/// \return a CSE-optimized individual
///
/// The function doesn't rely on the meaning of the symbols, just on the
/// genome layout.
///
/// Some features of the cse-optimized individual are:
/// - same signature of the unoptimized one;
/// - the interpretation yields the same value of the unoptimized one;
/// - the unoptimized individual has more active symbols.
///
i_mep i_mep::cse() const
{
  i_mep ret(*this);

  // std::map needs a compare function and there isn't a predefined less
  // operator for gene class.
  struct gene_cmp
  {
    bool operator()(const gene &a, const gene &b) const
    {
      if (a.sym->opcode() < b.sym->opcode())
        return true;

      if (a.sym->opcode() == b.sym->opcode())
      {
        if (a.sym->terminal())
          return terminal::cast(a.sym)->parametric() ? a.par < b.par : false;

        auto arity(a.sym->arity());
        for (decltype(arity) i(0); i < arity; ++i)
          if (a.args[i] < b.args[i])
            return true;
      }

      return false;
    }
  };

  std::map<gene, locus, gene_cmp> new_locus;

  for (index_t i(size()); i > 0; --i)
    for (category_t c(0); c < genome_.cols(); ++c)
    {
      const locus current_locus({i - 1, c});
      gene &g(ret.genome_(current_locus));

      std::transform(
        g.args.begin(), g.args.end(),
        g.args.begin(),
        [&, i=0u](auto arg) mutable -> gene::packed_index_t
        {
          const gene gene_arg(ret[g.locus_of_argument(i++)]);
          const auto where(new_locus.find(gene_arg));

          if (where == new_locus.end())
            return arg;

          assert(where->second.index <=
                 std::numeric_limits<gene::packed_index_t>::max());
          return where->second.index;
        });

      new_locus.try_emplace(g, current_locus);
    }

  return ret;
}

///
/// A Self-Adaptive Crossover operator.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring).
///
/// Well known elementary crossover operators traverse the problem domain in
/// different ways, exhibiting variable performances and specific problems.
/// An attempt to make the algorithm more robust is combining various search
/// strategies, encapsulated by the different elementary crossover operators
/// available, via self adaptation.
///
/// We associate to each individual the type of crossover used to create it
/// (initially this is set to a random type). This type is used afterwards to
/// determine which crossover to apply and allows the algorithm to adjust the
/// relative mixture of operators.
///
/// Here we briefly describe the elementary crossover operators that are
/// utilised:
///
/// **ONE POINT**
///
/// We randomly select a parent (between `from` and `to`) and a single locus
/// (common crossover point). The offspring is created with genes from the
/// chosen parent up to the crossover point and genes from the other parent
/// beyond that point.
/// One-point crossover is the oldest homologous crossover in tree-based GP.
///
/// **TREE**
///
/// Inserts a complete tree from one parent into the other.
/// The operation is less disruptive than other forms of crossover since
/// an entire tree is copied (not just a part).
///
/// **TWO POINTS**
///
/// We randomly select two loci (common crossover points). The offspring is
/// created with genes from the one parent before the first crossover point and
/// after the second crossover point; genes between crossover points are taken
/// from the other parent.
///
/// **UNIFORM CROSSOVER**
///
/// The i-th locus of the offspring has a 50% probability to be filled with
/// the i-th gene of `from` and 50% with i-th gene of `to`.
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
/// \note Parents must have the same size.
///
/// \remark
/// What has to be noticed is that the adaption of the parameter happens before
/// the fitness is given to it. That means that getting a good parameter
/// doesn't rise the individual's fitness but only its performance over time.
///
/// \see https://github.com/morinim/vita/wiki/bibliography#6
///
/// \relates i_mep
///
i_mep crossover(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.size() == rhs.size());

  const bool b(random::boolean());
  const i_mep &from(b ? rhs : lhs);
  i_mep          to(b ? lhs : rhs);

  switch (from.active_crossover_type_)
  {
  case i_mep::crossover_t::one_point:
    {
    const auto i_sup(from.size());
    const auto c_sup(from.categories());
    const auto cut(random::between<index_t>(1, i_sup - 1));

    for (index_t i(cut); i < i_sup; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        to.genome_(l) = from[l];
      }
    }
    break;

  case i_mep::crossover_t::two_points:
    {
    const auto i_sup(from.size());
    const auto c_sup(from.categories());

    const auto cut1(random::sup(i_sup - 1));
    const auto cut2(random::between(cut1 + 1, i_sup));

    for (index_t i(cut1); i != cut2; ++i)
      for (category_t c(0); c < c_sup; ++c)
      {
        const locus l{i, c};
        to.genome_(l) = from[l];
      }
    }
    break;

  case i_mep::crossover_t::uniform:
    {
    const auto i_sup(from.size());
    const auto c_sup(from.categories());

    for (index_t i(0); i != i_sup; ++i)
      for (category_t c(0); c < c_sup; ++c)
        if (random::boolean())
        {
          const locus l{i, c};
          to.genome_(l) = from[l];
        }
    }
    break;

  default:  // Tree crossover
    {
      auto crossover_ = [&](locus l, const auto &lambda) -> void
      {
        to.genome_(l) = from[l];

        for (const auto &al : from[l].arguments())
          lambda(al, lambda);
      };

      crossover_(random_locus(from), crossover_);
    }
    break;
  }

  to.active_crossover_type_ = from.active_crossover_type_;
  to.set_older_age(from.age());
  to.signature_.clear();

  Ensures(to.is_valid());
  return to;
}

locus random_locus(const i_mep &prg)
{
  std::set exons({prg.best()});
  auto iter(exons.begin());
  do
  {
    const auto args(prg[*iter].arguments());
    exons.insert(args.begin(), args.end());
  } while (++iter != exons.end());

  return random::element(as_const(exons));
}

namespace
{
std::ostream &language(std::ostream &s, symbol::format f, const i_mep &mep)
{
  std::function<std::string (const gene &)> language_;
  language_ = [&](const gene &g)
              {
                std::string ret(g.sym->terminal()
                                ? terminal::cast(g.sym)->display(g.par, f)
                                : function::cast(g.sym)->display(f));

                auto arity(g.sym->arity());
                for (decltype(arity) i(0); i < arity; ++i)
                {
                  const std::string from("%%" + std::to_string(i + 1) + "%%");
                  ret = replace_all(ret, from,
                                    language_(mep[g.locus_of_argument(i)]));
                }

                return ret;
              };

  std::string out(language_(mep[mep.best()]));
  if (out.length() > 2 && out.front() == '(' && out.back() == ')')
    out = out.substr(1, out.length() - 2);

  return s << out;
}

std::ostream &dump(const i_mep &mep, std::ostream &s)
{
  SAVE_FLAGS(s);

  const auto size(mep.size());
  const auto categories(mep.categories());

  const auto w1(1 + static_cast<int>(std::log10(size - 1)));
  const auto w2(1 + static_cast<int>(std::log10(categories)));

  for (index_t i(0); i < size; ++i)
    for (category_t c(0); c < categories; ++c)
    {
      const gene &g(mep[{i, c}]);

      s << '[' << std::setfill('0') << std::setw(w1) << i;

      if (categories > 1)
        s << ',' << std::setw(w2) << c;

      s  << "] " << g;

      for (const auto &l : g.arguments())
      {
        s << " [" << std::setw(w1) << l.index;
        if (categories > 1)
          s << ',' << std::setw(w2) << l.category;
        s << ']';
      }

      s << '\n';
    }

  return s;
}

void graphviz(const i_mep &mep, std::ostream &s)
{
  s << "graph {";

  for (auto i(mep.begin()); i != mep.end(); ++i)
  {
    s << 'g' << i.locus().index << '_' << i.locus().category << " [label="
      << *i << ", shape=" << (i->sym->arity() ? "box" : "circle") << "];";

    for (unsigned j(0); j < i->sym->arity(); ++j)
      s << 'g' << i.locus().index << '_' << i.locus().category << " -- g"
        << i->args[j] << '_' << function::cast(i->sym)->arg_category(j) << ';';
  }

  s << '}';
}

std::ostream &in_line(const i_mep &mep, std::ostream &s)
{
  std::function<void (locus)> in_line_;
  in_line_ = [&](locus l)
             {
               const gene &g(mep[l]);

               if (l != mep.best())
                 s << ' ';
               s << g;

               for (const auto &al : g.arguments())
                 in_line_(al);
             };

  in_line_(mep.best());
  return s;
}

std::ostream &list(const i_mep &mep, std::ostream &s)
{
  SAVE_FLAGS(s);

  const auto size(mep.size());
  const auto categories(mep.categories());

  const auto w1(1 + static_cast<int>(std::log10(size - 1)));
  const auto w2(1 + static_cast<int>(std::log10(categories)));

  const bool short_form(!out::long_form_flag(s));

  for (auto i(mep.begin()); i != mep.end(); ++i)
  {
    if (short_form && i->sym->terminal() && i.locus() != mep.best())
      continue;

    s << '[' << std::setfill('0') << std::setw(w1) << i.locus().index;

    if (categories > 1)
      s << ',' << std::setw(w2) << i.locus().category;

    s << "] " << *i;

    for (const auto &l: i->arguments())
    {
      s << ' ';

      if (short_form && mep[l].sym->terminal())
        s << mep[l];
      else
      {
        s << '[' << std::setw(w1) << l.index;
        if (categories > 1)
          s << ',' << std::setw(w2) << l.category;
        s << ']';
      }
    }

    s << '\n';
  }

  return s;
}

std::ostream &tree(const i_mep &mep, std::ostream &s)
{
  std::function<void (const gene &, const gene &, unsigned)> tree_;
  tree_ = [&](const gene &parent, const gene &child, unsigned indent)
          {
            if (child == parent ||
                parent.sym != child.sym ||
                function::cast(parent.sym)->associative() == false)
            {
              s << std::string(indent, ' ') << child << '\n';
              indent += 2;
            }

            for (const auto &l : child.arguments())
              tree_(child, mep[l], indent);
          };

  tree_(mep[mep.best()], mep[mep.best()], 0);
  return s;
}

}  // namespace

///
/// \param[out] s   output stream
/// \param[in]  ind individual to print
/// \return         output stream including `ind`
///
/// \relates i_mep
///
std::ostream &operator<<(std::ostream &s, const i_mep &ind)
{
  const auto format(out::print_format_flag(s));

  switch (format)
  {
  case out::dump_f:
    return dump(ind, s);

  case out::graphviz_f:
    graphviz(ind, s);
    return s;

  case out::in_line_f:
    return in_line(ind, s);

  case out::list_f:
    return list(ind, s);

  case out::tree_f:
    return tree(ind, s);

  default:
    assert(format >= out::language_f);
    return language(s, symbol::format(format - out::language_f), ind);
  }
}

}  // namespace vita
