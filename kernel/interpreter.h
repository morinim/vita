/**
 *
 *  \file interpreter.h
 *
 *  \author Manlio Morini
 *  \date 2011/02/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(INTERPRETER_H)
#define      INTERPRETER_H

#include <vector>

#include <boost/any.hpp>

#include "vita.h"

namespace vita
{

  class individual;

  ///
  /// \example example5.cc
  /// Output value calculation for an individual.
  ///
  class interpreter
  {
  public:
    explicit interpreter(const individual &, interpreter *const = 0);

    boost::any run();

    boost::any eval();
    boost::any eval(unsigned);
    boost::any eval_adf_arg(unsigned);

    bool check() const;

  private:
    /// Instruction pointer.
    unsigned _ip;

    interpreter *const _context;

    const individual &_ind;

    struct cache_elem
    {
      bool       empty;
      boost::any value;
    };

    mutable std::vector<cache_elem> _cache;
    mutable std::vector<boost::any> _context_cache;
  };
    
}  // namespace vita

#endif  // INTERPRETER_H
