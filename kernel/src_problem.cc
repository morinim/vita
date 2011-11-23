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

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "kernel/src_problem.h"
#include "kernel/individual.h"
#include "kernel/src_evaluator.h"
#include "kernel/primitive/factory.h"

namespace vita
{
# pragma GCC diagnostic ignored "-Wformat"
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
      for (unsigned i(0); i < dat_.columns(); ++i)
        if (!dat_.get_column(i).output)
        {
          std::string name(dat_.get_column(i).name);
          if (name.empty())
            name = "X" + boost::lexical_cast<std::string>(i);

          const category_t category(dat_.get_column(i).category_id);
          variable_ptr x(new variable(name, category));
          vars_.push_back(x);
          env.insert(x);
        }

      set_evaluator(classes() > 1
        ? 1     // Symbolic regression problem
        : 0);   // Classification problem
    }

    return parsed;
  }

# pragma GCC diagnostic ignored "-Wtype-limits"
  ///
  /// \param[in] sf name of the file containing the symbols.
  /// \return number of parsed symbols.
  ///
  unsigned src_problem::load_symbols(const std::string &sf)
  {
    using namespace boost::property_tree;

    std::string symbols;

    unsigned parsed(0);

    ptree pt;
    read_xml(sf, pt);

#if !defined(NDEBUG)
    std::cout << std::endl;
    for (unsigned i(0); i < dat_.categories(); ++i)
      std::cout << i << " " << dat_.get_category(i).name << " "
                << dat_.get_category(i).domain << std::endl;
#endif

    BOOST_FOREACH(ptree::value_type sc,
                  pt.get_child("symbolset.categories"))
      if (sc.first == "category")
      {
        const std::string xml_category(sc.second.get("<xmlattr>.name",
                                                      "numeric"));
        const category_t base_category(dat_.get_category(xml_category));
        const domain_t domain(dat_.get_category(base_category).domain);

        for (auto cat(sc.second.begin()); cat != sc.second.end(); ++cat)
          if (cat->first == "symbol")
          {
            const std::string name(cat->second.data());
            const unsigned args(symbol_factory::instance().args(name, domain));

#if !defined(NDEBUG)
            std::cout << std::endl
                      << "Symbol: " << name << std::endl
                      << "XML category: " << xml_category
                      << "   Base category: " << base_category
                      << "   Domain: " << domain
                      << "   Arg: " << args << std::endl;
#endif

            std::vector<category_t> categories(dat_.categories());
            for (unsigned i(0); i < dat_.categories(); ++i)
              categories[i] = i;

            std::list<std::vector<category_t>> sequences(
              seq_with_rep(categories, args));

            for (auto i(sequences.begin()); i != sequences.end(); ++i)
              if ((*i)[0] == base_category)
              {
#if !defined(NDEBUG)
                std::cout << "(";
                for (unsigned j(0); j < i->size(); ++j)
                  std::cout << (*i)[j] << (j+1==i->size() ? ')' : ',');
                std::cout << std::endl;
#endif
                env.insert(symbol_factory::instance().make(
                             name,
                             domain,
                             *i));
              }

            ++parsed;
          }
      }

    return parsed;
  }

  ///
  /// \param[in] categories this is the "dictionary" for the sequence.
  /// \param[in] args size of the sequence.
  /// \return a list of sequences with repetition of fixed length (\a args) of
  ///         elements taken from the given set (\a categories).
  ///
  std::list<std::vector<category_t>> src_problem::seq_with_rep(
    const std::vector<category_t> &categories, unsigned args)
  {
    assert(categories.size());
    assert(args);

    class swr
    {
    public:
      swr(const std::vector<category_t> &categories, unsigned args)
        : categories_(categories), args_(args)
      {
      }

      void operator()(unsigned level,
                      const std::vector<category_t> &base,
                      std::list<std::vector<category_t>> *out)
      {
        for (unsigned i(0); i < categories_.size(); ++i)
        {
          std::vector<category_t> current(base);
          current.push_back(categories_[i]);

          if (level+1 < args_)
            operator()(level+1, current, out);
          else
            out->push_back(current);
        }
      }

    private:
      const std::vector<category_t> &categories_;
      const unsigned args_;
    };

    std::list<std::vector<category_t>> out;
    swr(categories, args)(0, {}, &out);
    return out;
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
