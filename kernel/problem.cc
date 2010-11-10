/**
 *
 *  \file problem.cc
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */

#include "problem.h"
#include "search.h"

namespace vita
{

  /**
   * problem
   */
  problem::problem()
  {
    clear();
  }

  /**
   * clear
   */
  void
  problem::clear()
  {
    vars.clear();
    dat.clear();
  }

  /**
   * load_data
   * \param f
   */
  bool
  problem::load_data(const std::string &f)
  {
    dat.clear();

    if (dat.open(f) > 1)
    {
      // Sets up the variables.
      for (unsigned i(0); i < dat.variables(); ++i)
      {
	std::ostringstream s;
	s << 'X' << i;   
	const std::string str(s.str());
   
	vita::sr::variable *const x = new vita::sr::variable(str);
	vars.push_back(x);
	env.insert(x);
      }

      return true;
    }
    
    return false;
  }

  /**
   * load_symbols
   * \param sf[in]
   * \return
   */
  std::string
  problem::load_symbols(const std::string &sf)
  {
    std::string symbols;

    // Set up the symbols (variables have already been prepared).
    if (!sf.empty()) // Default functions.
    {
      std::ifstream from(sf.c_str());
      if (!from)
	return "";

      std::string name;
      while (from >> name)
      {
	symbols += name+" ";

	std::stringstream s;
	s << name;
	double n;
	if (s >> n)
	  env.insert(new vita::sr::constant(n));
	else if (name=="number")
	  env.insert(new vita::sr::number(-128,127));
	else if (name=="abs")
	  env.insert(new vita::sr::abs());
	else if (name=="add" || name=="+")
	  env.insert(new vita::sr::add());
	//else if (name=="and" || name=="&&")
	//  env.insert(new vita::sr::bool_and());
	//else if (name=="or" || name=="||")
	//  env.insert(new vita::sr::bool_not());
	//else if (name=="not" || name=="!")
	//  env.insert(new vita::sr::bool_or());
	else if (name=="div" || name=="/")
	  env.insert(new vita::sr::div());
	else if (name=="idiv")
	  env.insert(new vita::sr::idiv());
	else if (name=="ife")
	  env.insert(new vita::sr::ife());
	else if (name=="ifl")
	  env.insert(new vita::sr::ifl());
	else if (name=="ifz")
	  env.insert(new vita::sr::ifz());
	else if (name=="ln")
	  env.insert(new vita::sr::ln());
	else if (name=="mul" || name=="*")
	  env.insert(new vita::sr::mul());
	else if (name=="mod" || name=="%")
	  env.insert(new vita::sr::mod());
	else if (name=="sub" || name=="-")
	  env.insert(new vita::sr::sub());
      }
    }

    return symbols;
  }
  
  /**
   * classes
   */
  unsigned
  problem::classes() const
  {
    return dat.classes();
  }

  /**
   * variables
   */
  unsigned
  problem::variables() const
  {
    return dat.variables();
  }

  /**
   * check
   */
  bool
  problem::check() const
  {
    return dat.check() && vars.size() == dat.variables();
  }

}  // namespace vita
