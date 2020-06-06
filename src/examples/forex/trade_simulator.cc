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

#include <chrono>
#include <cstdio>
#include <iostream>
#include <stdexcept>

// MinGW compiled with the win32 threading model (a common choice) at the
// moment doesn't support C++11 threading classes (see
// <https://github.com/StephanTLavavej/mingw-distro/issues/26> and
// <https://github.com/meganz/mingw-std-threads>)
#if defined(__MINGW32__)
#include <windows.h>
inline void sleep_for(std::chrono::milliseconds x)
{ Sleep(x.count()); }
#else
#include <thread>
inline void sleep_for(std::chrono::milliseconds x)
{ std::this_thread::sleep_for(x); }
#endif

#include "trade_simulator.h"
#include "utility/utility.h"
#include "tinyxml2/tinyxml2.h"

namespace
{

struct trade_results
{
  trade_results(const std::filesystem::path &filepath)
  {
    std::ifstream results(filepath);
    while (!results.is_open())
    {
      sleep_for(std::chrono::milliseconds(500));
      results.open(filepath);
    }

    const std::string fps(filepath.string());

    if (!(results >> profit))
      throw std::runtime_error("Cannot read profit from " + fps);

    if (!(results >> short_trades))
      throw std::runtime_error("Cannot read number of short trades from "
                               + fps);

    if (!(results >> long_trades))
      throw std::runtime_error("Cannot read numer of long trades from " + fps);

    if (!(results >> drawdown))
      throw std::runtime_error("Cannot read balance drawdown from " + fps);

    results.close();
  }

  double profit       = 0.0;
  double short_trades = 0.0;
  double long_trades  = 0.0;
  double drawdown     = 0.0;

  double trades() const { return short_trades + long_trades; }
};

// Always output Windows EOL (`\r\n`).
// This is useful for ini files of Windows programs.
std::ostream &wendl(std::ostream &o)
{
  o.put('\r');
  o.put('\n');
  return o;
}

double fitness(const trade_results &r, int days)
{
  // Losing EAs don't require a complex evaluation.
  if (r.profit <= 0.0)
    return r.profit;

  // The recovery factor value reflects the riskiness of the strategy: the
  // amount of money risked by the EA to make the profit it obtained.
  double recovery_factor(r.profit / (r.drawdown + 0.1));

  // We require at least 7 trades / month to consider interesting an EA.
  const auto min_trades(std::max(7 * days / 30, 10));

  // The `trades_ratio_weight`:
  // - is `1` when EA performs the minimum number of trades (`min_trades`);
  // - increases "slowly" when `trades > min_trades`;
  // - decreases "quickly" when `trades < min_trades`.
  double trades_ratio(r.trades() / min_trades);
  double trades_ratio_weight(1.0 + std::log10(trades_ratio) / 10.0);

  // A penality weight bound to the displacement between short trades and
  // long trades.
  const auto displacement(0.5 - r.short_trades / r.trades());
  const auto displacement_weight(1.0 - displacement * displacement);

  return trades_ratio_weight * displacement_weight * recovery_factor;
}

}  // unnamed namespace


std::filesystem::path trade_simulator::full_path(
  const std::filesystem::path &fn) const
{
  return working_dir_ / fn;
}

