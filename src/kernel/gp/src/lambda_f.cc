/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2019-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/gp/src/lambda_f.h"

namespace vita::serialize
{

///
/// Saves a lambda function on persistent storage.
///
/// \param[in] out output stream
/// \param[in] l   lambda function
/// \return        `true` on success
///
bool save(std::ostream &out, const basic_src_lambda_f *l)
{
  out << l->serialize_id() << '\n';;
  return l->save(out);
}

bool save(std::ostream &out, const basic_src_lambda_f &l)
{
  return save(out, &l);
}

bool save(std::ostream &out, const std::unique_ptr<basic_src_lambda_f> &l)
{
  return save(out, l.get());
}

namespace lambda::detail
{

std::map<std::string, build_func> factory_;

}  // namespace lambda::detail

}  // namespace vita::serialize
