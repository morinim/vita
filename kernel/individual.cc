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
#include "kernel/symbol.h"
#include "kernel/transformer.h"

namespace vita
{
  std::vector<std::shared_ptr<const transformer>> individual::cross_array_
  {
    std::shared_ptr<const transformer>(new uniform_crossover()),
    std::shared_ptr<const transformer>(new one_point_crossover()),
    std::shared_ptr<const transformer>(new two_point_crossover())
  };

  ///
  /// \param[in] e base environment.
  /// \param[in] gen if true generates a random sequence of genes to initialize
  ///                the individual.
  ///
  individual::individual(const environment &e, bool gen)
    : active_cross_(0), best_(0), env_(&e), code_(e.code_length)
  {
    assert(e.check());

    // **** Random generate initial code. ****
    if (gen)
    {
      const unsigned specials(e.sset.specials());
      assert(specials < size());

      const unsigned sup(size() - specials);

      for (unsigned i(0); i < sup; ++i)
        code_[i] = gene(e.sset, i+1, e.code_length);

      for (unsigned i(0); i < specials; ++i)
        code_[sup+i] = gene(e.sset, i);

      assert(check());
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
  /// \param[out] last_symbol pointer to the la symbol of the compacted
  ///                         individual.
  /// \return a new compacted individual.
  ///
  /// Create a new individual functionally equivalent to \c this but with the
  /// active symbols compacted and stored at the beginning of the code vector.
  /// [<- active symbols ->][<- introns ->]
  /// 0 1 2 3 ...                         n    <- locus
  ///
  individual individual::compact(unsigned *last_symbol) const
  {
    individual dest(*env_, false);

    unsigned new_line(0), old_line(best_);
    for (const_iterator it(*this); it(); ++new_line, old_line = ++it)
    {
      dest.code_[new_line] = *it;

      for (unsigned l(0); l < new_line; ++l)
        for (unsigned arg(0); arg < dest.code_[l].sym->arity(); ++arg)
          if (dest.code_[l].args[arg] == old_line)
            dest.code_[l].args[arg] = new_line;
    }

    if (last_symbol && new_line)
      *last_symbol = new_line-1;

    assert(new_line == 0 ||
           (eff_size() && 0 < new_line && new_line <= dest.size()));

    return dest;
  }

  ///
  /// \param[out] first_t pointer to the first symbol of the optimized
  ///                     individual.
  /// \param[out] last_s pointer to the last symbol of the optimized individual.
  /// \return a new optimized individual.
  ///
  /// Create a new \a individual functionally equivalent to \c this but with the
  /// active functions compacted and stored at the beginning of the code vector
  /// and active terminals grouped at the end of the block. Redundant terminals
  /// are removed.
  /// [<- Active functions ->|<- Active terminals ->|<- Introns ->]
  ///
  individual individual::optimize(unsigned *first_t, unsigned *last_s) const
  {
    // Step 1: compact the active symbols and put them at the beginning of the
    // code vector.
    unsigned first_terminal;
    individual source(compact(&first_terminal));

    // Step 2: reorganize the symbols so that terminals will be at the end of
    // the active symbols's block.
    assert(source.code_[first_terminal].sym->terminal());
    const unsigned last_terminal(first_terminal);

    for (unsigned i(1); i < first_terminal; ++i)   // Looking for terminals.
      if (source.code_[i].sym->terminal())
      {
        // We have a new terminal that should moved at the end of the active
        // symbols' block.
        unsigned found(0);
        for (unsigned j(first_terminal); j <= last_terminal && !found; ++j)
        {
          assert(source.code_[j].sym->terminal());
          if (source.code_[i] == source.code_[j])
            found = j;
        }

        // The new terminal could be already present (because it was
        // duplicated). We only need one terminal for each type.
        if (found)
        {
          // Move the symbols before the duplicated terminal one location
          // forward. The duplicated terminal will be overwritten.
          // We must pay attention to exactly change the arguments of the
          // moved functions.
          for (unsigned j(i); j > 0; --j)
          {
            source.code_[j] = source.code_[j-1];

            for (unsigned k(0); k < source.code_[j].sym->arity(); ++k)
            {
              locus_t &arg(source.code_[j].args[k]);
              if (arg == i)
                arg = found;
              else if (arg < i)
                ++arg;
            }
          }

          ++source.best_;
        }
        else  // !found
        {
          --first_terminal;

          if (first_terminal != i)
          {
            // Rearrange the arguments of the functions before the terminal
            // that will be moved.
            for (unsigned j(source.best_); j < i; ++j)
              for (unsigned k(0); k < source.code_[j].sym->arity(); ++k)
              {
                locus_t &arg(source.code_[j].args[k]);

                if (arg == i)
                  arg = first_terminal;
                else if (i < arg && arg <= first_terminal)
                  --arg;
              }

            const gene g(source.code_[i]);

            // Move the symbols after the terminal one location backward. The
            // duplicated terminal will be overwritten (but we have a copy).
            // We must pay attention to exactly change the arguments of the
            // moved functions.
            for (unsigned j(i); j < first_terminal; ++j)
            {
              source.code_[j] = source.code_[j+1];

              for (unsigned k(0); k < source.code_[j].sym->arity(); ++k)
              {
                locus_t &arg(source.code_[j].args[k]);

                if (arg <= first_terminal)
                  --arg;
              }
            }

            source.code_[first_terminal] = g;
            --i;
          }
        }
      }

    if (first_t)
      *first_t = first_terminal;
    if (last_s)
      *last_s = last_terminal;

    return source;
  }

  ///
  /// \param[in] locus location of the \a individual.
  /// \return an individual obtained from \c this choosing the gene
  ///         sequence starting at \a locus.
  ///
  /// This function is often used along with the \ref blocks function.
  ///
  individual individual::get_block(unsigned locus) const
  {
    individual ret(*this);

    ret.best_ = locus;

    assert(ret.check());
    return ret;
  }

  ///
  /// \param[out] number of mutations performed.
  /// \return a new, mutated, individual.
  ///
  /// A new individual is created mutating \c this individual. If there are
  /// special symbols (env_->sset.specials() > 0) they are protected from
  /// mutation.
  ///
  individual individual::mutation(unsigned *const n_mutations) const
  {
    individual ret(*this);
    unsigned n(0);

    const unsigned specials(env_->sset.specials());
    assert(specials < size());
    const unsigned cs(size() - specials);

    for (unsigned i(0); i < cs; ++i)
      if (random::boolean(env_->p_mutation))
      {
        ++n;

        ret.code_[i] = gene(env_->sset, i+1, size());
      }

    assert(ret.check());

    if (n_mutations)
      *n_mutations = n;

    return ret;
  }

  ///
  /// \param[in] parent2 the second parent (being \c this the first).
  /// \return a pointer to \c this.
  ///
  /// There are some predefined crossover operators: we use the \c active_cross_
  /// index to choose which one to use. For a description of the available
  /// operators see the \see transformation \c class.
  ///
  individual individual::crossover(const individual &parent2) const
  {
    assert(check());
    assert(parent2.check());
    assert(active_cross_ < cross_array_.size());

    return (*cross_array_[active_cross_])(*this, parent2);
  }

  ///
  /// \return a list of indexes referring to active symbols.
  ///
  /// The function extract from the individual a list of indexes to blocks
  /// that are subsets of the active code. Indexes can be used as they would be
  /// individuals by the get_block function.
  ///
  std::list<unsigned> individual::blocks() const
  {
    std::list<unsigned> bl;

    unsigned line(best_);
    for (const_iterator i(*this); i(); line = ++i)
      if (code_[line].sym->arity())
        bl.push_back(line);
        /*
        for (unsigned j(0); j < code_[line].sym->arity(); ++j)
          if ( code_[code_[line].args[j]].sym->arity() )  // At least depth 3
          {
            bl.push_back(line);
            break;
          }
        */

    return bl;
  }

  ///
  /// \param[in] sym symbol used for replacement.
  /// \param[in] args new arguments.
  /// \param[in] line locus where replacement take place.
  /// \return a new \a individual with a gene replaced.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// \a symbol at line \a line with a new one ('sym' + 'args').
  ///
  individual individual::replace(symbol_ptr sym,
                                 const std::vector<locus_t> &args,
                                 unsigned line) const
  {
    assert(sym);

    individual ret(*this);

    ret.code_[line].sym = sym;
    for (unsigned i(0); i < args.size(); ++i)
      ret.code_[line].args[i] = args[i];

    assert(ret.check());
    return ret;
  }

  ///
  /// \param[in] sym symbol ysed for replacement.
  /// \param[in] args new arguments.
  /// \return a new individual with \a best_ line replaced.
  ///
  /// Create a new \a individual obtained from \c this replacing the original
  /// \a symbol at line \a best_ with a new one ('sym' + 'args').
  ///
  individual individual::replace(symbol_ptr sym,
                                 const std::vector<locus_t> &args) const
  {
    return replace(sym, args, best_);
  }

  ///
  /// \param[in] line index of a \a symbol in the \a individual.
  /// \return a new individual obtained from \c this inserting a random
  ///         \a terminal at index \a line.
  ///
  individual individual::destroy_block(unsigned line) const
  {
    assert(line < size() && !code_[line].sym->terminal());

    individual ret(*this);
    ret.code_[line] = gene(env_->sset);

    assert(ret.check());

    return ret;
  }

  ///
  /// \param[in] max_args maximum number of arguments for the ADF.
  /// \param[out] positions locus of the ADF arguments.
  /// \param[out] types type of the ADF arguments.
  /// \return the generalized individual.
  ///
  /// Changes up to \a max_args terminals (exactly \a max_args when available)
  /// of \c this individual with formal arguments, thus producing the body
  /// for a ADF.
  ///
  individual individual::generalize(std::size_t max_args,
                                    std::vector<locus_t> *const positions,
                                    std::vector<symbol_t> *const types) const
  {
    assert(max_args && max_args <= gene::k_args);

    std::vector<locus_t> terminals;

    // Step 1: mark the active terminal symbols.
    unsigned line(best_);
    for (const_iterator i(*this); i(); line = ++i)
      if (code_[line].sym->terminal())
        terminals.push_back(line);

    // Step 2: shuffle the terminals and pick elements 0..n-1.
    const unsigned n(std::min(max_args, terminals.size()));
    assert(n);

    if (n < size())
      for (unsigned j(0); j < n; ++j)
      {
        const unsigned r(random::between<unsigned>(j, terminals.size()));

        const unsigned tmp(terminals[j]);
        terminals[j] = terminals[r];
        terminals[r] = tmp;
      }

    // Step 3: randomly substitute n terminals with function arguments.
    individual ret(*this);
    for (unsigned j(0); j < n; ++j)
    {
      gene &g(ret.code_[terminals[j]]);
      if (types)
        types->push_back(g.sym->type());
      if (positions)
        positions->push_back(terminals[j]);
      g.sym = env_->sset.arg(j);
    }

    assert(!positions || (positions->size() && positions->size() <= max_args));
    assert(!types || (types->size() && types->size() <= max_args));
    assert(!positions || !types || positions->size() == types->size());

    return ret;
  }

  ///
  /// \return the type of the individual.
  ///
  symbol_t individual::type() const
  {
    return code_[best_].sym->type();
  }

  ///
  /// \param[in] x second term of comparison.
  /// \return true if the two individuals are equal (symbol by symbol,
  ///         including introns).
  ///
  bool individual::operator==(const individual &x) const
  {
    return code_ == x.code_ && best_ == x.best_;
  }

  ///
  /// \param[in] ind an individual to compare with \c this.
  /// \return a numeric measurement of the difference between \a ind and
  /// \c this (the number of different genes between individuals).
  ///
  unsigned individual::distance(const individual &ind) const
  {
    const unsigned cs(size());

    unsigned d(0);
    for (unsigned i(0); i < cs; ++i)
      if (code_[i] != ind.code_[i])
        ++d;

    return d;
  }

  ///
  /// \param[out] p byte stream compacted version of the gene sequence
  ///               starting at locus \a idx.
  /// \param[in] idx locus in \c this individual.
  ///
  /// The generated byte stream has some interesting properties:
  /// -
  ///
  void individual::pack(std::vector<boost::uint8_t> *const p,
                        unsigned idx) const
  {
    const gene &g(code_[idx]);

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
        pack(p, g.args[i]);
  }

  ///
  /// \param[in] p packed byte stream representation of an individual.
  /// \param[in] idx locus starting from where unpack \a p stream.
  /// \return number of bytes unpacked.
  ///
  unsigned individual::unpack(const std::vector<boost::uint8_t> &p,
                              unsigned idx)
  {
    unsigned unpacked(0);

    opcode_t opcode;
    std::memcpy(&opcode, &p[idx], sizeof(opcode));
    unpacked += sizeof(opcode);

    gene g;
    g.sym = env_->sset.decode(opcode);

    if (g.sym->parametric())
    {
      std::memcpy(&g.par, &p[idx+unpacked], sizeof(g.par));
      unpacked += sizeof(g.par);
    }

    code_.push_back(g);
    const unsigned base(size()-1);
    for (unsigned i(0); i < g.sym->arity(); ++i)
    {
      code_[base].args[i] = size();
      unpacked += unpack(p, idx+unpacked);
    }

    return unpacked;
  }

  ///
  /// \return \c true if the individual passes the internal consistency check.
  ///
  bool individual::check() const
  {
    bool last_is_terminal(false);
    unsigned line(best_);
    for (const_iterator it(*this); it(); line = ++it)
    {
      if (!code_[line].sym)
        return false;

      if (code_[line].sym->arity() > gene::k_args)
        return false;

      for (unsigned j(0); j < code_[line].sym->arity(); ++j)
        if (code_[line].args[j] >= size() || code_[line].args[j] <= line)
          return false;

      last_is_terminal = code_[line].sym->terminal();
    }

    const unsigned specials(env_->sset.specials());
    for (unsigned i(size()-specials); i < size(); ++i)
      if (!code_[i].sym->terminal())
        return false;

    return
      best_ < size() &&
      last_is_terminal &&
      size() < (1u << 8*sizeof(locus_t)) &&
      eff_size() <= size() &&
      env_->check();
  }

  ///
  /// \param[out] s output stream.
  /// \param[in] id
  ///
  void individual::graphviz(std::ostream &s, const std::string &id) const
  {
    if (id.empty())
      s << "graph";
    else
      s << "subgraph " << id;
    s << " {";

    unsigned line(best_);
    for (const_iterator it(*this); it(); line = ++it)
    {
      const gene &g(*it);

      s << 'g' << line << " [label="
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
        << "];";

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << 'g' << line << " -- g" << g.args[j] << ';';
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
    unsigned line(best_);
    for (const_iterator it(*this); it(); line = ++it)
    {
      const gene &g(*it);

      if (line != best_)
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
    const unsigned width(
      1 + static_cast<unsigned>(std::log10(static_cast<double>(size()-1))) );

    unsigned line(best_);
    for (const_iterator it(*this); it(); line = ++it)
    {
      const gene &g(*it);

      s << '[' << std::setfill('0') << std::setw(width) << line << "] "
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << ' ' << std::setw(width) << g.args[j];

      s << std::endl;
    }
  }

  ///
  /// \param[out] s
  /// \param[in] locus
  /// \param[in] indt
  /// \param[in] father
  ///
  void individual::tree(std::ostream &s,
                        unsigned locus, unsigned indt, unsigned father) const
  {
    const gene &g(code_[locus]);

    if (locus == father
        || !code_[father].sym->associative()
        || code_[father].sym != g.sym)
    {
      std::string spaces(indt, ' ');
      s << spaces
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display())
        << std::endl;
      indt += 2;
    }

    const unsigned arity(g.sym->arity());
    if (arity)
      for (unsigned i(0); i < arity; ++i)
        tree(s, g.args[i], indt, locus);
  }

  ///
  /// \param[out] s
  ///
  void individual::tree(std::ostream &s) const
  {
    tree(s, best_, 0, best_);
  }

  ///
  /// \param[out] s
  ///
  void individual::dump(std::ostream &s) const
  {
    const unsigned width(1 + std::log10(size()-1));

    for (unsigned i(0); i < size(); ++i)
    {
      const gene &g(code_[i]);

      s << '[' << std::setfill('0') << std::setw(width) << i << "] "
        << (g.sym->parametric() ? g.sym->display(g.par) : g.sym->display());

      for (unsigned j(0); j < g.sym->arity(); ++j)
        s << ' ' << std::setw(width) << g.args[j];

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
    lines_.insert(l_);
  }

  ///
  /// \return
  ///
  unsigned individual::const_iterator::operator++()
  {
    if (!lines_.empty())
    {
      lines_.erase(lines_.begin());

      assert(l_ < ind_.code_.size());
      const gene &g(ind_.code_[l_]);

      for (unsigned j(0); j < g.sym->arity(); ++j)
        lines_.insert(g.args[j]);

      l_ = *lines_.begin();
    }

    return l_;
  }
}  // Namespace vita
