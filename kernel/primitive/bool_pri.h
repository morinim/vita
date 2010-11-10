/**
 *
 *  \file bool_pri.h
 *
 *  \author Manlio Morini
 *  \date 2009/10/31
 *
 *  This file is part of VITA
 *
 */

  
#if !defined(SR_PRIMITIVE_H)
#define      SR_PRIMITIVE_H

#include <cstdlib>
#include <limits>
#include <sstream>

#include <boost/any.hpp>

#include "vita.h"
#include "function.h"
#include "interpreter.h"
#include "random.h"
#include "terminal.h"

namespace vita
{

namespace boolean
{

  /**
   * variable
   */
  class variable : public terminal
  {
  public:
    variable(const std::string &name) : terminal(name,sym_bool) {};
    
    boost::any eval(vita::interpreter &) const { return val; };

    bool val;
  };

  /**
   * zero
   */
  class zero : public terminal
  {
  public:
    zero() : terminal("0",sym_bool,terminal::default_weight*3) {};

    std::string display() const { return "0"; };

    boost::any eval(interpreter &) const { return false; };
  };

  /**
   * one
   */
  class one : public terminal
  {
  public:
    one() : terminal("1",sym_bool,terminal::default_weight*3) {};

    std::string display() const { return "1"; };

    boost::any eval(interpreter &) const { return true; };
  };

  /**
   * and
   */
  class and : public function
  {
  public:
    and() : function("AND",sym_bool,2,function::default_weight,true) {};

    boost::any eval(interpreter &i) const
    { 
      return boost::any_cast<bool>(i.eval(0)) && 
             boost::any_cast<bool>(i.eval(1)) );
    };
  };

  /**
   * not
   */
  class not : public function
  {
  public:
    not() : function("NOT",sym_bool,1) {};

    boost::any eval(interpreter &i) const 
    { return !boost::any_cast<bool>(i.eval(0)); };
  };

  /**
   * or
   */
  class or : public function
  {
  public:
    or() : function("OR",sym_bool,2,function::default_weight,true) {};

    boost::any eval(interpreter &i) const
    { 
      return boost::any_cast<bool>(i.eval(0)) || 
             boost::any_cast<bool>(i.eval(1)); 
    };
  };

}  // namespace boolean

}  // namespace vita
