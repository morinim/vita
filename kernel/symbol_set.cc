/**
 *
 *  \file symbol_set.cc
 *
 *  \author Manlio Morini
 *  \date 2011/05/27
 *
 *  This file is part of VITA
 *
 */

#include "symbol_set.h"
#include "adf.h"
#include "argument.h"
#include "random.h"

namespace vita
{

  ///
  /// Sets up the object.
  /// The constructor allocates memory for up to \a k_args argument.
  ///
  symbol_set::symbol_set()
  {
    clear();

    for (unsigned i(0); i < gene::k_args; ++i)
      _arguments.push_back(new argument(i));

    assert(check());
  }
 
  ///
  /// The destructor frees only memory allocated by the constructor. Object
  /// created by the client must be deleted by the client.
  ///
  symbol_set::~symbol_set()
  {
    for (std::vector<argument *>::const_iterator i(_arguments.begin());
         i != _arguments.end();
         ++i)
      delete *i;
  }

  ///
  /// Utility function used to help the constructor and the delete_symbols
  /// member function in the clean up process.
  ///
  void symbol_set::clear()
  {
    _adf.clear();
    _adf0.clear();
    _symbols.clear();
    _terminals.clear();
    _specials.clear();

    _sum = 0;
  }

  ///
  /// The basic rule of memory management in Vita is that any object that is
  /// created by the client must be deleted by the client... but, you know,
  /// every rule has an exception: subclasses of \a problem need to free
  /// memory in their destructors and the deallocation process is "factored"
  /// in the \a delete_symbols member function.
  ///
  void symbol_set::delete_symbols()
  {
    for (std::vector<symbol *>::const_iterator i(_symbols.begin());
         i != _symbols.end();
         ++i)
      delete *i;

    clear();
  }

  ///
  /// \param[in] n index of an argument symbol.
  /// \return a pointer to the n-th argument symbol.
  ///
  const argument *symbol_set::arg(unsigned n) const
  {
    assert(n < gene::k_args);
    return _arguments[n];
  }

  ///
  /// \param[in] i index of an adf0 function.
  /// \return a pointer to the i-th adf0 function.
  ///
  const adf_0 *symbol_set::get_adf0(unsigned i) const
  {
    return i < _adf0.size() ? _adf0[i] : 0;
  }

  ///
  /// \param[in] n index of a special \a symbol.
  /// \return a pointer to the n-th special \a symbol.
  ///
  const symbol *symbol_set::get_special(unsigned n) const
  {
    assert(n < _specials.size());
    return _specials[n];
  }

  ///
  /// \return the number of special symbols in the symbol set.
  ///
  unsigned symbol_set::specials() const
  {
    return _specials.size();
  }

  ///
  /// \param[in] i symbol to be added.
  /// \param[in] special if \c true the \a symbol is not used during initial
  ///            random generation but it's inserted at the end of the genome
  ///            in a protected area.
  ///
  /// Adds a new \a symbol to the set.
  ///
  void symbol_set::insert(symbol *const i, bool special)
  {
    assert(i && i->weight && i->check());
    
    if (special)
    {
      assert(i->terminal());
      _specials.push_back(static_cast<terminal *>(i));
    }
    else
    {
      _symbols.push_back(i);
      _sum += i->weight;
    }

    if (i->terminal())
    {
      _terminals.push_back(static_cast<terminal *>(i));

      adf_0 *const df = dynamic_cast<adf_0 *>(i);
      if (df)
        _adf0.push_back(df);
    }
    else  // not a terminal
    {
      adf_n *const df = dynamic_cast<adf_n *>(i);
      if (df)
        _adf.push_back(df);
    }

    assert(check());
  }

