/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2019 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_POPULATION_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_POPULATION_ITERATOR_TCC)
#define      VITA_POPULATION_ITERATOR_TCC

///
/// Iterator for a population.
///
/// `population<T>::base_iterator` / `population<T>::begin()` /
/// `population<T>::end()` are general and clear, so they should be the
/// preferred way to scan / perform an action over every individual of a
/// population.
///
/// \remark
/// For performance critical code accessing individuals via the `operator[]`
/// could give better results.
///
template<class T>
template<bool is_const>
class population<T>::base_iterator
{
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = T;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;

  using ptr = std::conditional_t<is_const, const_pointer, pointer>;
  using ref = std::conditional_t<is_const, const_reference, reference>;
  using pop = std::conditional_t<is_const, const population, population>;

  /// \param[in] p     a population
  /// \param[in] begin `false` for the `end()` iterator
  base_iterator(pop &p, bool begin)
    : pop_(&p), layer_(begin ? 0 : p.layers()), index_(0)
  {
  }

  /// Prefix increment operator.
  /// \return iterator to the next individual
  /// \warning Advancing past the `end()` iterator results in undefined
  ///          behaviour.
  base_iterator &operator++()
  {
    if (++index_ >= pop_->individuals(layer_))
    {
      index_ = 0;

      do  // skipping empty layers
        ++layer_;
      while (layer_ < pop_->layers() && !pop_->individuals(layer_));
    }

    assert((layer_ < pop_->layers() && index_ < pop_->individuals(layer_)) ||
           (layer_ == pop_->layers() && index_ == 0));

    return *this;
  }

  /// Postfix increment operator.
  /// \return iterator to the current individual
  base_iterator operator++(int)
  {
    base_iterator tmp(*this);
    operator++();
    return tmp;
  }

  /// \param[in] rhs second term of comparison
  /// \return        `true` if iterators point to correspondant individuals
  bool operator==(const base_iterator &rhs) const
  {
    return pop_ == rhs.pop_ && layer_ == rhs.layer_ && index_ == rhs.index_;
  }

  bool operator!=(const base_iterator &rhs) const
  {
    return !(*this == rhs);
  }

  unsigned layer() const
  {
    return layer_;
  }

  /// \return reference to the current individual
  ref operator*() const
  {
    return pop_->operator[]({layer_, index_});
  }

  /// \return pointer to the current individual
  ptr operator->() const
  {
    return &operator*();
  }

  friend std::ostream &operator<<(std::ostream &out, const base_iterator &i)
  {
    out << '[' << i.layer_ << ',' << i.index_ << ']';
    return out;
  }

private:
  std::conditional_t<is_const, const population *, population *> pop_;

  unsigned layer_;
  unsigned index_;
};

#endif  // include guard
