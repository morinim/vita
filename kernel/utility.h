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

#if !defined(VITA_UTILITY_H)
#define      VITA_UTILITY_H

#include <algorithm>

#include "kernel/vita.h"

namespace vita
{
  ///
  /// An implementation of make_unique() as proposed by Herb Sutter in
  /// GotW #102.
  ///
  template<typename T, typename ...Args>
  std::unique_ptr<T> make_unique(Args&& ...args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  ///
  /// \return an std::array filled with value \a v.
  ///
  /// This function is used to initialize array in member initialization list
  /// of a constructor (so to be compliant with Effective C++).
  /// To be efficient the compiler need to perform the RVO optimization / copy
  /// elision.
  ///
  /// \see
  /// <http://stackoverflow.com/questions/21993780/fill-stdarray-in-the-member-initialization-list>
  ///
  template<class T, unsigned N> std::array<T, N> make_array(T v)
  {
    std::array<T, N> temp;
    temp.fill(v);
    return temp;
  }

  ///
  /// \return \c true if v is less than epsilon-tolerance.
  ///
  /// \note
  /// \a epsilon is the smallest T-value that can be added to \c 1.0 without
  /// getting \c 1.0 back. Note that this is a much larger value than
  /// \c DBL_MIN.
  ///
  template<class T> inline bool issmall(T v)
  {
    static constexpr auto e(std::numeric_limits<T>::epsilon());

    return std::abs(v) < 2.0 * e;
  }

  ///
  /// \param[in] container a STL container.
  /// \param[in] pred a unary predicate.
  ///
  /// A shortcut for the well known C++ erase-remove idiom.
  ///
  template<class T, class Pred> void erase_if(T &container, Pred pred)
  {
    container.erase(std::remove_if(container.begin(),
                                   container.end(),
                                   pred),
                    container.end());
  }

  ///
  /// \param[in] s a string
  /// \return the content of string \a s converted in an object of type \a T.
  ///
  /// Reduced version of boost::lexical_cast.
  ///
  template<class T> T lexical_cast(const std::string &s)
  { return std::stoi(s); }
  template<> inline double lexical_cast(const std::string &s)
  { return std::stod(s); }
  template<> inline std::string lexical_cast(const std::string &s)
  { return s; }

  ///
  /// \brief A RAII class to restore the state of a stream to its original
  ///        state
  ///
  /// \a iomanip manipulators are "sticky" (except \c setw which only affects
  /// the next insertion). Often we need a way to apply an arbitrary number of
  /// manipulators to a stream and revert the state to whatever it was
  /// before.
  ///
  /// \note
  /// An alternative approach, which is also exception-safe, is to shuffle
  /// everything into a temporary \a stringstream and finally put that on
  /// the real stream (which has never changed its flags at all).
  /// Of course this is a little less performant.
  ///
  class ios_flag_saver
  {
  public:
    explicit ios_flag_saver(std::ios &s)
      : s_(s), flags_(s.flags()), precision_(s.precision()), width_(s.width())
    {}

    ~ios_flag_saver()
    {
      s_.flags(flags_);
      s_.precision(precision_);
      s_.width(width_);
    }

    DISALLOW_COPY_AND_ASSIGN(ios_flag_saver);

  private:
    std::ios &s_;
    std::ios::fmtflags flags_;
    std::streamsize precision_;
    std::streamsize width_;
  };
#define SAVE_FLAGS(s) ios_flag_saver save ## __LINE__(s)

  ///
  /// A single-pass output iterator that writes successive objects of type T
  /// into the std::basic_ostream object for which it was constructed, using
  /// operator<<. Optional delimiter string is written to the output stream
  /// after the SECOND write operation.
  ///
  /// Same interface as an std::ostream_iterator.
  ///
  /// Lifted from Jerry Coffin's prefix_ostream_iterator.
  ///
  template <class T, class C = char, class traits = std::char_traits<C>>
  class infix_iterator : public std::iterator<std::output_iterator_tag, void,
                                              void, void, void>
  {
  public:
    using char_type = C;
    using traits_type = traits;
    using ostream_type = std::basic_ostream<C, traits>;

    explicit infix_iterator(ostream_type &s, C const *d = nullptr)
      : os_(&s), delimiter_(d), first_elem_(true)
    {}

    infix_iterator<T, C, traits> &operator=(const T &item)
    {
      // Here's the only real change from ostream_iterator:
      // normally, the '*os << item;' would come before the 'if'.
      if (!first_elem_ && delimiter_)
        *os_ << delimiter_;
      *os_ << item;
      first_elem_ = false;
      return *this;
    }

    infix_iterator<T, C, traits> &operator*() { return *this; }
    infix_iterator<T, C, traits> &operator++() { return *this; }
    infix_iterator<T, C, traits> &operator++(int) { return *this; }

  private:
    std::basic_ostream<C, traits> *os_;
    C const *delimiter_;
    bool first_elem_;
  };
}  // namespace vita

#endif  // Include guard
