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
  // This is the general template. The last parameter is used for
  // disambiguation since we need three specializations that, without
  // the third parameter are, overlap (see below).
  template<class T, bool S, bool = is_team<T>::value> class core_reg_lambda_f;

  // ********* First specialization (individual stored inside) *********
  template<class T>
  class core_reg_lambda_f<T, true, false>
  {
  public:
    explicit core_reg_lambda_f(const T &ind) : ind_(ind), int_(ind_)
    { assert(debug()); }

    bool debug() const { return ind_.debug() && int_.debug(); }

  public:    // Public data members
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
    constexpr bool load(std::istream &) { return false; }
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
    bool load(std::istream &i)
    {
      unsigned n;
      if (!(i >> n) || !n)
        return false;

      decltype(team_) v;
      v.reserve(n);

      const environment &env(team_[0].ind_.env());
      const symbol_set &sset(team_[0].ind_.sset());

      for (unsigned j(0); j < n; ++j)
      {
        v.emplace_back(T(env, sset));

        if (!v[j].load(i))
          return false;
      }

      team_ = std::move(v);
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
  protected:
    /// Without names... there isn't anything to do.
    explicit class_names(const data &) {}

    std::string string(const any &) const;
  };

  template<>
  class class_names<true>
  {
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
  /// \param[in] a id of a class.
  /// \return the name of class \a a.
  ///
  template<bool N>
  std::string class_names<N>::string(const any &a) const
  {
    return std::to_string(any_cast<class_t>(a));
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

    return names_[any_cast<class_t>(a)];
  }
}}  // namespace vita::detail

#endif  // VITA_DETAIL_LAMBDA_F_H
