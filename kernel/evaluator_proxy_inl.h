/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_EVALUATOR_PROXY_INL_H)
#define      VITA_EVALUATOR_PROXY_INL_H

///
/// \param[in] eva pointer that lets the proxy access the real evaluator.
/// \param[in] ts 2^\a ts is the number of elements of the cache.
///
template<class T>
evaluator_proxy<T>::evaluator_proxy(std::unique_ptr<evaluator<T>> eva,
                                    unsigned ts)
  : eva_(std::move(eva)), cache_(ts)
{
  assert(eva_);
  assert(ts > 6);
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know.
/// \return the fitness of \a ind.
///
template<class T>
fitness_t evaluator_proxy<T>::operator()(const T &prg)
{
  fitness_t f;
  if (cache_.find(prg.signature(), &f))
  {
    assert(cache_.hits());

#if defined(CLONE_SCALING)
    // Before evaluating a program, we check if identical programs
    // (clones) are already present in the population.
    // When the number of clones is grater than zero, the fitness assigned to
    // the program is multiplied by a clone-scaling factor.
    // For further details see "Evolving Assembly Programs: How Games Help
    // Microprocessor Validation" - F.Corno, E.Sanchez, G.Squillero.
    const double perc(double(cache_.seen(prg.signature())) / cache_.hits());
    if (0.01 < perc && perc < 1.0)
      f -= (f * perc).abs() * 2.0;
#endif

    // Hash collision checking code can slow down the program very much.
#if !defined(NDEBUG)
    const fitness_t f1((*eva_)(prg));
    if (!almost_equal(f[0], f1[1], 0.00001))
      std::cerr << "********* COLLISION ********* [" << f << " != " << f1
                << "]" << std::endl;

    // In the above comparison we consider only the first component of the
    // fitness otherwise we can have false positives.
    // For example if the fitness is a 2D vector (where the first component
    // is the "score" on the training set and the second one is the effective
    // length of the program), then the following two programs:
    //
    // PROGRAM A                 PROGRAM B
    // ------------------        ------------------
    // [000] FADD 001 002        [000] FADD 001 001
    // [001] X1                  [001] X1
    // [002] X1
    //
    // have the same signature, the same stored "score" but distinct
    // effective size and so distinct fitnesses.
#endif
  }
  else
  {
    f = (*eva_)(prg);

    cache_.insert(prg.signature(), f);

#if !defined(NDEBUG)
    fitness_t f1;
    assert(cache_.find(prg.signature(), &f1));
    assert(f == f1);
#endif
  }

  return f;
}

///
/// \param[in] prg the program (individual/team) whose fitness we want to know.
/// \return the an approximation of the fitness of \a i.
///
template<class T>
fitness_t evaluator_proxy<T>::fast(const T &prg)
{
  return eva_->fast(prg);
}

///
/// \param[in] what what should be cleared? (all, cache, stats)
///
/// \brief Resets the evaluation cache / clear the statistics.
///
template<class T>
void evaluator_proxy<T>::clear(unsigned what)
{
  switch (what)
  {
  case evaluator_proxy<T>::all:
  case evaluator_proxy<T>::cache:
    cache_.clear();
    break;

  case evaluator_proxy<T>::stats:
#if defined(CLONE_SCALING)
    cache_.reset_seen();
#endif
    break;
  }
}

///
/// \param[in] prg a program (individual/team).
///
/// Clears the cached informations for program \a prg.
///
template<class T>
void evaluator_proxy<T>::clear(const T &prg)
{
  cache_.clear(prg.signature());
}

///
/// \return the accuracy of \a prg.
///
template<class T>
double evaluator_proxy<T>::accuracy(const T &prg) const
{
  return eva_->accuracy(prg);
}

///
/// \param[in] i a program (individual/team).
/// \return how many times we have seen the program \a prg from during the
///         current run of the evolution / the last call of the clear
///         function.
///
template<class T>
unsigned evaluator_proxy<T>::seen(const T &prg) const
{
  return cache_.seen(prg.signature());
}

///
/// \return number of cache probes / hits.
///
template<class T>
std::string evaluator_proxy<T>::info() const
{
  return
    "hits " + boost::lexical_cast<std::string>(cache_.hits()) +
    ", probes " + boost::lexical_cast<std::string>(cache_.probes()) +
    " (ratio " +
    boost::lexical_cast<std::string>(cache_.hits() * 100 / cache_.probes()) +
    "%)";
}

///
/// \param[in] prg a program (individual/team).
/// \return a pointer to the executable version of \a prg.
///
template<class T>
std::unique_ptr<lambda_f<T>> evaluator_proxy<T>::lambdify(const T &prg) const
{
  return eva_->lambdify(prg);
}

#endif  // EVALUATOR_PROXY_INL_H
