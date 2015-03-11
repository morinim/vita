/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_SEARCH_H)
#  error "Don't include this file directly, include the specific .h instead"
#endif

#if !defined(VITA_SEARCH_TCC)
#define      VITA_SEARCH_TCC

///
/// \param[in] p the problem we're working on. The lifetime of `p` must exceed
///              the lifetime of `this` class.
///
template<class T, template<class> class ES>
search<T, ES>::search(problem &p) : active_eva_(nullptr), env_(p.env), prob_(p)
{
  assert(debug(true));
}

template<class T, template<class> class ES>
fitness_t search<T, ES>::fitness(const T &ind)
{
  return (*active_eva_)(ind);
}

///
/// \param[in] n number of runs.
/// \return best individual found.
///
template<class T, template<class> class ES>
T search<T, ES>::run(unsigned n)
{
  return run_nvi(n);
}

///
/// \param[in] s an up to date run summary.
/// \return `true` when a run should be interrupted.
///
template<class T, template<class> class ES>
bool search<T, ES>::stop_condition(const summary<T> &s) const
{
  // We use an accelerated stop condition when
  // * all the individuals have the same fitness
  // * after env_.g_without_improvement generations the situation doesn't
  //   change.
  assert(env_.g_without_improvement);
  if (s.gen - s.last_imp > env_.g_without_improvement &&
      issmall(s.az.fit_dist().variance()))
    return true;

  return false;
}

///
/// \param[in] ind individual to be transformed in a lambda function.
/// \return the lambda function associated with `ind` (`nullptr` in case of
///         errors).
///
/// The lambda function depends on the active evaluator.
///
template<class T, template<class> class ES>
std::unique_ptr<lambda_f<T>> search<T, ES>::lambdify(const T &ind)
{
  return active_eva_->lambdify(ind);
}

///
/// \param[in] e the evaluator that should be set as active.
///
template<class T, template<class> class ES>
void search<T, ES>::set_evaluator(std::unique_ptr<evaluator<T>> e)
{
  if (env_.ttable_size)
    active_eva_ = vita::make_unique<evaluator_proxy<T>>(std::move(e),
                                                        env_.ttable_size);
  else
    active_eva_ = std::move(e);
}

///
/// \param[in] verbose if `true` prints error messages to `std::cerr`.
/// \return `true` if the object passes the internal consistency check.
///
template<class T, template<class> class ES>
bool search<T, ES>::debug(bool verbose) const
{
  if (!env_.debug(verbose, false))
    return false;

  return debug_nvi(verbose);
}
#endif  // Include guard
