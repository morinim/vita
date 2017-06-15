/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2017 EOS di Manlio Morini.
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
#include "kernel/cache_hash.h"
#include "kernel/log.h"
#include "kernel/random.h"

namespace vita
{
///
/// Generates the initial, random expressions that make up an individual.
///
/// \param[in] e base environment
///
/// The constructor is implemented so as to ensure that there is no violation
/// of the type system's constraints.
///
i_mep::i_mep(const environment &e)
  : individual(), genome_(e.code_length, e.sset->categories()), best_{0, 0}
{
  Expects(e.sset);
  Expects(size());
  Expects(e.patch_length);
  Expects(size() > e.patch_length);
  Expects(categories());

  const index_t i_sup(size()), patch(i_sup - e.patch_length);
  const category_t c_sup(categories());

  // STANDARD SECTION. Filling the genome with random symbols.
  for (index_t i(0); i < patch; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(e.sset->roulette(c), i + 1, i_sup);

  // PATCH SUBSECTION. Placing terminals for satisfying constraints on types.
  for (index_t i(patch); i < i_sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
      genome_(i, c) = gene(e.sset->roulette_terminal(c));

  Ensures(debug());
}

///
/// Creates a new individual containing genes from `gv`.
///
/// \param[in] gv vector of genes
///
/// This is useful for debugging purpouse (i.e. setup ad-hoc individuals).
///
i_mep::i_mep(const std::vector<gene> &gv)
  : individual(),
    genome_(static_cast<unsigned>(gv.size()),
            std::max_element(std::begin(gv), std::end(gv),
                             [](gene g1, gene g2)
                             {
                               return g1.sym->category() < g2.sym->category();
                             })->sym->category() + 1),
    best_{0, 0}
{
  index_t i(0);

  for (const auto &g : gv)
    genome_({i++, g.sym->category()}) = g;

  Ensures(debug());
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
  ret.best_ = l;
  ret.signature_.clear();

  Ensures(ret.debug());
  return ret;
}

///
/// A new individual is created mutating `this`.
///
/// \param[in] p probability of gene mutation
/// \param[in] e the current environment
/// \return      number of mutations performed
///
unsigned i_mep::mutation(double p, const environment &e)
{
  Expects(0.0 <= p && p <= 1.0);

  unsigned n(0);

  const auto i_size(size());
  const auto patch(i_size - e.patch_length);

  for (auto i(begin()); i != end(); ++i)  // Here mutation affects only exons
    if (random::boolean(p))
    {
      const auto ix(i.locus().index);
      const auto ct(i.locus().category);

      const gene g(ix < patch ? gene(e.sset->roulette(ct), ix + 1, i_size)
                              : gene(e.sset->roulette_terminal(ct)));

      if (*i != g)
      {
        ++n;
        *i = g;
      }
    }

  if (n)
    signature_.clear();

  Ensures(debug());
  return n;
}

///
/// Calculates a set of indexes to blocks contained in `this` individual.
///
/// \return a list of loci referring to active symbols
///
/// Indexes can be used by the i_mep::get_block function.
///
/// \note
/// A block is a subset of the active code composed of, at least, a function.
///
std::vector<locus> i_mep::blocks() const
{
  std::vector<locus> bl;

  for (auto i(begin()); i != end(); ++i)
    if (i->sym->arity())
      bl.push_back(i.locus());

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

  Ensures(ret.debug());
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
    ret.genome_({index, c}) = gene(sset.roulette_terminal(c));

  ret.signature_.clear();

  Ensures(ret.debug());
  return ret;
}

///
/// Produces the body for a ADF.
///
/// \param[in] sset     a symbol set
/// \param[in] max_args maximum number of arguments for the ADF
/// \return             the generalized individual and a set of loci (ADF
///                     arguments positions)
///
/// Changes up to `max_args` terminals (exactly `max_args` when available)
/// of `this` individual with formal arguments, thus producing the body for a
/// ADF.
///
std::pair<i_mep, std::vector<locus>> i_mep::generalize(
  unsigned max_args, const symbol_set &sset) const
{
  Expects(max_args && max_args <= gene::k_args);

  std::vector<locus> terminals;

  // Step 1: mark the active terminal symbols.
  for (auto i(begin()); i != end(); ++i)
    if (i->sym->terminal())
      terminals.push_back(i.locus());

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
    ret.genome_(terminals[j]).sym = &sset.arg(j);
  ret.signature_.clear();

  Ensures(ret.debug());

  return {ret, std::vector<locus>(terminals.begin(), terminals.begin() + n)};
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

  Ensures(!eq ||
          signature_.empty() != x.signature_.empty() ||
          signature_ == x.signature_);

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
/// \param[in] l  locus in this individual
/// \param[out] p byte stream compacted version of the gene sequence
///               starting at locus `l`
///
/// Useful for individual comparison / information retrieval.
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
/// Converts this individual in a packed byte level representation and
/// performs the MurmurHash3 algorithm on it.
///
/// \return the signature of this individual
///
hash_t i_mep::hash() const
{
  Expects(size());
  // if (empty())
  //   return hash_t();

  // From an individual to a packed byte stream...
  thread_local std::vector<unsigned char> packed;

  packed.clear();
  pack(best(), &packed);

  /// ... and from a packed byte stream to a signature...
  const auto len(static_cast<unsigned>(packed.size() *
                                       sizeof(packed[0])));  // Length in bytes

  return vita::hash(packed.data(), len, 1973);
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
bool i_mep::debug() const
{
  if (empty())
  {
    if (!genome_.empty())
    {
      print.debug("Inconsistent internal status for empty individual");
      return false;
    }

    if (best() != locus::npos())
    {
      print.error("Empty individual must have undefined best locus");
      return false;
    }

    if (!signature_.empty())
    {
      print.error("Empty individual must empty signature");
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
        print.error("Empty symbol pointer at locus ", l);
        return false;
      }

      // Correspondence between arity of the symbol and numbers of parameters.
      const auto arity(genome_(l).sym->arity());
      if (genome_(l).args.size() != arity)
      {
        print.error("Arity and actual number of parameters don't match");
        return false;
      }

      // Checking arguments' addresses.
      for (const auto &arg : genome_(l).args)
      {
        // Arguments' addresses must be smaller than the size of the genome.
        if (arg >= size())
        {
          print.error("Argument is out of range");
          return false;
        }

        // Function address must be smaller than its arguments' addresses.
        if (arg <= i)
        {
          print.error("Wrong reference in locus ", l);
          return false;
        }
      }
    }

  for (category_t c(0); c < categories(); ++c)
    if (!genome_(genome_.rows() - 1, c).sym->terminal())
    {
      print.error("Last symbol of type ", c, " in the genome isn't a terminal");
      return false;
    }

  // Type checking.
  for (index_t i(0); i < size(); ++i)
    for (category_t c(0); c < categories(); ++c)
    {
      const locus l{i, c};

      if (genome_(l).sym->category() != c)
      {
        print.error("Wrong category: ", l,
                    genome_(l).sym->display(), " -> ",
                    genome_(l).sym->category(), " should be ", c);
        return false;
      }
    }

  if (best().index >= size())
  {
    print.error("Incorrect index for first active symbol");
    return false;
  }
  if (best().category >= categories())
  {
    print.error("Incorrect category for first active symbol");
    return false;
  }

  if (categories() == 1 && active_symbols() > size())
  {
    print.error("active_symbols() cannot be greater than size() in single "
                "category individuals");
    return false;
  }

  return signature_.empty() || signature_ == hash();
}

///
/// The output stream contains a graph of this individual described in dot
/// language.
///
/// \param[in]  mep individual to be exported
/// \param[out] s   output stream
/// \param[in]  id  used for subgraph plot (usually this is an empty string)
///
/// \see http://www.graphviz.org/
///
void graphviz(const i_mep &mep, std::ostream &s, const std::string &id)
{
  if (id.empty())
    s << "graph";
  else
    s << "subgraph " << id;
  s << " {";

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

///
/// The individual is printed on a single line with symbols separated by
/// spaces.
///
/// \param[in]  mep individual to be printed
/// \param[out] s   output stream
/// \return         a reference to the output stream
///
/// Not at all human readable, but a compact representation for
/// import / export.
///
/// \note
/// Prints active genes visiting the genome in pre-order.
///
/// \relates i_mep
///
std::ostream &in_line(const i_mep &mep, std::ostream &s)
{
  std::function<void (locus)> in_line_(
    [&](locus l)
    {
      const gene &g(mep[l]);

      if (l != mep.best())
        s << ' ';
      s << g;

      const auto arity(g.sym->arity());
      for (auto i(decltype(arity){0}); i < arity; ++i)
        in_line_(g.arg_locus(i));
    });

  in_line_(mep.best());
  return s;
}

///
/// Prints a human readable representation of the individual.
///
/// \param[in]  mep        the individual to be printed
/// \param[out] s          output stream
/// \param[in]  short_form if `true` prints a shorter and more human-readable
///                        form of the genome
/// \return                a reference to the output stream
///
/// Do you remember C=64's `LIST`? :-)
///
///     10 PRINT "HOME"
///     20 PRINT "SWEET"
///     30 GOTO 10
///
/// \relates i_mep
///
std::ostream &list(const i_mep &mep, std::ostream &s, bool short_form)
{
  SAVE_FLAGS(s);

  const auto size(mep.size());
  const auto categories(mep.categories());

  const auto w1(1 + static_cast<int>(std::log10(size - 1)));
  const auto w2(1 + static_cast<int>(std::log10(categories)));

  for (auto i(mep.begin()); i != mep.end(); ++i)
  {
    if (short_form && i->sym->terminal() && i.locus() != mep.best())
      continue;

    s << '[' << std::setfill('0') << std::setw(w1) << i.locus().index;

    if (categories > 1)
      s << ',' << std::setw(w2) << i.locus().category;

    s << "] " << *i;

    const auto arity(i->sym->arity());
    for (unsigned j(0); j < arity; ++j)
    {
      s << ' ';

      const auto arg_j(i->arg_locus(j));

      if (short_form && mep[arg_j].sym->terminal())
        s << mep[arg_j];
      else
      {
        s << '[' << std::setw(w1) << arg_j.index;
        if (categories > 1)
          s << ',' << std::setw(w2) << arg_j.category;
        s << ']';
      }
    }

    s << '\n';
  }

  return s;
}

///
/// \param[in]  mep the individual to be printed
/// \param[out] s   output stream
/// \return         a reference to the (modified) output stream
///
std::ostream &tree(const i_mep &mep, std::ostream &s)
{
  std::function<void (const gene &, const gene &, unsigned)> tree_(
    [&](const gene &parent, const gene &child, unsigned indent)
    {
      if (child == parent ||
          parent.sym != child.sym ||
          function::cast(parent.sym)->associative() == false)
      {
        s << std::string(indent, ' ') << child << '\n';
        indent += 2;
      }

      const auto arity(child.sym->arity());
      for (auto i(decltype(arity){0}); i < arity; ++i)
        tree_(child, mep[child.arg_locus(i)], indent);
    });

  tree_(mep[mep.best()], mep[mep.best()], 0);
  return s;
}

///
/// Prints the complete content of an individual.
///
/// \param[in]  mep individual to be printed
/// \param[out] s   output stream
/// \return         a reference to `s`
///
/// \relates i_mep
///
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

      const auto arity(g.sym->arity());
      for (auto j(decltype(arity){0}); j < arity; ++j)
      {
        const auto arg_j(g.arg_locus(j));

        s << " [" << std::setw(w1) << arg_j.index;
        if (categories > 1)
          s << ',' << std::setw(w2) << arg_j.category;
        s << ']';
      }

      s << '\n';
    }

  return s;
}

///
/// \param[out] s   output stream
/// \param[in]  ind individual to print
/// \return         output stream including `ind`
///
/// \relates i_mep
///
std::ostream &operator<<(std::ostream &s, const i_mep &ind)
{
  return list(ind, s, true);
}

///
/// \param[in] e  environment used to build the individual
/// \param[in] in input stream
/// \return       `true` if the object has been loaded correctly
///
/// \note
/// If the load operation isn't successful the current individual isn't
/// modified.
///
bool i_mep::load_impl(std::istream &in, const environment &e)
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
/// \param[out] out output stream
/// \return         `true` if the object has been saved correctly
///
bool i_mep::save_impl(std::ostream &out) const
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
    out << best().index << ' ' << best().category << '\n';

  return out.good();
}

///
/// A sort of "common subexpression elimination" optimization.
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

  using ARRAY_ELEM_TYPE = std::remove_reference_t<decltype(gene::args[0])>;

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
/// Uniform Crossover.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring).
///
/// The i-th locus of the offspring has a 50% probability to be filled with
/// the i-th gene of `lhs` and 50% with i-th gene of `rhs`. Parents must
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
i_mep crossover(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.debug());
  Expects(rhs.debug());
  Expects(lhs.size() == rhs.size());

  i_mep ret(lhs);

  for (auto i(rhs.begin()); i != rhs.end(); ++i)
    if (random::boolean())
      ret.genome_(i.locus()) = *i;

  ret.set_older_age(rhs.age());
  ret.signature_.clear();

  Ensures(ret.debug(true));
  return ret;
}
#elif defined(ONE_POINT_CROSSOVER)
///
/// One Point Crossover.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring)
///
/// We randomly select a parent (between `lhs` and `rhs`) and a single locus
/// (common crossover point). The offspring is created with genes from the
/// chosen parent up to the crossover point and genes from the other parent
/// beyond that point.
/// One-point crossover is the oldest homologous crossover in tree-based GP.
///
/// \note Parents must have the same size.
///
i_mep crossover(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.debug());
  Expects(rhs.debug());
  Expects(lhs.size() == rhs.size());

  const auto i_sup(lhs.size());
  const auto c_sup(lhs.categories());

  const auto cut(random::between<index_t>(1, i_sup - 1));

  const bool b(random::boolean());
  const i_mep *parents[] = {&lhs, &rhs};
  i_mep ret(*parents[b]);

  for (index_t i(cut); i < i_sup; ++i)
    for (category_t c(0); c < c_sup; ++c)
    {
      const locus l{i, c};
      ret.genome_(l) = parents[!b]->genome_(l);
    }

  ret.set_older_age(parents[!b]->age());
  ret.signature_.clear();

  Ensures(ret.debug());
  return ret;
}
#elif defined(TREE_CROSSOVER)
///
/// Tree Crossover.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring)
///
/// Inserts a complete tree from one parent into the other.
/// The operation is less disruptive than other forms of crossover since
/// an entire tree is copied (not just a part).
///
/// \note Parents must have the same size.
///
i_mep crossover(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.debug());
  Expects(rhs.debug());
  Expects(lhs.size() == rhs.size());

