/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_I_MEP_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_INDIVIDUAL_MEP_ITERATOR_TCC)
#define      VITA_INDIVIDUAL_MEP_ITERATOR_TCC

///
/// \brief Iterator to scan the active genes of an \c individual
///
class i_mep::const_iterator
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
  /// i_mep::end()).
  ///
  const_iterator() : loci_(), ind_(nullptr) {}

  ///
  /// \param[in] id an individual.
  ///
  explicit const_iterator(const i_mep &id) : ind_(&id)
  {
    loci_.insert(id.best_);
  }

  ///
  /// \return locus of the next active symbol.
  ///
  const_iterator &operator++()
  {
    if (!loci_.empty())
    {
      const gene &g((*ind_)[*loci_.cbegin()]);

      const auto arity(g.sym->arity());
      for (auto j(decltype(arity){0}); j < arity; ++j)
        loci_.insert(g.arg_locus(j));

      loci_.erase(loci_.begin());
    }

    return *this;;
  }

  ///
  /// \param[in] rhs second term of comparison.
  ///
  /// Returns \c true if iterators point to the same locus or they are both
  /// to the end.
  ///
  bool operator==(const const_iterator &rhs) const
  {
    return (loci_.empty() && rhs.loci_.empty()) ||
           loci_.cbegin() == rhs.loci_.cbegin();
  }

  bool operator!=(const const_iterator &i2) const
  {
    return !(*this == i2);
  }

  ///
  /// \return reference to the current \a locus of the \a individual.
  ///
  reference operator*() const
  {
    return *loci_.cbegin();
  }

  ///
  /// \return pointer to the current \c locus of the \c individual.
  ///
  pointer operator->() const
  {
    return &(*loci_.cbegin());
  }

private:  // Private data members
  // A partial set of active loci to be explored.
  std::set<value_type> loci_;

  // A pointer to the individual we are iterating on.
  const i_mep *const ind_;
};  // class i_mep::const_iterator

#endif  // Include guard
