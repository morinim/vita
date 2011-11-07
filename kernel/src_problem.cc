/**
 *
 *  \file src_problem.cc
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

#include <fstream>

#include "kernel/src_problem.h"
#include "kernel/individual.h"
#include "kernel/src_evaluator.h"
#include "kernel/primitive/factory.h"

namespace vita
{
  ///
  /// \param[in] st success threashold: when fitness is greater than this value,
  ///               the the datum is considered learned (matched, classified,
  ///               resolved...)
  ///
  /// New empty instance.
  ///
  src_problem::src_problem(fitness_t st) : problem(st)
  {
    clear();

    evaluator_ptr e1(new abs_evaluator(&dat_, &vars_));
    add_evaluator(e1);
    //evaluator_ptr e2(new gaussian_evaluator(&dat_, &vars_));
    //add_evaluator(e2);
    evaluator_ptr e3(new dyn_slot_evaluator(&dat_, &vars_));
    add_evaluator(e3);
  }

  ///
  /// Resets the object.
  ///
  void src_problem::clear()
  {
    problem::clear();
    vars_.clear();
    dat_.clear();
  }

  ///
  /// \param[in] f name of the file containing the learning collection.
  /// \return number of lines read.
  ///
  unsigned src_problem::load_data(const std::string &f)
  {
    dat_.clear();

    const unsigned parsed(dat_.open(f));
    if (parsed > 0)
    {
      // Sets up the variables.
      for (unsigned i(0); i < dat_.variables(); ++i)
      {
        std::ostringstream s;
        s << 'X' << i;
        const std::string str(s.str());

        variable_ptr x(new variable(str));
        vars_.push_back(x);
        env.insert(x);
      }

      set_evaluator(classes() > 1
        ? 1     // Symbolic regression problem
        : 0);   // Classification problem
    }

    return parsed;
  }

  ///
  /// \param[in] sf name of the file containing the symbols.
  /// \return a space separated string containing the names of the loaded
  ///         symbols.
  ///
  std::string src_problem::load_symbols(const std::string &sf)
  {
    std::string symbols;

    // Set up the symbols (variables have already been prepared).
    if (!sf.empty())  // Default functions.
    {
      std::ifstream from(sf.c_str());
      if (!from)
        return "";

      std::string name;
      while (from >> name)
      {
        symbols += name+" ";

        symbol_ptr sp;

        std::stringstream s;
        s << name;
        double n;
        if (s >> n)
          sp.reset(new vita::dbl::constant(n));
        else if (name == "number")
          sp.reset(new vita::dbl::number(-128, 127));
        else if (name == "abs")
          sp.reset(new vita::dbl::abs());
        else if (name == "add" || name == "+")
          sp.reset(new vita::dbl::add());
        // else if (name=="and" || name=="&&")
        //   sp.reset(new vita::dbl::bool_and());
        // else if (name == "or" || name == "||")
        //   sp.reset(new vita::dbl::bool_not());
        // else if (name == "not" || name == "!")
        //   sp.reset(new vita::dbl::bool_or());
        else if (name == "div" || name == "/")
          sp.reset(new vita::dbl::div());
        else if (name == "idiv")
          sp.reset(new vita::dbl::idiv());
        else if (name == "ife")
          sp.reset(new vita::dbl::ife());
        else if (name == "ifl")
          sp.reset(new vita::dbl::ifl());
        else if (name == "ifz")
          sp.reset(new vita::dbl::ifz());
        else if (name == "ln")
          sp.reset(new vita::dbl::ln());
        else if (name == "mul" || name == "*")
          sp.reset(new vita::dbl::mul());
        else if (name == "mod" || name == "%")
          sp.reset(new vita::dbl::mod());
        else if (name == "sub" || name == "-")
          sp.reset(new vita::dbl::sub());

        env.insert(sp);
      }
    }

    return symbols;
  }

  ///
  /// \return number of categories of the problem (>= 1).
  ///
  unsigned src_problem::categories() const
  {
    return dat_.categories();
  }

  ///
  /// \return number of classes of the problem (== 0 for a symbolic regression
  ///         problem, > 1 for a classification problem).
  ///
  unsigned src_problem::classes() const
  {
    assert(dat_.classes() != 1);

    return dat_.classes();
  }

  ///
  /// \return dimension of the input vectors (i.e. the number of variable of
  ///         the problem).
  ///
  unsigned src_problem::variables() const
  {
    return dat_.variables();
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool src_problem::check() const
  {
    return problem::check() && dat_.check() && vars_.size() == dat_.variables();
  }
}  // namespace vita
