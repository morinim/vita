/**
 *
 *  \file individual.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011-2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <algorithm>

#include "individual.h"
#include "adf.h"
#include "argument.h"
#include "environment.h"
#include "random.h"
#include "ttable_hash.h"

namespace vita
{
  std::function<individual (const individual &, const individual &)>
  individual::crossover(uniform_crossover);

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
    : genome_(*e.code_length, e.sset.categories()),
      signature_(), best_({{0, 0}}), env_(&e)
  {
    assert(e.debug(true, true));

    if (gen)  // random generate initial code
    {
      assert(size());
      const index_t sup(size() - 1);

      const category_t categories(e.sset.categories());
      assert(categories);
      assert(categories < sup);

      // STANDARD SECTION. Filling the genome with random symbols.
      for (index_t i(0); i < sup; ++i)
        for (category_t c(0); c < categories; ++c)
          genome_(i, c) = gene(e.sset.roulette(c), i + 1, size());

      // PATCH SUBSECTION. Placing terminals for satisfying constraints on
      // types.
      for (category_t c(0); c < categories; ++c)
        genome_(sup, c) = gene(e.sset.roulette_terminal(c));

      assert(debug(true));
    }
  }

  ///
  /// \return the effective size of the individual.
  /// \see size
  ///
  size_t individual::eff_size() const
  {
    size_t ef(0);

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

    assert(ret.debug());
    return ret;
  }

  /*
  void individual::hoist()
  {
    const unsigned sup(size() - 1);
    const unsigned categories(env_->sset.categories());

    for (index_t i(0); i < sup; ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i, c}};

        set(l, genome_({{i+1, c}}));

        for (unsigned j(0); j < genome_(l).sym->arity(); ++j)
        {
          --genome_(l).args[j];

          assert(genome_(l).args[j] > i);
        }
      }

    for (category_t c(0); c < categories; ++c)
      set({{sup, c}}, gene(env_->sset.roulette_terminal(c)));
  }
  */

  ///
  /// \param[in] p probability of gene mutation.
  /// \return number of mutations performed.
  ///
  /// A new individual is created mutating \c this.
  ///
  unsigned individual::mutation(double p)
  {
    unsigned n(0);

    const index_t sup(size() - 1);

    // Here mutation affects only exons.
    for (const_iterator it(*this); it(); ++it)
      if (random::boolean(p))
      {
        ++n;

        const index_t i(it.l[locus_index]);
        const category_t c(it.l[locus_category]);

        if (i < sup)
          set(it.l, gene(env_->sset.roulette(c), i + 1, size()));
        else
          set(it.l, gene(env_->sset.roulette_terminal(c)));
      }

/*
    // MUTATION OF THE ENTIRE GENOME (EXONS + INTRONS).
    const category_t categories(env_->sset.categories());

    for (index_t i(0); i < sup; ++i)
      for (category_t c(0); c < categories; ++c)
        if (random::boolean(p))
        {
          ++n;

          set({{i, c}}, gene(env_->sset.roulette(c), i + 1, size()));
        }

    for (category_t c(0); c < categories; ++c)
      if (random::boolean(p))
      {
        ++n;

        set({{sup, c}}, gene(env_->sset.roulette_terminal(c)));
      }
*/
    assert(debug());

    return n;
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

    for (const_iterator i(*this); i(); ++i)
      if (genome_(i.l).sym->arity())
        bl.push_back(i.l);

    return bl;
  }

  ///
  /// \param[in] l locus where replacement takes place.
  /// \param[in] g new gene for replacement.
  /// \return a new individual with gene at locus \a l replaced by \a g.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// symbol at locus \a l with \a g.
  ///
  /// \note
  /// individual::replace method is similar to individual::set but the former
  /// creates a new individual while the latter modify \a this.
  ///
  individual individual::replace(const locus &l, const gene &g) const
  {
    assert(g.debug());

    individual ret(*this);

    ret.set(l, g);

    assert(ret.debug());
    return ret;
  }

  ///
  /// \param[in] g new gene for replacement.
  /// \return a new individual with gene at locus \a best_ replaced by \a g.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// symbol at locus \a best_ with \a g.
  ///
  individual individual::replace(const gene &g) const
  {
    return replace(best_, g);
  }

  ///
  /// \param[in] gv vector of genes.
  /// \return a new individual .
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// gene at index \a i with a new one (\a gv[i]).
  ///
  individual individual::replace(const std::vector<gene> &gv) const
  {
    individual ret(*this);

    index_t i(0);
    for (const auto &g : gv)
      ret.set({{i++, g.sym->category()}}, g);

    assert(ret);
    return ret;
  }


  ///
  /// \param[in] index index of a \a symbol in the \a individual.
  /// \return a new individual obtained from \c this inserting a random
  ///         \a terminal at index \a line.
  ///
  individual individual::destroy_block(index_t index) const
  {
    assert(index < size());

    individual ret(*this);
    const category_t categories(env_->sset.categories());
    for (category_t c(0); c < categories; ++c)
      ret.set({{index, c}}, gene(env_->sset.roulette_terminal(c)));

    assert(ret.debug());
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
  individual individual::generalize(size_t max_args,
                                    std::vector<locus> *const loci) const
  {
    assert(max_args && max_args <= gene::k_args);

    std::vector<locus> terminals;

    // Step 1: mark the active terminal symbols.
    for (const_iterator i(*this); i(); ++i)
      if (genome_(i.l).sym->terminal())
        terminals.push_back(i.l);

    // Step 2: shuffle the terminals and pick elements 0..n-1.
    const size_t n(std::min(max_args, terminals.size()));
    assert(n);

    if (n < size())
      for (size_t j(0); j < n; ++j)
      {
        const size_t r(random::between<size_t>(j, terminals.size()));

        std::swap(terminals[j], terminals[r]);
      }

    // Step 3: randomly substitute n terminals with function arguments.
    individual ret(*this);
    for (size_t j(0); j < n; ++j)
    {
      gene &g(ret.genome_(terminals[j]));
      if (loci)
        loci->push_back(terminals[j]);
      g.sym = env_->sset.arg(j);
      ret.signature_.clear();
    }

    assert(!loci || (loci->size() && loci->size() <= max_args));
    assert(ret.debug());

    return ret;
  }

  ///
  /// \return the category of the individual.
  ///
  category_t individual::category() const
  {
    return best_[locus_category];
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
  size_t individual::distance(const individual &ind) const
  {
    const size_t cs(size());
    const size_t categories(env_->sset.categories());

    size_t d(0);
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
  template <class T>
  void individual::pack(const locus &l,
                        std::vector<T> *const p) const
  {
    const gene &g(genome_(l));

    const opcode_t opcode(g.sym->opcode());

    const T *const s1 = reinterpret_cast<const T *>(&opcode);
    for (size_t i(0); i < sizeof(opcode); ++i)
      p->push_back(s1[i]);

    if (g.sym->parametric())
    {
      const T *const s2 = reinterpret_cast<const T *>(&g.par);
      for (size_t i(0); i < sizeof(g.par); ++i)
        p->push_back(s2[i]);
    }
    else
      for (size_t i(0); i < g.sym->arity(); ++i)
        pack({{g.args[i], function::cast(g.sym)->arg_category(i)}}, p);
  }

  ///
  /// \brief A specialized version of pack for 32bit reading/packaging.
  ///
  /// It is slightly simpler (faster?) than pack<T>.
  ///
  template <>
  void individual::pack(const locus &l,
                        std::vector<std::uint32_t> *const p) const
  {
    const gene &g(genome_(l));

    p->push_back(static_cast<std::uint32_t>(g.sym->opcode()));

    if (g.sym->parametric())
      p->push_back(static_cast<std::uint32_t>(g.par));
    else
      for (size_t i(0); i < g.sym->arity(); ++i)
        pack({{g.args[i], function::cast(g.sym)->arg_category(i)}}, p);
  }

  ///
  /// \return the signature of \c this individual.
  ///
  /// Converts \c this individual in a packed byte level representation and
  /// performs the MurmurHash3 algorithm on it.
  ///
  template<class T>
  hash_t individual::hash() const
  {
    // From an individual to a packed byte stream...
    static std::vector<T> packed;
    packed.clear();
    // In a multithread environment the two lines above must be changed with:
    //   std::vector<T> packed;
    //   // static keyword and clear() call deleted.

    pack(best_, &packed);

    /// ... and from a packed byte stream to a signature...
    const unsigned len(packed.size() * sizeof(T));  // Length in bytes

    hash_t h;
    vita::hash(packed.data(), len, 1973, h.data);
    return h;
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
  bool individual::debug(bool verbose) const
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
      if (!genome_(genome_.rows() - 1, c).sym->terminal())
      {
        if (verbose)
          std::cerr << "Last symbol of type " << c
                    << " in the genome isn't a terminal." << std::endl;
        return false;
      }

    // Type checking.
    for (index_t i(0); i < size(); ++i)
      for (category_t c(0); c < categories; ++c)
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

    if (best_[locus_index] >= size())
    {
      if (verbose)
        std::cerr << "Incorrect index for first active symbol." << std::endl;
      return false;
    }
    if (best_[locus_category] >= categories)
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
      env_->debug(verbose, true) &&
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
    for (const_iterator it(*this); it(); ++it)
    {
      const gene &g(*it);

      s << 'g' << it.l[locus_index] << '_' << it.l[locus_category]
        << " [label="
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
        << ", shape=" << (g.sym->arity() ? "box" : "parallelogram") << "];";

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << 'g' << l[locus_index] << '_' << l[locus_category] << " -- g"
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
    for (const_iterator it(*this); it(); ++it)
    {
      const gene &g(*it);

      if (it.l != best_)
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
    const size_t categories(env_->sset.categories());
    const unsigned w1(
      1 + static_cast<unsigned>(std::log10(static_cast<double>(size() - 1))));
    const unsigned w2(
      1 + static_cast<unsigned>(std::log10(static_cast<double>(categories))));

    for (const_iterator it(*this); it(); ++it)
    {
      const gene &g(*it);

      s << '[' << std::setfill('0') << std::setw(w1) << it.l[locus_index];

      if (categories > 1)
        s << ", " << std::setw(w2) << it.l[locus_category];

      s << "] "
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (size_t j(0); j < g.sym->arity(); ++j)
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
        tree(s, {{g.args[i], function::cast(g.sym)->arg_category(i)}}, indent,
             child);
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
    const unsigned width(1 + std::log10(size() - 1));

    for (unsigned i(0); i < size(); ++i)
    {
      s << '[' << std::setfill('0') << std::setw(width) << i << "] ";

      for (category_t c(0); c < categories; ++c)
      {
        const gene &g(genome_(i, c));

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
    : l(id.best_), ind_(id)
  {
    loci_.insert(l);
  }

  ///
  /// \return locus of the next line containing an active symbol.
  ///
  const locus individual::const_iterator::operator++()
  {
    if (!loci_.empty())
    {
      loci_.erase(loci_.begin());

      const gene &g(ind_.genome_(l));

      for (unsigned j(0); j < g.sym->arity(); ++j)
      {
        const locus l{{g.args[j], function::cast(g.sym)->arg_category(j)}};

        loci_.insert(l);
      }

      l = *loci_.begin();
    }

    return l;
  }

  ///
  /// \param[in] in input stream.
  /// \return \c true if individual was loaded correctly.
  ///
  /// \note
  /// If the load operation isn't successful the current individual isn't
  /// changed.
  ///
  bool individual::load(std::istream &in)
  {
    locus best;
    if (!(in >> best[locus_index] >> best[locus_category]))
      return false;

    size_t rows, cols;
    if (!(in >> rows >> cols) || !rows || !cols)
      return false;

    matrix<gene> genome(rows, cols);
    for (size_t r(0); r < rows; ++r)
      for (size_t c(0); c < cols; ++c)
      {
        opcode_t opcode;
        if (!(in >> opcode))
          return false;

        gene g;
        g.sym = env_->sset.decode(opcode);
        if (!g.sym)
          return false;

        if (g.sym->parametric())
          if (!(in >> g.par))
            return false;

        if (g.sym->arity())
          for (size_t i(0); i < g.sym->arity(); ++i)
            if (!(in >> g.args[i]))
              return false;

        genome(r, c) = g;
      }

    hash_t signature;
    if (!signature.load(in))
      return false;

    if (best[locus_index] >= genome.rows())
      return false;

    best_ = best;
    genome_ = genome;
    signature_ = signature;

    return true;
  }

  ///
  /// \param[out] out output stream.
  /// \return \c true if individual was saved correctly.
  ///
  bool individual::save(std::ostream &out) const
  {
    out << best_[locus_index] << ' ' << best_[locus_category] << std::endl;

    out << genome_.rows() << ' ' << genome_.cols() << std::endl;
    for (const gene &g : genome_)
    {
      out << g.sym->opcode();

      if (g.sym->parametric())
        out << ' ' << g.par;

      for (size_t i(0); i < g.sym->arity(); ++i)
        out << ' ' << g.args[i];

      out << std::endl;
    }

    const bool signature_ok(signature_.save(out));

    return out.good() && signature_ok;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover (we only generate a single
  ///         offspring).
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
    assert(p1.debug());
    assert(p2.debug());

    individual offspring(p1);

    for (individual::const_iterator it(p1); it(); ++it)
      if (random::boolean())
        offspring.set(it.l, p2[it.l]);

/*
    const index_t cs(p1.size());
    const category_t categories(p1.env().sset.categories());

    assert(cs == p2.size());
    assert(categories == p2.env().sset.categories());

    for (index_t i(0); i < cs; ++i)
      for (category_t c(0); c < categories; ++c)
        if (random::boolean())
        {
          const locus l{{i,c}};
          offspring.set(l, p2[l]);
        }
*/
    assert(offspring.debug(true));
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover (We only generate a single
  ///         offspring).
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a single locus
  /// (common crossover point). The offspring is created with genes from the
  /// choosen parent up to the crossover point and genes from the other parent
  /// beyond that point.
  /// One-point crossover is the oldest homologous crossover in tree-based GP.
  ///
  /// \note
  /// Parents must have the same size.
  ///
  individual one_point_crossover(const individual &p1, const individual &p2)
  {
    assert(p1.debug());
    assert(p2.debug());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());
    const unsigned categories(p1.env().sset.categories());

    const index_t cut(random::between<index_t>(1, cs - 1));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (index_t i(cut); i < cs; ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i,c}};
        offspring.set(l, (*parents[!base])[l]);
      }

    assert(offspring.debug());
    return offspring;
  }

  ///
  /// \param[in] p1 the first parent.
  /// \param[in] p2 the second parent.
  /// \return the result of the crossover (we only generate a single
  ///         offspring).
  ///
  /// We randomly select a parent (between \a p1 and \a p2) and a two loci
  /// (common crossover points). The offspring is created with genes from the
  /// choosen parent before the first crossover point and after the second
  /// crossover point; genes between crossover points are taken from the other
  /// parent.
  ///
  /// \note
  /// Parents must have the same size.
  ///
  individual two_point_crossover(const individual &p1, const individual &p2)
  {
    assert(p1.debug());
    assert(p2.debug());
    assert(p1.size() == p2.size());

    const unsigned cs(p1.size());
    const unsigned categories(p1.env().sset.categories());

    const index_t cut1(random::between<unsigned>(0, cs-1));
    const index_t cut2(random::between<unsigned>(cut1+1, cs));

    const individual *parents[2] = {&p1, &p2};
    const bool base(random::boolean());

    individual offspring(*parents[base]);

    for (index_t i(cut1); i < cut2; ++i)
      for (category_t c(0); c < categories; ++c)
      {
        const locus l{{i, c}};
        offspring.set(l, (*parents[!base])[l]);
      }

    assert(offspring.debug());
    return offspring;
  }
}  // Namespace vita
