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
  ///
  /// \param[in] st success threashold: when fitness is greater than this value,
  ///               the datum is considered learned (matched, classified,
  ///               resolved...)
  ///
  /// New empty instance.
  ///
  src_problem::src_problem(fitness_t st) : problem(st)
  {
    clear();

    unsigned i;
    i = add_evaluator(std::make_shared<abs_evaluator>(&dat_, &vars_));
    assert(i == k_abs_evaluator);

    i = add_evaluator(std::make_shared<count_evaluator>(&dat_, &vars_));
    assert(i == k_count_evaluator);

    i = add_evaluator(std::make_shared<dyn_slot_evaluator>(&dat_, &vars_));
    assert(i == k_dyn_slot_evaluator);

    //add_evaluator(std::make_shared<gaussian_evaluator>(&dat_, &vars_));
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
  /// \return number of lines parsed.
  ///
  unsigned src_problem::load_data(const std::string &f)
  {
    dat_.clear();

    const unsigned parsed(dat_.open(f));
    if (parsed > 0)
    {
      // Sets up the variables.
      for (unsigned i(1); i < dat_.columns(); ++i)
      {
        std::string name(dat_.get_column(i).name);
        if (name.empty())
          name = "X" + boost::lexical_cast<std::string>(i);

        const category_t category(dat_.get_column(i).category_id);
        const variable_ptr x(std::make_shared<variable>(name, category));
        vars_.push_back(x);
        env.insert(x);
      }

      // Sets up the labels for nominal attributes.
      for (category_t c(0); c < dat_.categories(); ++c)
      {
        const data::category &cat(dat_.get_category(c));
        for (auto lp(cat.labels.begin()); lp != cat.labels.end(); ++lp)
        {
          const symbol_ptr label(std::make_shared<constant>(*lp, c));
          env.insert(label);
        }
      }

      set_evaluator(classes() > 1
        ? k_dyn_slot_evaluator   // classification problem
        : k_abs_evaluator);      // symbolic regression problem
    }

    return parsed;
  }

  ///
  /// Default symbol set. This is useful for simple problems (single category
  /// regression / classification).
  ///
  void src_problem::setup_default_symbols()
  {
    symbol_factory &factory(symbol_factory::instance());

    for (category_t category(0); category < dat_.categories(); ++category)
      if (compatible({category}, {"numeric"}))
      {
        env.insert(factory.make("1", vita::d_double, {category}));
        env.insert(factory.make("2", vita::d_double, {category}));
        env.insert(factory.make("3", vita::d_double, {category}));
        env.insert(factory.make("4", vita::d_double, {category}));
        env.insert(factory.make("5", vita::d_double, {category}));
        env.insert(factory.make("6", vita::d_double, {category}));
        env.insert(factory.make("7", vita::d_double, {category}));
        env.insert(factory.make("8", vita::d_double, {category}));
        env.insert(factory.make("9", vita::d_double, {category}));
        env.insert(factory.make("ABS", vita::d_double, {category}));
        env.insert(factory.make("ADD", vita::d_double, {category}));
        env.insert(factory.make("DIV", vita::d_double, {category}));
        env.insert(factory.make("LN",  vita::d_double, {category}));
        env.insert(factory.make("MUL", vita::d_double, {category}));
        env.insert(factory.make("MOD", vita::d_double, {category}));
        env.insert(factory.make("SUB", vita::d_double, {category}));
      }
  }

  ///
  /// \param[in] sf name of the file containing the symbols.
  /// \return number of parsed symbols.
  ///
  unsigned src_problem::load_symbols(const std::string &sf)
  {
    using namespace boost::property_tree;

    unsigned parsed(0);

    cvect categories(dat_.categories());
    for (unsigned i(0); i < categories.size(); ++i)
      categories[i] = i;

    ptree pt;
    read_xml(sf, pt);

#if !defined(NDEBUG)
    std::cout << std::endl << std::endl;
    for (unsigned i(0); i < dat_.categories(); ++i)
      std::cout << "Category " << i << ": " << dat_.get_category(i).name
                << " (domain " << dat_.get_category(i).domain << ")"
                << std::endl;
    std::cout << std::endl;
#endif

    symbol_factory &factory(symbol_factory::instance());

    BOOST_FOREACH(ptree::value_type s, pt.get_child("symbolset"))
      if (s.first == "symbol")
      {
        const std::string sym_name(s.second.get<std::string>("<xmlattr>.name"));
        const std::string sym_sig(s.second.get<std::string>(
                                    "<xmlattr>.signature", ""));

        if (sym_sig.empty())
        {
          BOOST_FOREACH(ptree::value_type sig, s.second)
            if (sig.first == "signature")
            {
              std::vector<std::string> args;
              BOOST_FOREACH(ptree::value_type arg, sig.second)
                if (arg.first == "arg")
                  args.push_back(arg.second.data());

              // From the list of all the sequences with repetition of
              // args.size() elements (categories)...
              const std::list<cvect> sequences(seq_with_rep(categories,
                                                            args.size()));

              // ...we choose those compatible with the xml signature of the
              // current symbol.
              for (auto i(sequences.begin()); i != sequences.end(); ++i)
                if (compatible(*i, args))
                {
                  const domain_t domain(dat_.get_category((*i)[0]).domain);
#if !defined(NDEBUG)
                  //const domain_t domain(dat_.get_category(i->back()).domain);
                  std::cout << "Domain " << domain << ": " << sym_name << '(';
                  for (unsigned j(0); j < i->size(); ++j)
                    std::cout << dat_.get_category((*i)[j]).name
                              << (j+1 == i->size() ? ")" : ", ");
                  std::cout << std::endl;
#endif
                  env.insert(factory.make(sym_name, domain, *i));
                }
            }
        }
        else  // !sym_sig.empty() => one category, uniform symbol initialization
        {
          for (category_t category(0); category < dat_.categories(); ++category)
            if (compatible({category}, {sym_sig}))
            {
              const domain_t domain(dat_.get_category(category).domain);

              const unsigned n_args(factory.args(sym_name, domain));

#if !defined(NDEBUG)
              std::cout << "Domain " << domain << ": " << sym_name << '(';
              for (unsigned j(0); j < n_args; ++j)
                std::cout << dat_.get_category(category).name
                          << (j+1 == n_args ? ")" : ", ");
              std::cout << std::endl;
#endif
              env.insert(factory.make(sym_name, domain,
                                      cvect(n_args, category)));
            }
        }

        ++parsed;
      }

    return parsed;
  }

  ///
  /// \param[in] instance a vector of categories.
  /// \param[in] pattern a mixed vector of category names and domain names.
  /// \return \c true if \a instance match \a pattern.
  ///
  /// For instance:
  /// \verbatim
  /// category_t km_h, name;
  /// compatible({km_h}, {"km/h"}) == true
  /// compatible({km_h}, {"numeric"}) == true
  /// compatible({km_h}, {"string"}) == false
  /// compatible({km_h}, {"name"}) == false
  /// compatible({name}, {"string"}) == true
  /// \endverbatim
  ///
  bool src_problem::compatible(const cvect &instance,
                               const std::vector<std::string> &pattern) const
  {
    assert(instance.size() == pattern.size());

    for (unsigned i(0); i < instance.size(); ++i)
    {
      bool generic(data::from_weka.find(pattern[i]) != data::from_weka.end());

      if (generic)  // numeric, string, integer...
      {
        if (dat_.get_category(instance[i]).domain !=
            data::from_weka.find(pattern[i])->second)
          return false;
      }
      else
      {
        if (instance[i] != dat_.get_category(pattern[i]))
          return false;
      }
    }

    return true;
  }

  ///
  /// \param[in] categories this is the "dictionary" for the sequence.
  /// \param[in] args size of the sequence.
  /// \return a list of sequences with repetition of fixed length (\a args) of
  ///         elements taken from the given set (\a categories).
  ///
  std::list<src_problem::cvect> src_problem::seq_with_rep(
    const cvect &categories,
    unsigned args)
  {
    assert(categories.size());
    assert(args);

    class swr
    {
    public:
      swr(const cvect &categories, unsigned args)
        : categories_(categories), args_(args)
      {
      }

      void operator()(unsigned level, const cvect &base, std::list<cvect> *out)
      {
        for (unsigned i(0); i < categories_.size(); ++i)
        {
          cvect current(base);
          current.push_back(categories_[i]);

          if (level+1 < args_)
            operator()(level+1, current, out);
          else
            out->push_back(current);
        }
      }

    private:
      const cvect &categories_;
      const unsigned args_;
    };

    std::list<cvect> out;
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
