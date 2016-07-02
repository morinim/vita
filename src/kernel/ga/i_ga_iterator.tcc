/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_GA_I_GA_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_INDIVIDUAL_GA_ITERATOR_TCC)
#define      VITA_INDIVIDUAL_GA_ITERATOR_TCC

///
/// \brief Iterator to scan the active genes of an individual
///
class i_ga::const_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t ;
  using value_type = locus;
  using pointer = const value_type *;
  using reference = const value_type &;

  ///
  /// \brief Builds an empty iterator.
  ///
  /// Empty iterator is used as sentry (it is the value returned by
  /// i_ga::end()).
  ///
  const_iterator() : sup_(0), i_(std::numeric_limits<decltype(i_)>::max()) {}

  ///
  /// \param[in] id an individual.
  ///
  explicit const_iterator(const i_ga &id) : sup_(id.parameters()), i_(0)
  {
  }

  ///
  /// \return locus of the next active symbol.
  ///
  const_iterator &operator++()
  {
    ++i_;

    if (i_ >= sup_)
      i_ = std::numeric_limits<decltype(i_)>::max();

    return *this;;
  }

  ///
  /// \param[in] rhs second term of comparison.
  ///
  /// Returns `true` if iterators point to the same locus.
  ///
  bool operator==(const const_iterator &rhs) const
  {
    return i_ == rhs.i_;
  }

  bool operator!=(const const_iterator &rhs) const
  {
    return !(*this == rhs);
  }

  ///
  /// \return the current locus of the individual.
  ///
  value_type operator*() const
  {
    return {0, i_};
  }

private:
  const category_t sup_;
  category_t i_;
};  // class i_ga::const_iterator

#endif  // Include guard
