/**
 *
 *  \file individual.cc
 *
 *  Copyright (c) 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <algorithm>

#include "kernel/individual.h"
#include "kernel/adf.h"
#include "kernel/argument.h"
#include "kernel/environment.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// \param[in] e base environment.
  /// \param[in] gen if \c true generates a random sequence of genes to
  ///                initialize the individual.
  ///
  /// The process that generates the initial, random expressions have to be
  /// implemented so as to ensure that they do not violate the type system's
  /// constraints.
  ///
  individual::individual(const environment &e, bool gen)
    : crossover_(uniform_crossover), best_(locus{{0, 0}}), env_(&e),
      genome_(boost::extents[*e.code_length][e.sset.categories()]),
      signature_()
  {
    assert(e.check(true, true));

    if (gen)  // random generate initial code
    {
      assert(2 <= size());
      const index_t sup(size() - 1);

      const category_t categories(e.sset.categories());
      assert(categories);
      assert(categories < sup);

      // STANDARD SECTION. Filling the genome with random symbols.
      for (index_t i(0); i < sup; ++i)
        for (category_t c(0); c < categories; ++c)
          genome_[i][c] = gene(e.sset.roulette(c), i+1, size());

      // PATCH SUBSECTION. Placing terminals for satisfying constraints on
      // types.
      for (category_t c(0); c < categories; ++c)
        genome_[sup][c] = gene(e.sset.roulette_terminal(c));

      assert(check(true));
    }
  }

  ///
  /// \return the effective size of the individual.
  /// \see size
  ///
  unsigned individual::eff_size() const
  {
    unsigned ef(0);

    for (const_iterator it(*this); it(); ++it)
      ++ef;

    return ef;
  }

  ///
  /// \param[in] l locus of the genome.
  /// \return an individual obtained from \c this choosing the gene
  ///         sequence starting at \a l.
  ///
  /// This function is often used along with the \ref blocks function.
  ///
  individual individual::get_block(const locus &l) const
  {
    individual ret(*this);
    ret.best_ = l;
    ret.signature_.clear();

    assert(ret.check());
    return ret;
  }

  ///
  /// \param[in] p probability of gene mutation.
  /// \return number of mutations performed.
  ///
  /// A new individual is created mutating \c this.
  ///
  unsigned individual::mutation(double p)
  {
    unsigned n(0);

    const unsigned sup(size() - 1);
    const unsigned categories(env_->sset.categories());

    for (index_t i(0); i < sup; ++i)
      for (category_t c(0); c < categories; ++c)
        if (random::boolean(p))
        {
          ++n;

          set(locus{{i, c}}, gene(env_->sset.roulette(c), i+1, size()));
        }

    for (category_t c(0); c < categories; ++c)
      if (random::boolean(p))
      {
        ++n;

        set(locus{{sup, c}}, gene(env_->sset.roulette_terminal(c)));
      }

    assert(check());

    return n;
  }

  ///
  /// \param[in] parent2 the second parent (being \c this the first).
  /// \return a reference to \c this.
  ///
  /// There are some predefined crossover operators: we use the \a cross_
  /// function wrapper to choose which one to use.
  ///
  individual individual::crossover(const individual &parent2) const
  {
    assert(check());
    assert(parent2.check());

    return crossover_(*this, parent2);
  }

  ///
  /// \param[in] cw a function pointer / functor / anonymous (lambda) function
  ///               used for crossover operation.
  ///
  /// Crossover implementation can be changed / selected at runtime by this
  /// method.
  /// Please note that this is one of the very few individual methods that
  /// aren't const.
  ///
  void individual::set_crossover(const crossover_wrapper &cw)
  {
    crossover_ = cw;
  }

  ///
  /// \return a list of loci referring to active symbols.
  ///
  /// The function extract from the individual a list of indexes to blocks
  /// that are subsets of the active code. Indexes can be used by the
  /// individual::get_block function.
  ///
  std::list<locus> individual::blocks() const
  {
    std::list<locus> bl;

    locus l(best_);
    for (const_iterator i(*this); i(); l = ++i)
      if (genome_(l).sym->arity())
        bl.push_back(l);

    return bl;
  }

  ///
  /// \param[in] sym symbol used for replacement.
  /// \param[in] args new arguments.
  /// \param[in] l locus where replacement take place.
  /// \return a new \a individual with a gene replaced.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// \a symbol at locus \a l with a new one ('sym' + 'args').
  ///
  individual individual::replace(const symbol_ptr &sym,
                                 const std::vector<unsigned> &args,
                                 const locus &l) const
  {
    assert(sym);

    individual ret(*this);

    ret.genome_(l).sym = sym;
    for (unsigned i(0); i < args.size(); ++i)
      ret.genome_(l).args[i] = args[i];

    ret.signature_.clear();

    assert(ret.check());
    return ret;
  }

  ///
  /// \param[in] sym symbol used for replacement.
  /// \param[in] args new arguments.
  /// \return a new individual with gene at locus \a best_ replaced.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// \a symbol at locus \a best_ with a new one ('sym' + 'args').
  ///
  individual individual::replace(const symbol_ptr &sym,
                                 const std::vector<unsigned> &args) const
  {
    return replace(sym, args, best_);
  }

  ///
  /// \param[in] line index of a \a symbol in the \a individual.
  /// \return a new individual obtained from \c this inserting a random
  ///         \a terminal at index \a line.
  ///
  individual individual::destroy_block(unsigned index) const
  {
    assert(index < size());

    individual ret(*this);
    const unsigned categories(env_->sset.categories());
    for (category_t c(0); c < categories; ++c)
      ret.set(locus{{index, c}}, gene(env_->sset.roulette_terminal(c)));

    assert(ret.check());
    return ret;
  }

  ///
  /// \param[in] max_args maximum number of arguments for the ADF.
  /// \param[out] loci the ADF arguments are here.
  /// \return the generalized individual.
  ///
  /// Changes up to \a max_args terminals (exactly \a max_args when available)
  /// of \c this individual with formal arguments, thus producing the body
  /// for a ADF.
  ///
  individual individual::generalize(unsigned max_args,
                                    std::vector<locus> *const loci) const
  {
    assert(max_args && max_args <= gene::k_args);

    std::vector<locus> terminals;

    // Step 1: mark the active terminal symbols.
    locus l(best_);
    for (const_iterator i(*this); i(); l = ++i)
      if (genome_(l).sym->terminal())
        terminals.push_back(l);

    // Step 2: shuffle the terminals and pick elements 0..n-1.
    const unsigned n(std::min(max_args,
                              static_cast<unsigned>(terminals.size())));
    assert(n);

    if (n < size())
      for (unsigned j(0); j < n; ++j)
      {
        const unsigned r(random::between<unsigned>(j, terminals.size()));

        const locus tmp(terminals[j]);
        terminals[j] = terminals[r];
        terminals[r] = tmp;
      }

    // Step 3: randomly substitute n terminals with function arguments.
    individual ret(*this);
    for (unsigned j(0); j < n; ++j)
    {
      gene &g(ret.genome_(terminals[j]));
      if (loci)
        loci->push_back(terminals[j]);
      g.sym = env_->sset.arg(j);
      ret.signature_.clear();
    }

    assert(!loci || (loci->size() && loci->size() <= max_args));
    assert(ret.check());

    return ret;
  }

  ///
  /// \return the category of the individual.
  ///
  category_t individual::category() const
  {
    return best_[1];
  }

  ///
  /// \param[in] x second term of comparison.
  /// \return \c true if the two individuals are equal (symbol by symbol,
  ///         including introns).
  ///
  bool individual::operator==(const individual &x) const
  {
    return genome_ == x.genome_ && best_ == x.best_;
  }

  ///
  /// \param[in] ind an individual to compare with \c this.
  /// \return a numeric measurement of the difference between \a ind and
  /// \c this (the number of different genes between individuals).
  ///
  unsigned individual::distance(const individual &ind) const
  {
    const unsigned cs(size());
    const unsigned categories(env_->sset.categories());

    unsigned d(0);
    for (index_t i(0); i < cs; ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i, c}};
        if (genome_(l) != ind.genome_(l))
          ++d;
      }

    return d;
  }

  ///
  /// \param[in] l locus in \c this individual.
  /// \param[out] p byte stream compacted version of the gene sequence
  ///               starting at locus \a l.
  ///
  /// This function map syntactically distinct (but logically equivalent)
  /// individuals to the same byte stream. This is a very interesting
  /// property, useful for individual comparison / information retrieval.
  ///
  void individual::pack(const locus &l,
                        std::vector<boost::uint8_t> *const p) const
  {
    const gene &g(genome_(l));

    const opcode_t opcode(g.sym->opcode());

    const boost::uint8_t *const s1 = (boost::uint8_t *)(&opcode);
    for (unsigned i(0); i < sizeof(opcode); ++i)
      p->push_back(s1[i]);

    if (g.sym->parametric())
    {
      const boost::uint8_t *const s2 = (boost::uint8_t *)(&g.par);
      for (unsigned i(0); i < sizeof(g.par); ++i)
        p->push_back(s2[i]);
    }
    else
      for (unsigned i(0); i < g.sym->arity(); ++i)
        pack(locus{{g.args[i], function::cast(g.sym)->arg_category(i)}},
             p);
  }

  ///
  /// \return the signature of \c this individual.
  ///
  /// Converts \c this individual in a packed byte level representation and
  /// performs the MurmurHash3 algorithm on it.
  /// MurmurHash3 (http://code.google.com/p/smhasher/), by Austin Appleby, is a
  /// relatively simple non-cryptographic hash algorithm. It is noted for being
  /// fast, with excellent distribution, avalanche behavior and overall
  /// collision resistance.
  ///
  hash_t individual::hash() const
  {
    // From an individual to a packed byte stream...
    std::vector<boost::uint8_t> packed;
    pack(best_, &packed);

    /// ... and from a packed byte stram to a signature...
    /// Murmurhash3 follows.
    const unsigned len(packed.size());
    const unsigned n_blocks(len / 16);

    const boost::uint64_t seed(1973);
    boost::uint64_t h1(seed);
    boost::uint64_t h2(seed);

    boost::uint64_t c1(0x87c37b91114253d5);
    boost::uint64_t c2(0x4cf5ad432745937f);

    const boost::uint64_t *const blocks(
      reinterpret_cast<const boost::uint64_t *>(&packed[0]));

    for (unsigned i(0); i < n_blocks; ++i)
    {
      boost::uint64_t k1(blocks[i*2 + 0]);
      boost::uint64_t k2(blocks[i*2 + 1]);

      k1 *= c1;
      k1  = ROTL64(k1, 31);
      k1 *= c2;
      h1 ^= k1;

      h1 = ROTL64(h1, 27);
      h1 += h2;
      h1 = h1*5 + 0x52dce729;

      k2 *= c2;
      k2  = ROTL64(k2, 33);
      k2 *= c1;
      h2 ^= k2;

      h2 = ROTL64(h2, 31);
      h2 += h1;
      h2 = h2*5 + 0x38495ab5;
    }

    const boost::uint8_t *const tail(
      static_cast<const boost::uint8_t *>(&packed[0] + n_blocks*16));

    boost::uint64_t k1(0);
    boost::uint64_t k2(0);

    switch (len & 15)
    {
    case 15: k2 ^= boost::uint64_t(tail[14]) << 48;
    case 14: k2 ^= boost::uint64_t(tail[13]) << 40;
    case 13: k2 ^= boost::uint64_t(tail[12]) << 32;

    case 12: k2 ^= boost::uint64_t(tail[11]) << 24;
    case 11: k2 ^= boost::uint64_t(tail[10]) << 16;
    case 10: k2 ^= boost::uint64_t(tail[ 9]) << 8;
    case  9:
      k2 ^= boost::uint64_t(tail[ 8]) << 0;
      k2 *= c2;
      k2  = ROTL64(k2, 33);
      k2 *= c1;
      h2 ^= k2;

    case  8: k1 ^= boost::uint64_t(tail[ 7]) << 56;
    case  7: k1 ^= boost::uint64_t(tail[ 6]) << 48;
    case  6: k1 ^= boost::uint64_t(tail[ 5]) << 40;
    case  5: k1 ^= boost::uint64_t(tail[ 4]) << 32;
    case  4: k1 ^= boost::uint64_t(tail[ 3]) << 24;
    case  3: k1 ^= boost::uint64_t(tail[ 2]) << 16;
    case  2: k1 ^= boost::uint64_t(tail[ 1]) << 8;
    case  1:
      k1 ^= boost::uint64_t(tail[ 0]) << 0;
      k1 *= c1;
      k1  = ROTL64(k1, 31);
      k1 *= c2;
      h1 ^= k1;
    }

    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    h2 ^= h2 >> 16;
    h2 *= 0x85ebca6b;
    h2 ^= h2 >> 13;
    h2 *= 0xc2b2ae35;
    h2 ^= h2 >> 16;

    h1 += h2;
    h2 += h1;

    return hash_t(h1, h2);
  }

  ///
  /// \return the signature of \c this individual.
  ///
  /// Signature map syntactically distinct (but logically equivalent)
  /// individuals to the same value. This is a very interesting  property,
  /// useful for individual comparison, information retrieval, entropy
  /// calculation...
  ///
  hash_t individual::signature() const
  {
    if (signature_.empty())
      signature_ = hash();

    return signature_;
  }

  ///
  /// \param[in] verbose if \c true prints error messages to \c std::cerr.
  /// \return \c true if the individual passes the internal consistency check.
  ///
  bool individual::check(bool verbose) const
  {
    const unsigned categories(env_->sset.categories());

    for (unsigned i(0); i < size(); ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i, c}};

        if (!genome_(l).sym)
        {
          if (verbose)
            std::cerr << "Empty symbol pointer at locus " << l << '.'
                      << std::endl;
          return false;
        }

        // Maximum number of function arguments is gene::k_args.
        if (genome_(l).sym->arity() > gene::k_args)
        {
          if (verbose)
            std::cerr << "Function arity exceeds maximum size." << std::endl;
          return false;
        }

        // Checking arguments' addresses.
        for (unsigned j(0); j < genome_(l).sym->arity(); ++j)
        {
          // Arguments' addresses must be smaller than the size of the genome.
          if (genome_(l).args[j] >= size())
          {
            if (verbose)
              std::cerr << "Argument is out of range." << std::endl;
            return false;
          }

          // Function address must be smaller than its arguments' addresses.
          if (genome_(l).args[j] <= i)
          {
            if (verbose)
              std::cerr << "Self reference in locus " << l << '.' << std::endl;
            return false;
          }
        }
      }

    for (category_t c(0); c < categories; ++c)
      if (!genome_[genome_.size() - 1][c].sym->terminal())
      {
        if (verbose)
          std::cerr << "Last symbol of type " << c
                    << " in the genome isn't a terminal." << std::endl;
        return false;
      }

    // Type checking.
    for (unsigned i(0); i < size(); ++i)
      for (unsigned c(0); c < categories; ++c)
      {
        const locus l{{i, c}};

        if (genome_(l).sym->category() != c)
        {
          if (verbose)
            std::cerr << "Wrong category: " << l << genome_(l).sym->display()
                      << " -> " << genome_(l).sym->category()
                      << " should be " << c << std::endl;
          return false;
        }
      }

    if (best_[0] >= size())
    {
      if (verbose)
        std::cerr << "Incorrect index for first active symbol." << std::endl;
      return false;
    }
    if (best_[1] >= categories)
    {
      if (verbose)
        std::cerr << "Incorrect category for first active symbol." << std::endl;
      return false;
    }

    if (categories == 1 && eff_size() > size())
    {
      if (verbose)
        std::cerr << "eff_size() cannot be greater than size() in single "\
                     "category individuals." << std::endl;
      return false;
    }

    return
      env_->check(verbose, true) &&
      (signature_.empty() || signature_ == hash());
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] id used for subgraph plot (usually this is an empty string).
  ///
  /// The output stream contains a graph, described in dot language
  /// (http://www.graphviz.org), of \c this individual.
  ///
  void individual::graphviz(std::ostream &s, const std::string &id) const
  {
    if (id.empty())
      s << "graph";
    else
      s << "subgraph " << id;
    s << " {";

    locus l(best_);
    for (const_iterator it(*this); it(); l = ++it)
    {
      const gene &g(*it);

      s << 'g' << l[0] << '_' << l[1] << " [label="
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
        << ", shape=" << (g.sym->arity() ? "box" : "parallelogram") << "];";

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << 'g' << l[0] << '_' << l[1] << " -- g"
          << g.args[j] << '_' << function::cast(g.sym)->arg_category(j) << ';';
    }

    s << '}' << std::endl;
  }

  ///
  /// \param[out] s output stream
  ///
  /// The \a individual is printed on a single line with symbols separated by
  /// spaces. Not at all human readable, but a compact representation for
  /// import / export.
  ///
  void individual::in_line(std::ostream &s) const
  {
    locus l(best_);
    for (const_iterator it(*this); it(); l = ++it)
    {
      const gene &g(*it);

      if (l != best_)
        s << ' ';
      s << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());
    }
  }

  ///
  /// \param[out] s output stream
  ///
  /// Do you remember C=64 list? :-)
  ///
  /// 10 PRINT "HOME"
  /// 20 PRINT "SWEET"
  /// 30 GOTO 10
  ///
  void individual::list(std::ostream &s) const
  {
    const unsigned categories(env_->sset.categories());
    const unsigned w1(
      1 + static_cast<unsigned>(std::log10(static_cast<double>(size() - 1))));
    const unsigned w2(
      1 + static_cast<unsigned>(std::log10(static_cast<double>(categories))));

    locus l(best_);
    for (const_iterator it(*this); it(); l = ++it)
    {
      const gene &g(*it);

      s << '[' << std::setfill('0') << std::setw(w1) << l[0];

      if (categories > 1)
        s << ", " << std::setw(w2) << l[1];

      s << "] "
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << ' ' << std::setw(w1) << g.args[j];

      s << std::endl;
    }
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] child child node (this is the node we are "printing").
  /// \param[in] indent indentation level.
  /// \param[in] parent parent of \a child.
  ///
  void individual::tree(std::ostream &s, const locus &child, unsigned indent,
                        const locus &parent) const
  {
    const gene &g(genome_(child));

    if (child == parent
        || !genome_(parent).sym->associative()
        || genome_(parent).sym != g.sym)
    {
      std::string spaces(indent, ' ');
      s << spaces
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
        << std::endl;
      indent += 2;
    }

    const unsigned arity(g.sym->arity());
    if (arity)
      for (unsigned i(0); i < arity; ++i)
        tree(s, locus{{g.args[i], function::cast(g.sym)->arg_category(i)}},
             indent, child);
  }

  ///
  /// \param[out] s output stream.
  ///
  void individual::tree(std::ostream &s) const
  {
    tree(s, best_, 0, best_);
  }

  ///
  /// \param[out] s output stream.
  ///
  /// Print the complete content of this individual.
  ///
  void individual::dump(std::ostream &s) const
  {
    const unsigned categories(env_->sset.categories());
    const unsigned width(1 + std::log10(size()-1));

    for (unsigned i(0); i < size(); ++i)
    {
      s << '[' << std::setfill('0') << std::setw(width) << i << "] ";

      for (category_t c(0); c < categories; ++c)
      {
        const gene &g(genome_[i][c]);

        if (categories > 1)
          s << '{';

        s << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

        for (unsigned j(0); j < g.sym->arity(); ++j)
          s << ' ' << std::setw(width) << g.args[j];

        if (categories > 1)
          s << '}';
      }

      s << std::endl;
    }
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] ind individual to print.
  /// \return output stream including \a ind.
  ///
  std::ostream &operator<<(std::ostream &s, const individual &ind)
  {
    ind.list(s);

    return s;
  }

  ///
  /// \param[in] id
  ///
  individual::const_iterator::const_iterator(const individual &id)
    : ind_(id), l_(id.best_)
  {
    loci_.insert(l_);
  }

  ///
  /// \return locus of the next line containing an active symbol.
  ///
  const locus &individual::const_iterator::operator++()
  {
    if (!loci_.empty())
    {
      loci_.erase(loci_.begin());

      const gene &g(ind_.genome_(l_));

      for (unsigned j(0); j < g.sym->arity(); ++j)
      {
        const locus l{{g.args[j], function::cast(g.sym)->arg_category(j)}};

        loci_.insert(l);
      }

      l_ = *loci_.begin();
    }

    return l_;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover.
  ///
  /// The i-th locus of the offspring has a 50% probability to be filled with
  /// the i-th gene of \a p1 and 50% with i-th gene of \a p2. Parents must have
  /// the same size.
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
  individual uniform_crossover(const individual &p1, const individual &p2)
  {
    assert(p1.check());
    assert(p2.check());

    const unsigned cs(p1.size());
    const unsigned categories(p1.env().sset.categories());

    assert(cs == p2.size());
    assert(categories == p2.env().sset.categories());

    individual offspring(p1);

    for (unsigned i(0); i < cs; ++i)
      for (category_t c(0); c < categories; ++c)
        if (random::boolean())
        {
          const locus l{{i,c}};
          offspring.set(l, p2[l]);
        }

    assert(offspring.check(true));
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover.
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a single locus
  /// (common crossover point). The offspring is created with genes from the
  /// choosen parent up to the crossover point and genes from the other parent
  /// beyond that point. Parents must have the same size.
  /// One-point crossover is the oldest homologous crossover in tree-based GP.
  ///
  individual one_point_crossover(const individual &p1, const individual &p2)
  {
    assert(p1.check());
    assert(p2.check());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());
    const unsigned categories(p1.env().sset.categories());

    const unsigned cut(random::between<unsigned>(0, cs-1));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (unsigned i(cut); i < cs; ++i)
      for (unsigned c(0); c < categories; ++c)
      {
        const locus l{{i,c}};
        offspring.set(l, (*parents[!base])[l]);
      }

    assert(offspring.check());
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover.
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a two loci
  /// (common crossover points). The offspring is created with genes from the
  /// choosen parent before the first crossover point and after the second
  /// crossover point; genes between crossover points are taken from the other
  /// parent.
  /// Parents must have the same size.
  ///
  individual two_point_crossover(const individual &p1, const individual &p2)
  {
    assert(p1.check());
    assert(p2.check());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());
    const unsigned categories(p1.env().sset.categories());

    const unsigned cut1(random::between<unsigned>(0, cs-1));
    const unsigned cut2(random::between<unsigned>(cut1+1, cs));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (unsigned i(cut1); i < cut2; ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i, c}};
        offspring.set(l, (*parents[!base])[l]);
      }

    assert(offspring.check());
    return offspring;
  }
}  // Namespace vita
