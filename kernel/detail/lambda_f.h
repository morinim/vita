/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_DETAIL_LAMBDA_F_H)
#define      VITA_DETAIL_LAMBDA_F_H

namespace vita { namespace detail
{
  template<bool> struct is_true : std::false_type {};
  template<> struct is_true<true> : std::true_type {};

  // ***********************************************************************
  // *  core_reg_lambda_f                                                  *
  // ***********************************************************************

  /// This is the general template. The last parameter is used for
  /// disambiguation since we need three specializations that, without
  /// the third parameter, overlap (see below).
  /// This technique is called tag dispatching by type (Barend Gehrels).
  template<class T, bool S, bool = is_team<T>::value> class core_reg_lambda_f;

  // ********* First specialization (individual stored inside) *********
  template<class T>
  class core_reg_lambda_f<T, true, false>
  {
  public:
    explicit core_reg_lambda_f(const T &ind) : ind_(ind), int_(ind_)
    { assert(debug()); }

    bool debug() const { return ind_.debug() && int_.debug(); }

    // Without this copy assignment operator the object cannot by copied since
    // the default assignment is deleted due to the non-copyable int_
    // data member.
    // We just need to copy the ind_ data member, the int_ interpreter only
    // contains a reference to ind_.
    core_reg_lambda_f<T, true, false> &operator=(
      const core_reg_lambda_f<T, true, false> &other)
    {
      if (this != &other)
        ind_ = other.ind_;

      return *this;
    }

  public:   // Public data members
    T ind_;
    mutable src_interpreter<T> int_;

  public:   // Serialization
    bool load(std::istream &in)
    {
      unsigned n;
      if (!(in >> n) || n != 1)
        return false;

      return ind_.load(in);
    }

    bool save(std::ostream &out) const
    {
      out << 1 << std::endl;
      return ind_.save(out);
    }
  };

  // ********* Second specialization (individual not stored) *********
  template<class T>
  class core_reg_lambda_f<T, false, false>
  {
  public:
    explicit core_reg_lambda_f(const T &ind) : int_(ind)
    { assert(debug()); }

    bool debug() const { return int_.debug(); }

  public:   // Public data members
    mutable src_interpreter<T> int_;

  public:   // Serialization
    constexpr bool load(std::istream &) const { return false; }
    constexpr bool save(std::ostream &) const { return false; }
  };

  // ********* Third specialization (teams) *********
  template<class T, bool S>
  class core_reg_lambda_f<team<T>, S, true>
  {
  public:
    explicit core_reg_lambda_f(const team<T> &t)
    {
      team_.reserve(t.individuals());
      for (const auto &ind : t)
        team_.emplace_back(ind);

      assert(debug());
    }

    bool debug() const
    {
      for (const auto &lambda : team_)
        if (!lambda.debug())
          return false;

      return true;
    }

  public:   // Public data members
    std::vector<core_reg_lambda_f<T, S>> team_;

  public:   // Serialization
    /// Load is atomic: if it doesn't succeed this object isn't modified; if
    /// it succeeds the team if replaced with a new team (eventually with a
    /// different size) loaded from the input stream.
    bool load(std::istream &i)
    {
      unsigned n;
      if (!(i >> n) || !n)
        return false;

      decltype(team_) v;
      v.reserve(n);

      assert(team_.size());
      const environment &env(team_[0].ind_.env());
      const symbol_set &sset(team_[0].ind_.sset());

      for (unsigned j(0); j < n; ++j)
      {
        v.emplace_back(T(env, sset));

        if (!v.back().load(i))
          return false;
      }

      team_ = v;
      return true;
    }

    bool save(std::ostream &o) const
    {
      o << team_.size() << std::endl;
      if (!o.good())
        return false;

      for (const auto &l : team_)
        if (!l.save(o))
          return false;

      return o.good();
    }
  };

  // ***********************************************************************
  // *  class_names                                                        *
  // ***********************************************************************

  ///
  /// \brief A class to (optionally) store a vector of names
  ///
  /// \tparam N if \c true stores the names otherwise keeps the memory free.
  ///
  /// This class is used for optimize the storage of vita::class_lambda_f. The
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
  public:   // Serialization
    constexpr bool load(std::istream &) const { return true; }
    constexpr bool save(std::ostream &) const { return true; }

  protected:
    /// Without names... there isn't anything to do.
    explicit class_names(const data &) {}

    std::string string(const any &) const;
  };

  template<>
  class class_names<true>
  {
  public:   // Serialization
    bool load(std::istream &);
    bool save(std::ostream &) const;

  protected:
    explicit class_names(const data &);

    std::string string(const any &) const;

  private:
    std::vector<std::string> names_;
  };

  ///
  /// \param[in] d the training set.
  ///
  inline class_names<true>::class_names(const data &d) : names_(d.classes())
  {
    const auto classes(d.classes());
    assert(classes > 1);

    for (auto i(decltype(classes){0}); i < classes; ++i)
      names_[i] = d.class_name(i);
  }

  ///
  /// \param[in] in input stream.
  /// \return true on success.
  ///
  /// Loads the names from input stream \a in.
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
    {
      getline(in, line);

      if (in.fail())
        return false;
    }

    names_ = v;

    return true;
  }

  ///
  /// \param[out] o output stream.
  /// \return true on success.
  ///
  /// Saves the names (one per line, end of line character is '\n'). The first
  /// line contains the number of names.
  ///
  inline bool class_names<true>::save(std::ostream &o) const
  {
    o << names_.size() << std::endl;
    if (!o.good())
      return false;

    for (const auto &n : names_)
    {
      o << n << std::endl;
      if (!o.good())
        return false;
    }

    return o.good();
  }

  ///
  /// \param[in] a id of a class.
  /// \return the name of class \a a.
  ///
  template<bool N>
  std::string class_names<N>::string(const any &a) const
  {
    return std::to_string(anycast<class_t>(a));
  }

  ///
  /// \param[in] a id of a class.
  /// \return the name of class \a a.
  ///
  inline std::string class_names<true>::string(const any &a) const
  {
    // Specialized class templates result in a normal class with a funny name
    // and not a template. When we specialize class_names<true>, it is no
    // longer a template and the implementation of its class members are not
    // template specializations. So we haven't to put template<> at the
    // beginning.

    return names_[anycast<class_t>(a)];
  }
}}  // namespace vita::detail

#endif  // VITA_DETAIL_LAMBDA_F_H
