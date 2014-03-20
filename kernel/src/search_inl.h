/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SRC_SEARCH_INL_H)
#define      VITA_SRC_SEARCH_INL_H

template<class T, template<class> class ES>
src_search<T, ES>::src_search(src_problem *const p)
  : search<T, ES>(p),
    p_symre(evaluator_id::rmae), p_class(evaluator_id::gaussian)
{
  if (p->data()->size() && !this->active_eva_)
    set_evaluator(p->classification() ? p_class : p_symre);
}

///
/// \param[in] id numerical id of the evaluator to be activated.
/// \param[in] msg input parameters for the evaluator constructor.
/// \return \c true if the active evaluator has been changed.
///
/// \note
/// If the evaluator \a id is not compatible with the problem type the
/// function return \c false and the active evaluator stays the same.
///
template<class T, template<class> class ES>
bool src_search<T, ES>::set_evaluator(evaluator_id id, const std::string &msg)
{
  auto &data(*this->prob_->data());

  if (data.classes() > 1)
  {
    switch (id)
    {
    case evaluator_id::bin:
      search<T, ES>::set_evaluator(make_unique<binary_evaluator<T>>(data));
      return true;

    case evaluator_id::dyn_slot:
      {
        auto x_slot(msg.empty() ? 10u : boost::lexical_cast<unsigned>(msg));
        search<T, ES>::set_evaluator(
          make_unique<dyn_slot_evaluator<T>>(data, x_slot));
      }
      return true;

    case evaluator_id::gaussian:
      search<T, ES>::set_evaluator(make_unique<gaussian_evaluator<T>>(data));
      return true;

    default:
      return false;
    }
  }
  else // Symbolic regression
  {
    switch (id)
    {
    case evaluator_id::count:
      search<T, ES>::set_evaluator(make_unique<count_evaluator<T>>(data));
      return true;

    case evaluator_id::mae:
      search<T, ES>::set_evaluator(make_unique<mae_evaluator<T>>(data));
      return true;

    case evaluator_id::rmae:
      search<T, ES>::set_evaluator(make_unique<rmae_evaluator<T>>(data));
      return true;

    case evaluator_id::mse:
      search<T, ES>::set_evaluator(make_unique<mse_evaluator<T>>(data));
      return true;

    default:
      return false;
    }
  }
}

///
/// \param[in] verbose if \c true prints error messages to \c std::cerr.
/// \return \c true if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool src_search<T, ES>::debug(bool verbose) const
{
  if (p_symre == evaluator_id::undefined)
  {
    if (verbose)
      std::cerr << "Undefined ID for preferred sym.reg. evaluator."
                << std::endl;
    return false;
  }

  if (p_class == evaluator_id::undefined)
  {
    if (verbose)
      std::cerr << "Undefined ID for preferred classification evaluator."
                << std::endl;
    return false;
  }

  return search<T, ES>::debug(verbose);
}

#endif  // Include guard
