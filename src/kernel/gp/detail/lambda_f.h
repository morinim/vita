/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_LAMBDA_F_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_DETAIL_LAMBDA_F_H)
#define      VITA_DETAIL_LAMBDA_F_H

namespace vita::detail
{

template<bool> struct is_true : std::false_type {};
template<> struct is_true<true> : std::true_type {};

// ***********************************************************************
// *  reg_lambda_f_storage                                               *
// ***********************************************************************

// This is the general template.
// The last parameter is used for disambiguation since we need three
// specializations that, without the third parameter, would overlap (see
// below). This technique is called tag dispatching by type (Barend Gehrels).
template<class T, bool S, bool = is_team<T>::value> class reg_lambda_f_storage;

// ********* First specialization (individual stored inside) *********
template<class T>
class reg_lambda_f_storage<T, true, false>
{
public:
  explicit reg_lambda_f_storage(const T &ind) : ind_(ind), int_(&ind_)
  { Ensures(is_valid()); }

  explicit reg_lambda_f_storage(const reg_lambda_f_storage &rls)
    : ind_(rls.ind_), int_(src_interpreter<T>(&ind_))
  {
    Ensures(is_valid());
  }

  reg_lambda_f_storage(std::istream &in, const symbol_set &ss)
    : int_(&ind_)
  {
    if (!ind_.load(in, ss))
      throw exception::data_format("Cannot load individual");

    int_ = src_interpreter<T>(&ind_);

    Ensures(is_valid());
  }

  reg_lambda_f_storage &operator=(const reg_lambda_f_storage &rhs)
  {
    if (this != &rhs)
    {
      ind_ = rhs.ind_;
      int_ = src_interpreter<T>(&ind_);
    }

    Ensures(is_valid());
    return *this;
  }

  template<class ...Args> value_t run(Args && ...args) const
  {
    // Consider that there are situations in which `&int_.program() != &ind_`
    // and this function will blow up.
    // It shouldn't happen: `is_valid` checks for this problematic condition,
    // `operator=` resets the pointer to the reference individual... but it's
    // not enough.
    // Consider a scenario in which a vector of objects containing
    // `reg_lambda_f_storage` needs reallocation after a `push_back`. All the
    // `int_` object are invalidated...
    //
    // `src_interpreter` is a lightweight object so we could recreate it here
    // every time or we could add a check:
    // `if (&int_.program() != &ind_)  int_ = src_interpreter<T>(&ind_);`
    // Both solutions should be checked for possible performance hits.
    return int_.run(std::forward<Args>(args)...);
  }

  bool is_valid() const
  {
    return &int_.program() == &ind_;
  }

  // Serialization.
  bool save(std::ostream &out) const { return ind_.save(out); }

private:
  T ind_;
  mutable src_interpreter<T> int_;
};

// ********* Second specialization (individual not stored) *********
template<class T>
class reg_lambda_f_storage<T, false, false>
{
public:
  explicit reg_lambda_f_storage(const T &ind) : int_(&ind)
  { Ensures(is_valid()); }

  template<class ...Args> value_t run(Args && ...args) const
  {
    return int_.run(std::forward<Args>(args)...);
  }

  bool is_valid() const { return true; }

  // Serialization
  bool save(std::ostream &out) const
  {
    return int_.program().save(out);
  }

private:
  mutable src_interpreter<T> int_;
};

// ********* Third specialization (teams) *********
template<class T, bool S>
class reg_lambda_f_storage<team<T>, S, true>
{
public:
  explicit reg_lambda_f_storage(const team<T> &t) : team_()
  {
    team_.reserve(t.individuals());
    for (const auto &ind : t)
      team_.emplace_back(ind);

    Ensures(is_valid());
  }

