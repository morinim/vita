/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FOREX_TRADE_SIMULATOR_H)
#define      FOREX_TRADE_SIMULATOR_H

#include "kernel/vita.h"

///
/// Interface to Metatrader5 software.
///
class trade_simulator
{
public:
  trade_simulator();

  vita::fitness_t run(const vita::team<vita::i_mep> &);

private:
  std::string full_path(const std::string &) const;

  std::string      ea_name_;
  std::string  ea_template_;
  std::string results_name_;
  std::string  working_dir_;
};

#endif  // include guard
