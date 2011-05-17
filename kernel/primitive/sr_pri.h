/**
 *
 *  \file sr_pri.h
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

namespace sr
{

  /**
   * variable
   */
  class variable : public terminal
  {
  public:
    variable(const std::string &name, symbol_t t=sym_real)
      : terminal(name,t,true) {};
    
    boost::any eval(vita::interpreter &) const
    { 
      return is_bad(val) ? boost::any() : val; 
    };

    double val;
  };

  /**
   * constant
   */
  class constant : public terminal
  {
  public:
    constant(double c, symbol_t t=sym_real) 
      : terminal("CONST",t,terminal::default_weight*2), val(c) {};

    std::string display() const
    {
      std::ostringstream s;
      s << val;
      return s.str();
    };

    boost::any eval(interpreter &) const { return val; };

    const double val;
  };
  
  /**
   * number
   */
  class number : public terminal
  {
  public:
    number(int m, int u, symbol_t t=sym_real) 
      : terminal("NUM",t,default_weight*2,true), min(m), upp(u) {}; 

    int init() const { return random::between<int>(min,upp); };

    std::string display(int v) const
    {
      std::ostringstream s;
      s << double(v);
      return s.str();
    };

    boost::any eval(interpreter &i) const
    { 
      return double(boost::any_cast<int>(i.eval())); 
    };

  private:
    const int min, upp;
  };

  /**
   * abs
   */
  class abs : public function
  {
  public:
    abs(symbol_t t=sym_real) : function("ABS",t,1) {};

    boost::any eval(interpreter &i) const
    {
      const boost::any ev(i.eval(0));
      if (ev.empty())  return ev;

      return std::fabs(boost::any_cast<double>(ev));
    };
  };

  /**
   * add
   */
  class add : public function
  {
  public:
    add(symbol_t t=sym_real) 
      : function("ADD",t,2,function::default_weight,true) {};

    boost::any eval(interpreter &i) const 
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;
      
      const double ret(boost::any_cast<double>(ev0) + 
                       boost::any_cast<double>(ev1));
      if (isinf(ret))  return boost::any();

      return ret;
    };
  };

  /**
   * div
   */
  class div : public function
  {
  public:
    div(symbol_t t=sym_real) : function("DIV",t,2) {};

    boost::any eval(interpreter &i) const 
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      const double ret(boost::any_cast<double>(ev0) /
                       boost::any_cast<double>(ev1));
      if (is_bad(ret))  return boost::any();

      return ret;
    };
  };

  /**
   * idiv
   */
  class idiv : public function
  {
  public:
    idiv(symbol_t t=sym_real) : function("IDIV",t,2) {};

    boost::any eval(interpreter &i) const 
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      const double ret(std::floor(boost::any_cast<double>(ev0) / 
                                  boost::any_cast<double>(ev1))); 
      if (is_bad(ret))  return boost::any();

      return ret;
    };
  };

  /**
   * ife
   */
  class ife : public function
  {
  public:
    ife(symbol_t t=sym_real) : function("IFE",t,4) {};

    boost::any eval(interpreter &i) const
    {
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      const double cmp(std::fabs(boost::any_cast<double>(ev0) -
                                 boost::any_cast<double>(ev1)));

      if (cmp < float_epsilon)
	return i.eval(2);
      else
	return i.eval(3);
    };
  };

  /**
   * ifl
   */
  class ifl : public function
  {
  public:
    ifl(symbol_t t=sym_real) : function("IFL",t,4) {};

    boost::any eval(interpreter &i) const
    {
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      if ( boost::any_cast<double>(ev0) < boost::any_cast<double>(ev1) )
	return i.eval(2);
      else
	return i.eval(3);
    };
  };

  /**
   * ifz
   */
  class ifz : public function
  {
  public:
    ifz(symbol_t t=sym_real) : function("IFZ",t,3) {};

    boost::any eval(interpreter &i) const
    {
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      if (std::fabs(boost::any_cast<double>(ev0)) < float_epsilon)
	return i.eval(1);
      else
	return i.eval(2);
    };
  };

  /**
   * ln
   */
  class ln : public function
  {
  public:
    ln(symbol_t t=sym_real) 
      : function("LN",t,1,function::default_weight/2) {};

    boost::any eval(interpreter &i) const 
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const double ret(std::log(boost::any_cast<double>(i.eval(0)))); 
      if (is_bad(ret))  return boost::any();

      return ret;
    };
  };

  /**
   * mod
   */
  class mod : public function
  {
  public:
    mod(symbol_t t=sym_real) : function("MOD",t,2) {};

    boost::any eval(interpreter &i) const
    {
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      const double ret(std::fmod(boost::any_cast<double>(ev0),
                                 boost::any_cast<double>(ev1)));
      if (is_bad(ret))  return boost::any();

      return ret;
    };
  };

  /**
   * mul
   */
  class mul : public function
  {
  public:
    mul(symbol_t t=sym_real) 
      : function("MUL",t,2,function::default_weight,true) {};

    boost::any eval(interpreter &i) const
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;

      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;

      const double ret(boost::any_cast<double>(ev0) * 
                       boost::any_cast<double>(ev1)); 
      if (isinf(ret))  return boost::any();
      
      return ret;
    };
  };

  /**
   * sin
   */
  class sin : public function
  {
  public:
    sin(symbol_t t=sym_real) : function("SIN",t,1) {};

    boost::any eval(interpreter &i) const 
    {
      const boost::any ev(i.eval(0));
      if (ev.empty())  return ev;

      return std::sin(boost::any_cast<double>(ev)); 
    };
  };

  /**
   * sub
   */
  class sub : public function
  {
  public:
    sub(symbol_t t=sym_real) : function("SUB",t,2) {};

    boost::any eval(interpreter &i) const
    { 
      const boost::any ev0(i.eval(0));
      if (ev0.empty())  return ev0;
 
      const boost::any ev1(i.eval(1));
      if (ev1.empty())  return ev1;
 
      const double ret(boost::any_cast<double>(ev0) - 
                       boost::any_cast<double>(ev1)); 
      if (isinf(ret))  return boost::any();

      return ret;
    };
  };

}  // namespace sr

}  // namespace vita

#endif  // PRIMITIVE_H
