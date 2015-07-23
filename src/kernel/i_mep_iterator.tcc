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
/// \brief Iterator to scan the active genes of an individual
///
class i_mep::const_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = locus;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  /// \brief Builds an empty iterator
  ///
  /// Empty iterator is used as sentry (it is the value returned by end()).
  const_iterator() : loci_(), ind_(nullptr) {}

  /// \param[in] id an individual.
  explicit const_iterator(const i_mep &id) : ind_(&id)
  {
    loci_.insert(id.best_);
  }

  /// \return locus of the next active symbol.
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

    return *this;
  }

  /// \param[in] rhs second term of comparison.
  ///
  /// Returns `true` if iterators point to the same locus or they are both
  /// to the end.
  bool operator==(const const_iterator &rhs) const
  {
    assert(ind_ == rhs.ind_);

    return (loci_.empty() && rhs.loci_.empty()) ||
           loci_.cbegin() == rhs.loci_.cbegin();
  }

  bool operator!=(const const_iterator &rhs) const
  {
    return !(*this == rhs);
  }

  /// \return reference to the current locus of the individual.
  const_reference operator*() const
  {
    return *loci_.cbegin();
  }

  /// \return pointer to the current locus of the individual.
  const_pointer operator->() const
  {
    return &operator*();
  }

private:  // Private data members
  // A partial set of active loci to be explored.
  // We have tried with `std::vector<bool>` and `std::vector<uint8_t>` based
  // iterators without measuring significant speed differences.
  std::set<value_type> loci_;

  // A pointer to the individual we are iterating on.
  const i_mep *ind_;
};

#endif  // Include guard
