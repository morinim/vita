/**
 *
 *  \file src/search_inl.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2013 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(SRC_SEARCH_INL_H)
#define      SRC_SEARCH_INL_H

template<class T, template<class> class ES>
src_search<T, ES>::src_search(src_problem *const p)
  : search<T, ES>(p),
    p_symre(k_rmae_evaluator), p_class(k_gaussian_evaluator)
{
  if (p->data()->size() && !this->active_eva_)
    set_evaluator(p->classification() ? p_class : p_symre);
}

///
/// \param[in] id numerical id of the evaluator to be activated
/// \param[in] msg input parameters for the evaluator constructor.
///
template<class T, template<class> class ES>
void src_search<T, ES>::set_evaluator(evaluator_id id, const std::string &msg)
{
  switch (id)
  {
  case k_count_evaluator:
    search<T, ES>::set_evaluator(
      make_unique<count_evaluator<T>>(*this->prob_->data()));
    break;

  case k_mae_evaluator:
    search<T, ES>::set_evaluator(
      make_unique<mae_evaluator<T>>(*this->prob_->data()));
    break;

  case k_rmae_evaluator:
    search<T, ES>::set_evaluator(
      make_unique<rmae_evaluator<T>>(*this->prob_->data()));
    break;

  case k_mse_evaluator:
    search<T, ES>::set_evaluator(
    make_unique<mse_evaluator<T>>(*this->prob_->data()));
    break;

  case k_bin_evaluator:
    search<T, ES>::set_evaluator(
      make_unique<binary_evaluator<T>>(*this->prob_->data()));
    break;

  case k_dyn_slot_evaluator:
  {
    const auto x_slot(msg.empty() ? 10u : boost::lexical_cast<unsigned>(msg));
    search<T, ES>::set_evaluator(
      make_unique<dyn_slot_evaluator<T>>(*this->prob_->data(), x_slot));
    break;
  }

  case k_gaussian_evaluator:
    search<T, ES>::set_evaluator(
      make_unique<gaussian_evaluator<T>>(*this->prob_->data()));
    break;

  default:
    break;
  }
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool src_search<T, ES>::debug(bool verbose) const
{
  if (p_symre >= k_sup_evaluator)
  {
    if (verbose)
      std::cerr << "Incorrect ID for preferred sym.reg. evaluator."
                << std::endl;
    return false;
  }

  if (p_class >= k_sup_evaluator)
  {
    if (verbose)
      std::cerr << "Incorrect ID for preferred classification evaluator."
                << std::endl;
    return false;
  }

  return search<T, ES>::debug(verbose);
}

#endif  // SRC_SEARCH_INL_H
