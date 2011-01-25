/**
 *
 *  \file interpreter.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "interpreter.h"
#include "adf.h"
#include "individual.h"
#include "symbol.h"

namespace vita
{

  ///
  /// \param[in] ind individual whose value we are interested in,
  /// \param[in] ctx context in which we calculate the output value (used for
  ///                the evaluation of ADF).
  ///
  interpreter::interpreter(const individual &ind, 
                           interpreter *const ctx) 
    : _ip(ind._best), _context(ctx), _ind(ind),
      _cache(ind.size()),
      _context_cache(ctx ? ctx->_ind.size() : 0)
  {
  }

  ///
  /// \return the output value of \c this individual.
  ///
  boost::any
  interpreter::run()
  {
    for (unsigned i(0); i < _cache.size(); ++i)
      _cache[i] = boost::any();
    // Probably the \c _context_cache vector will be deleted (it was introduced 
    // before the \c _cache vector).
    for (unsigned i(0); i < _context_cache.size(); ++i)
      _context_cache[i] = boost::any();

    _ip = _ind._best;
    return _ind._code[_ip].sym->eval(*this);
  }

  ///
  /// \return the output value of the current terminal  symbol.
  ///
  boost::any
  interpreter::eval()
  {
    const gene &g(_ind._code[_ip]);

    assert(g.sym->parametric());
    return g.par;
  }

  ///
  /// \param[in] i i-th argument of the current function.
  /// \return the value of the i-th argument of the current function.
  ///
  boost::any
  interpreter::eval(unsigned i)
  {
    const gene &g(_ind._code[_ip]);

    assert(i < g.sym->argc());

    if (_cache[g.args[i]].empty())
    {
      const unsigned backup(_ip); 
      _ip = g.args[i];
      assert (_ip > backup);
      const boost::any ret(_ind._code[_ip].sym->eval(*this));
      _ip = backup;
      _cache[g.args[i]] = ret;
    }
#if !defined(NDEBUG)
    else
    {
      const unsigned backup(_ip); 
      _ip = g.args[i];
      assert (_ip > backup);
      const boost::any ret(_ind._code[_ip].sym->eval(*this));
      _ip = backup;
      if (ret.type() == typeid(int))
	assert(boost::any_cast<int>(ret) == 
	       boost::any_cast<int>(_cache[g.args[i]]));
      else if (ret.type() == typeid(double))
	assert(boost::any_cast<double>(ret) == 
	       boost::any_cast<double>(_cache[g.args[i]]));
    }
#endif
    
    return _cache[g.args[i]];

    //return _ind._code[g.args[i]].sym->eval(interpreter(_ind,_context,
    //                                                   g.args[i]));
  }

  ///
  /// \param[in] i-th argument of the current ADF,
  /// \return the value of the i-th argument of the curren ADF function.
  ///
  boost::any
  interpreter::eval_adf_arg(unsigned i)
  {
    const gene context_g(_context->_ind._code[_context->_ip]);

    assert( _context && _context->check() && i < gene_args && 
            dynamic_cast<const adf *>(context_g.sym) );

    if (_context_cache[context_g.args[i]].empty())
      _context_cache[context_g.args[i]] = _context->eval(i);

    return _context_cache[context_g.args[i]];
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
  bool
  interpreter::check() const
  {
    return
      _ip < _ind._code.size() &&
      (!_context || _context->check());
  }

}  // Namespace vita
