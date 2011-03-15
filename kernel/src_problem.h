/**
 *
 *  \file src_problem.h
 *
 *  \author Manlio Morini
 *  \date 2011/03/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(SRC_PROBLEM_H)
#define      SRC_PROBLEM_H

#include <cmath>

#include "vita.h"
#include "data.h"
#include "problem.h"
#include "primitive/sr_pri.h"

namespace vita
{

  class src_problem : public problem
  {
  public:
    src_problem();
    ~src_problem();

    bool load_data(const std::string &);
    std::string load_symbols(const std::string &);

    void clear();

    unsigned classes() const;
    unsigned variables() const;

    bool check() const;
    
  private:
    std::vector<vita::sr::variable *> vars;
    data dat;
  };

}  // namespace vita

#endif  // SRC_PROBLEM_H
