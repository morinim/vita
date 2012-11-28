/**
 *
 *  \file src_interpreter.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2012 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "src_interpreter.h"

namespace vita
{
  ///
  /// \param[in] ex values to be stored in the input variables.
  ///
  void src_interpreter::load_vars(const data::example &ex)
  {
    assert(ex.input.size() == variables_->size());

    for (size_t i(0); i < variables_->size(); ++i)
      (*variables_)[i]->val = ex.input[i];
  }
}  // Namespace vita
