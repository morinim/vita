/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2013-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <any>

#if !defined(VITA_ANY_H)
#define      VITA_ANY_H

namespace vita
{

template<class T> T to(const std::any &);

}  // namespace vita

#endif  // include guard