  const bool b(random::boolean());
  const i_mep *parents[] = {&lhs, &rhs};
  i_mep ret(*parents[b]);

  const auto delta(random::between(0u, lhs.active_symbols()));

  for (auto i(std::next(parents[!b]->begin(), delta));
       i != parents[!b]->end();
       ++i)
    ret.genome_(i.locus()) = *i;

  ret.set_older_age(parents[!b]->age());
  ret.signature_.clear();

  Ensures(ret.debug());
  return ret;
}
#else  // TWO_POINT_CROSSOVER (default)
///
/// Two Point Crossover.
///
/// \param[in] lhs first parent
/// \param[in] rhs second parent
/// \return        the result of the crossover (we only generate a single
///                offspring)
///
/// We randomly select a parent (between `lhs` and `rhs`) and a two loci
/// (common crossover points). The offspring is created with genes from the
/// chosen parent before the first crossover point and after the second
/// crossover point; genes between crossover points are taken from the other
/// parent.
///
/// \note Parents must have the same size.
///
i_mep crossover(const i_mep &lhs, const i_mep &rhs)
{
  Expects(lhs.debug());
  Expects(rhs.debug());
  Expects(lhs.size() == rhs.size());

  const auto i_sup(lhs.size());
  const auto c_sup(lhs.categories());

  const auto cut1(random::sup(i_sup - 1));
  const auto cut2(random::between(cut1 + 1, i_sup));

  const bool b(random::boolean());
  const i_mep *parents[] = {&lhs, &rhs};
  i_mep ret(*parents[b]);

  for (index_t i(cut1); i != cut2; ++i)
    for (category_t c(0); c < c_sup; ++c)
    {
      const locus l{i, c};
      ret.genome_(l) = parents[!b]->genome_(l);
    }

  ret.set_older_age(parents[!b]->age());
  ret.signature_.clear();

  Ensures(ret.debug());
  return ret;
}
#endif

}  // namespace vita
