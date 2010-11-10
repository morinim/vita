/**
 *
 *  \file argument.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(ARGUMENT_H)
#define      ARGUMENT_H

#include "vita.h"
#include "terminal.h"

namespace vita
{

  class interpreter;

  class argument : public terminal
  {
  public:
    argument(unsigned);

    std::string display() const;

    unsigned index() const;

    boost::any eval(interpreter &) const;

    bool check() const;

  private:
    const unsigned _index;
  };

  /**
   * index
   * \return
   */
  inline
  unsigned
  argument::index() const
  {
    return _index;
  }

  /**
   * display
   * \return
   */
  inline
  std::string
  argument::display() const
  {
    std::ostringstream s;
    s << "ARG" << '_' << _index;
    return s.str();
  }
    
}  // namespace vita

#endif  // ARGUMENT_H
