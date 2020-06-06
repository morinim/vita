/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#if !defined(FOREX_TRADE_SIMULATOR_H)
#define      FOREX_TRADE_SIMULATOR_H

#define HAS_UNCAUGHT_EXCEPTIONS 1

#include <filesystem>

#include "kernel/vita.h"
#include "third_party/date/date.h"

///
/// A temporal interval.
///
struct period
{
  period() = default;
  period(date::year_month_day d1, date::year_month_day d2)
    : start(std::min(d1, d2)), end(std::max(d1, d2))
  { Expects(!empty()); }

  bool empty() const { return start == end; }
  date::days size() const;

  date::year_month_day start;
  date::year_month_day   end;
};

///
/// Interface to Metatrader5 software.
///
class trade_simulator
{
public:
  trade_simulator();

  vita::fitness_t run(const vita::team<vita::i_mep> &);

private:
  std::filesystem::path full_path(const std::filesystem::path &) const;
  void write_ea_file(const vita::team<vita::i_mep> &) const;
  void write_ini_file(const period &) const;

  std::filesystem::path      ea_name_;
  std::filesystem::path     ini_name_;
  std::filesystem::path results_name_;
  std::filesystem::path  working_dir_;

  std::string ea_template_;

  std::string        deposit_;
  std::string execution_mode_;
  std::string          model_;
  std::string         period_;
  std::string         symbol_;

  period   training_set_;
  period validation_set_;
};

#endif  // include guard
