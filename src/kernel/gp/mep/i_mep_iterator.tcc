/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2021 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_I_MEP_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_I_MEP_ITERATOR_TCC)
#define      VITA_I_MEP_ITERATOR_TCC

///
/// Iterator to scan the active genes of an individual.
///
template<bool is_const>
class i_mep::basic_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = gene;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  using ptr = std::conditional_t<is_const, const_pointer, pointer>;
  using ref = std::conditional_t<is_const, const_reference, reference>;
  using ind = std::conditional_t<is_const, const i_mep, i_mep>;

  /// Builds an empty iterator.
  ///
  /// Empty iterator is used as sentry (it's the value returned by end()).
  basic_iterator() : loci_(), ind_(nullptr) {}

  /// \param[in] id an individual
  explicit basic_iterator(ind &id) : loci_({id.best()}), ind_(&id)
  {
  }

  /// \return iterator representing the next active gene
  basic_iterator &operator++()
  {
    if (!loci_.empty())
    {
      const auto args(operator*().arguments());

      if (args.empty())
        loci_.erase(loci_.begin());
      else
      {
        auto node(loci_.extract(loci_.begin()));
        node.value() = args.front();
        loci_.insert(std::move(node));

        loci_.insert(std::next(args.begin()), args.end());
      }
    }

    return *this;
  }

  /// \param[in] rhs second term of comparison
  /// \return        `true` if iterators point to the same locus or they are
  ///                both to the end
  bool operator==(const basic_iterator &rhs) const
  {
    Ensures(!ind_ || !rhs.ind_ || ind_ == rhs.ind_);

    return (loci_.empty() && rhs.loci_.empty()) ||
           loci_.cbegin() == rhs.loci_.cbegin();
  }

  bool operator!=(const basic_iterator &rhs) const
  {
    return !(*this == rhs);
  }

  /// \return reference to the current locus of the individual
  ref operator*() const
  {
    return ind_->genome_(locus());
  }

  /// \return pointer to the current locus of the individual
  ptr operator->() const
  {
    return &operator*();
  }

  /// \return the locus of the current gene
  vita::locus locus() const
  {
    return *loci_.cbegin();
  }

private:
  // A partial set of active loci to be explored.
  // We have tried with `std::vector<bool>` and `std::vector<uint8_t>` based
  // iterators without measuring significant speed differences.
  std::set<vita::locus> loci_;

  // A pointer to the individual we are iterating on.
  ind *ind_;
};

#endif  // include guard
