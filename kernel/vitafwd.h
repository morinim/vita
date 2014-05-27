/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(VITA_VITAFWD_H)
#define      VITA_VITAFWD_H

namespace vita
{
  class i_mep;
  template<class T> class team;

  template<class T> class interpreter;

  template<class T,
           template<class> class SS,
           template<class> class CS,
           template<class> class RS> class evolution_strategy;
  template<class T> class alps_es;
  template<class T> class std_es;

  template<class T, template<class> class ES> class src_search;
}  // namespace vita

#endif  // Include guard