  ///
  void symbol_set::reset_adf_weights()
  {
    for (unsigned i(0); i < _adf0.size(); ++i)
    {
      const unsigned w(_adf0[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      _sum -= delta;
      _adf0[i]->weight -= delta;

      if (delta && _adf0[i]->weight == 0)
      {
        for (unsigned j(0); j < _terminals.size(); ++j)
          if (_terminals[j]->opcode() == _adf0[i]->opcode())
          {
            _terminals.erase(_terminals.begin()+j);
            break;
          }

        for (unsigned j(0); j < _symbols.size(); ++j)
          if (_symbols[j]->opcode() == _adf0[i]->opcode())
          {
            _symbols.erase(_symbols.begin()+j);
            break;
          }
      }
    }

    for (unsigned i(0); i < _adf.size(); ++i)
    {
      const unsigned w(_adf[i]->weight);
      const unsigned delta(w >  1 ? w/2 :
                           w == 1 ? 1 : 0);
      _sum -= delta;
      _adf[i]->weight -= delta; 
    }
  }

  ///
  /// \param[in] only_t if true extracts only terminals.
  /// \return a random symbol.
  ///
  /// If \a only_t == \c true extracts a \a terminal else a random \a symbol 
  /// (may be a \a terminal, a primitive function or an ADF).
  ///
  const symbol *symbol_set::roulette(bool only_t) const
  {
    assert(_sum);

    if (only_t)
    {
      const unsigned i(random::between<unsigned>(0,_terminals.size()));

      assert( !dynamic_cast<argument *>(_terminals[i]) );
      return _terminals[i];
    }

    const unsigned slot(random::between<unsigned>(0,_sum)); 

    unsigned i(0);
    for (unsigned wedge(_symbols[i]->weight);
	 wedge <= slot && i+1 < _symbols.size();
	 wedge += _symbols[++i]->weight)
      ;

    assert( !dynamic_cast<argument *>(_symbols[i]) );
    return _symbols[i];
  }

  ///
  /// \param[in] opcode numerical code used as primary key for a symbol.
  /// \return a pointer to the \c symbol identified by 'opcode' (0 if not 
  ///         found).
  ///
  const symbol *symbol_set::decode(unsigned opcode) const
  {
    for (unsigned i(0); i < _symbols.size(); ++i)
      if (_symbols[i]->opcode() == opcode)
	return _symbols[i];

    return 0;
  }

  ///
  /// \param[in] dex the name of a symbol.
  /// \return a pointer to the \c symbol identified by 'dex' (0 if not found).
  ///
  /// Please note that opcode are primary key for symbols because they are 
  /// automatically assigned. The name of a symbol is choosen by the user,
  /// so if you don't pay attention different symbols may have the same name.
  ///
  const symbol *symbol_set::decode(const std::string &dex) const
  {
    for (unsigned i(0); i < _symbols.size(); ++i)
      if (_symbols[i]->display() == dex)
	return _symbols[i];

    return 0;
  }

  ///
  /// \return true if the individual passes the internal consistency check.
  ///
  bool symbol_set::check() const
  {
    unsigned long sum(0);

    for (unsigned j(0); j < _symbols.size(); ++j)
    {
      if (!_symbols[j]->check())
	return false;

      sum += _symbols[j]->weight;

      if (_symbols[j]->weight == 0)
        return false;

      bool found(false);    
      if (_symbols[j]->terminal())
      {
        // Terminals must be in the _terminals vector.
        for (unsigned i(0); i < _terminals.size() && !found; ++i)
          found = (_symbols[j] == _terminals[i]);

        if (dynamic_cast<adf_0 *>(_symbols[j]))
          for (unsigned i(0); i < _adf0.size() && !found; ++i)
            found = (_symbols[j] == _adf0[i]);
      }
      else if (dynamic_cast<adf_n *>(_symbols[j]))
        for (unsigned i(0); i < _adf.size() && !found; ++i)
          found = (_symbols[j] == _adf[i]);
      else
        found = true;
     
      if (!found)
	return false;
    }
  
    if (sum != _sum)
      return false;

    // There should be one terminal at least.
    return _symbols.size()==0 || _terminals.size()>0;
  }

}  // Namespace vita
