/**
 *
 *  \file adf.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(ADF_H)
#define      ADF_H

#include "vita.h"
#include "function.h"
#include "individual.h"

namespace vita
{

  class interpreter;

  class adf : public function
  {
  public:
    adf(const individual &, const std::vector<symbol_t> &, unsigned);

    boost::any eval(interpreter &) const;

    std::string display() const;

    const individual &get_code() const;

    bool check() const;

  private:
    static unsigned _adf_count;

    const unsigned _id;
    individual   _code;
  };

}  // namespace vita

#endif  // ADF_H