  reg_lambda_f_storage(std::istream &in, const symbol_set &ss) : team_()
  {
    unsigned n;
    if (!(in >> n) || !n)
      throw exception::data_format("Unknown/wrong number of programs");

    team_.reserve(n);
    for (unsigned j(0); j < n; ++j)
      team_.emplace_back(in, ss);

    Ensures(is_valid());
  }

  bool is_valid() const
  {
    return true;
  }

  // Serialization.
  bool save(std::ostream &o) const
  {
    return (o << team_.size() << '\n')
           && std::all_of(team_.begin(), team_.end(),
                          [&o](const auto &lambda) { return lambda.save(o); });
  }

public:
  std::vector<reg_lambda_f_storage<T, S>> team_;
};

// ***********************************************************************
// *  class_names                                                        *
// ***********************************************************************

///
/// A class which (optionally) stores a vector of names.
///
/// \tparam N if `true` stores the names (otherwise saves memory)
///
/// This class is used for optimize the storage of basic_class_lambda_f. The
/// strategy used is the so called 'Empty Base Class Optimization': the
/// compiler is allowed to flatten the inheritance hierarchy in a way that
/// the empty base class does not consume space (this is not true for empty
/// class data members because C++ requires data member to have non-zero size
/// to ensure object identity).
///
/// \see
/// http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Empty_Base_Optimization
///
template<bool N>
class class_names
{
public:
  // *** Serialization ***
  bool load(std::istream &) { return true; }
  bool save(std::ostream &) const { return true; }

protected:
  /// Without names... there is nothing to do.
  explicit class_names(const dataframe &) {}
  class_names() = default;

  std::string string(const value_t &) const;
};

template<>
class class_names<true>
{
public:
  // *** Serialization ***
  bool load(std::istream &);
  bool save(std::ostream &) const;

protected:
  explicit class_names(const dataframe &);
  class_names() : names_() {}

  std::string string(const value_t &) const;

private:
  std::vector<std::string> names_;
};

///
/// \param[in] d the training set.
///
inline class_names<true>::class_names(const dataframe &d) : names_(d.classes())
{
  Expects(d.classes() > 1);
  const auto classes(d.classes());

  for (auto i(decltype(classes){0}); i < classes; ++i)
    names_[i] = d.class_name(i);
}

///
/// Loads the names from storage.
///
/// \param[in] in input stream
/// \return       `true` on success
///
inline bool class_names<true>::load(std::istream &in)
{
  unsigned n;
  if (!(in >> n) || !n)
    return false;

  decltype(names_) v(n);

  // When used immediately after whitespace-delimited input, e.g. after
  //     int n; std::cin >> n;
  // getline consumes the endline character left on the input stream by
  // operator>>, and returns immediately. A common solution, before
  // switching to line-oriented input, is to ignore all leftover
  // characters on the line of input with:
  std::ws(in);

  for (auto &line : v)
    if (!getline(in, line))
      return false;

  names_ = v;

  return true;
}

///
/// Saves the names.
///
/// \param[out] o output stream
/// \return       `true` on success
///
/// One name per line, end of line character is `\n`. First line contains the
/// number of names.
///
inline bool class_names<true>::save(std::ostream &o) const
{
  if (!(o << names_.size() << '\n'))
    return false;

  for (const auto &n : names_)
    if (!(o << n << '\n'))
      return false;

  return o.good();
}

///
/// \param[in] a id of a class
/// \return      the name of class `a`
///
template<bool N>
std::string class_names<N>::string(const value_t &a) const
{
  return std::to_string(std::get<D_INT>(a));
}

///
/// \param[in] a id of a class
/// \return      the name of class `a`
///
inline std::string class_names<true>::string(const value_t &a) const
{
  // Specialized class templates result in a normal class with a funny name
  // and not a template. When we specialize class_names<true>, it is no
  // longer a template and the implementation of its class members are not
  // template specializations. So we haven't to put template<> at the
  // beginning.

  return names_[std::get<D_INT>(a)];
}

}  // namespace vita::detail

#endif  // include guard
