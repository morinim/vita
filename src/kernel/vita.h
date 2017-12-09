/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2016-2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  Convenience header composed to be used by Vita framework's clients.
 *  Vita isn't Boost and a master header is a simplification for the API user.
 *  Anyway to reduce compile times in large projects, clients can include only
 *  individual headers.
 *
 *  \warning **NOT TO BE USED INTERNALLY**
 */

#if !defined(VITA_VITA_H)
#define      VITA_VITA_H

#include "kernel/i_mep.h"
#include "kernel/team.h"
#include "kernel/ga/i_ga.h"
#include "kernel/ga/i_de.h"
#include "kernel/ga/primitive.h"
#include "kernel/ga/search.h"
#include "kernel/src/search.h"
#include "kernel/src/primitive/bool.h"
#include "kernel/src/primitive/int.h"
#include "kernel/src/primitive/real.h"
#include "kernel/src/primitive/string.h"
#include "utility/csv_parser.h"

#endif  // include guard
