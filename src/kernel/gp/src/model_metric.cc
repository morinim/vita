/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/model_metric.h"

namespace vita
{

///
/// \param[in] l the model whose accuracy we are evaluating
/// \param[in] d a dataset
/// \return      the accuracy
///
double accuracy_metric::operator()(const core_reg_lambda_f *l,
                                   const dataframe &d) const
{
  Expects(!d.classes());
  Expects(d.begin() != d.end());

  std::uintmax_t ok(0), total_nr(0);

  for (const auto &example : d)
  {
    if (const auto res = (*l)(example);
        has_value(res) && issmall(lexical_cast<D_DOUBLE>(res)
                                  - label_as<D_DOUBLE>(example)))
      ++ok;

    ++total_nr;
  }

  Ensures(total_nr);
  return static_cast<double>(ok) / static_cast<double>(total_nr);
}

///
/// \param[in] l the model whose accuracy we are evaluating
/// \param[in] d a dataset
/// \return      the accuracy
///
double accuracy_metric::operator()(const core_class_lambda_f *l,
                                   const dataframe &d) const
{
  Expects(d.classes());
  Expects(d.begin() != d.end());

  std::uintmax_t ok(0), total_nr(0);

  for (const auto &example : d)
  {
    if (l->tag(example).label == label(example))
      ++ok;

    ++total_nr;
  }

  Ensures(total_nr);
  return static_cast<double>(ok) / static_cast<double>(total_nr);
}

}  // namespace vita
