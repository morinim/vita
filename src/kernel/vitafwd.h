/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2014-2022 EOS di Manlio Morini.
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
class data;

class i_mep;
class i_ga;
template<class T> class team;

template<class T> class interpreter;

template<class T> class evaluator;
template<class T, class DS> class src_evaluator;

class basic_src_lambda_f;

class core_reg_lambda_f;
class core_class_lambda_f;
class model_metric;

template<class T,
         template<class> class SS,
         template<class> class CS,
         template<class> class RS> class evolution_strategy;
template<class T, template<class> class CS> class basic_alps_es;
template<class T> class std_es;

template<class T, template<class> class ES> class src_search;

template<class T> class summary;
}  // namespace vita

#endif  // include guard