///
/// Inits the object.
///
/// Reads the configuration from `forex.xml` file.
///
trade_simulator::trade_simulator()
{
  const auto ini(full_path("forex.xml"));

  tinyxml2::XMLDocument doc;
  if (const std::string ini_s(ini.string());
      doc.LoadFile(ini_s.c_str()) != tinyxml2::XML_SUCCESS)
    throw std::runtime_error("Error opening configuration file: " + ini_s);

  tinyxml2::XMLHandle h(&doc);
  tinyxml2::XMLHandle files(h.FirstChildElement("mtgp")
                             .FirstChildElement("files"));
  tinyxml2::XMLHandle tester(h.FirstChildElement("mtgp")
                              .FirstChildElement("tester"));

  const auto value_or = [](tinyxml2::XMLHandle he, const char def[])
  {
    return he.ToElement() && he.ToElement()->GetText()
             ? he.ToElement()->GetText() : def;
  };

  auto read_file = [this](const std::filesystem::path &name)
  {
    std::ifstream from(full_path(name));
    if (!from)
      throw std::runtime_error("Error opening " + name.string());

    std::stringstream buffer;
    buffer << from.rdbuf();
    return buffer.str();
  };

  // --- Various filenames ---
  ea_name_      = value_or(files.FirstChildElement("name"), "gpea.mq5");
  ini_name_     = value_or(files.FirstChildElement("ini"), "gpea.ini");
  results_name_ = value_or(files.FirstChildElement("results"), "results.txt");
  working_dir_  = value_or(files.FirstChildElement("workingdir"), "./");

  // --- Content of buffered files ---
  ea_template_ = read_file(value_or(files.FirstChildElement("template"),
                                    "template.mq5"));

  // --- Testing parameters ---
  symbol_         = value_or(tester.FirstChildElement("symbol"), "EURUSD");
  period_         = value_or(tester.FirstChildElement("period"), "M15");
  deposit_        = value_or(tester.FirstChildElement("deposit"), "10000");
  model_          = value_or(tester.FirstChildElement("model"), "1");
  execution_mode_ = value_or(tester.FirstChildElement("execution_mode"), "65");

  std::istringstream from_date(value_or(tester.FirstChildElement("from_date"),
                                        "2016-01-01"));
  std::istringstream to_date(value_or(tester.FirstChildElement("to_date"),
                                      "2017-01-01"));
  std::istringstream forward_date(
    value_or(tester.FirstChildElement("forward_date"), ""));

  from_date >> date::parse("%F", training_set_.start);

  if (forward_date.str().empty())
    to_date >> date::parse("%F", training_set_.end);
  else
  {
    forward_date >> date::parse("%F", validation_set_.start);
    to_date >> date::parse("%F", validation_set_.end);
    training_set_.end = date::sys_days(validation_set_.start) - date::days(1);
  }

  Ensures(validation_set_.empty() == forward_date.str().empty());
  Ensures(!training_set_.empty());
}

// Writes the ini file used by Metatrader to setup the back-test.
void trade_simulator::write_ini_file(const period &p) const
{
  // Get the expert name removing the extension from EA filename.
  const auto expert(ea_name_.stem());

  // See <https://www.metatrader5.com/en/terminal/help/start_advanced/start>
  std::ofstream o(full_path(ini_name_));
  if (!o)
    throw std::runtime_error("Error creating ini file: " + ini_name_.string());

  o << "[Tester]"                          << wendl
    << "Expert="        << expert          << wendl
    << "Symbol="        << symbol_         << wendl
    << "Period="        << period_         << wendl
    << "Deposit="       << deposit_        << wendl
    << "Model="         << model_          << wendl
    << "Optimization=0"                    << wendl
    << "ExecutionMode=" << execution_mode_ << wendl;

  o << "FromDate=";
  date::to_stream(o, "%Y.%m.%d", p.start);
  o << wendl;

  o << "ToDate=";
  date::to_stream(o, "%Y.%m.%d", p.end);
  o << wendl;

  o << "ReplaceReport=1" << wendl
    << "ShutdownTerminal=1" << wendl
    << "Visual=0" << wendl;

  o.close();
}

void trade_simulator::write_ea_file(const vita::team<vita::i_mep> &prg) const
{
  std::stringstream ss[2];

  ss[0] << vita::out::mql_language << prg[0];
  ss[1] << vita::out::mql_language << prg[1];

  auto ea(ea_template_);

  ea = vita::replace(ea,
                     "bool buy_pattern() {return false;}",
                     "bool buy_pattern() {return " + ss[0].str() + ";}");

  ea = vita::replace(ea,
                     "bool sell_pattern() {return false;}",
                     "bool sell_pattern() {return " + ss[1].str() + ";}");

  const auto fo(full_path(ea_name_));
  const auto fo_tmp(full_path(ea_name_) += ".tmp");
  std::ofstream o(fo_tmp);
  if (!o)
    throw std::runtime_error("Error creating EA file: " + fo_tmp.string());
  o << ea;
  o.close();

  std::filesystem::rename(fo_tmp, fo);
}

date::days period::size() const
{
  return date::sys_days(end) - date::sys_days(start);
}

vita::fitness_t trade_simulator::run(const vita::team<vita::i_mep> &prg)
{
  write_ini_file(training_set_);
  write_ea_file(prg);

  const auto results_path(full_path(results_name_));
  trade_results r(results_path);
  std::filesystem::remove(results_path);

  double fit(fitness(r, training_set_.size().count()));

  using vita::log;
  vitaINFO << "CURRENT EA. Profit:" << r.profit
           << " Drawdown:" << r.drawdown
           << " Trades:" << r.trades()
           << " Fit:" << fit;

  return {fit, r.profit, r.drawdown, r.trades()};
}
