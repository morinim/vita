/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2011-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_PROBLEM_H)
#define      VITA_PROBLEM_H

#include "kernel/environment.h"

namespace vita
{
///
/// Aggregates the problem-related data needed by an evolutionary program.
///
class problem
{
public:
  problem();

  template<class S, class ...Args> symbol *insert(Args &&...);

  virtual bool is_valid() const;

  // What a horror! Public data members... please read the coding style
  // document for project Vita.
  environment env;
  symbol_set sset;
};

///
/// Adds a symbol to the internal symbol set.
///
/// \tparam    S    symbol to be added
/// \param[in] args arguments used to build `S`
/// \return         a raw pointer to the symbol just added (or `nullptr` in
///                 case of error)
///
template<class S, class ...Args> symbol *problem::insert(Args &&... args)
{
  return sset.insert(std::make_unique<S>(std::forward<Args>(args)...));
}

}  // namespace vita

#endif  // include guard
