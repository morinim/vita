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

#include <chrono>
#include <cstdio>
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

std::string trade_simulator::merge_path(const std::string &p1,
                                        const std::string &p2)
{
  if (p1.empty())
    return p2;
  if (p2.empty())
    return p1;

  const char sep('/');

  const auto last_p1(p1.back() == sep ? std::prev(p1.end()) : p1.end());
  const auto first_p2(p2.front() == sep ? std::next(p2.begin()) : p2.begin());

  return std::string(p1.begin(), last_p1) +
         std::string(1, sep) +
         std::string(first_p2, p2.end());
}

std::string trade_simulator::full_path(const std::string &fn) const
{
  return merge_path(working_dir_, fn);
}

///
/// \param[in] example_dir directory containing configuration files
///
trade_simulator::trade_simulator()
{
  const std::string ini(full_path("forex.xml"));

  tinyxml2::XMLDocument doc;
  if (doc.LoadFile(ini.c_str()) != tinyxml2::XML_SUCCESS)
    throw std::runtime_error("Error opening initialization file: " + ini);

  tinyxml2::XMLHandle h(&doc);
  tinyxml2::XMLHandle files(h.FirstChildElement("mtgp")
                             .FirstChildElement("files"));

  const auto coalesce = [](const tinyxml2::XMLElement *e, const char def[])
                        {
                          return e && e->GetText() ? e->GetText() : def;
                        };

  // --- Template name ---
  auto *e(files.FirstChildElement("template").ToElement());
  std::string f_ea_template(coalesce(e, "template.mq5"));
  std::ifstream from(full_path(f_ea_template));
  if (!from)
    throw std::runtime_error("Error opening EA template: " + f_ea_template);

  std::stringstream buffer;
  buffer << from.rdbuf();
  ea_template_ = buffer.str();

  ea_name_ = coalesce(files.FirstChildElement("name").ToElement(), "gpea.mq5");
  results_name_ = coalesce(files.FirstChildElement("results").ToElement(),
                           "results.txt");
  working_dir_ = coalesce(files.FirstChildElement("workingdir").ToElement(),
                          "./");
}

double trade_simulator::run(const vita::team<vita::i_mep> &prg)
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

  const auto fo(merge_path(working_dir_, ea_name_));
  const auto fo_tmp(fo + ".tmp");
  std::ofstream o(fo_tmp);
  if (!o)
    throw std::runtime_error("Error creating EA file: " + fo);
  o << ea;
  o.close();
  std::rename(fo_tmp.c_str(), fo.c_str());

  const auto fr(merge_path(working_dir_, results_name_));
  std::ifstream results(fr);
  while (!results.is_open())
  {
    sleep_for(std::chrono::milliseconds(500));
    results.open(fr);
  }

  double profit;
  if (!(results >> profit))
    throw std::runtime_error("Cannot read profit from " + fr);

  unsigned short_trades;
  if (!(results >> short_trades))
    throw std::runtime_error("Cannot read number of short trades from " + fr);

  unsigned long_trades;
  if (!(results >> long_trades))
    throw std::runtime_error("Cannot read numer of long trades from " + fr);

  results.close();
  std::remove(fr.c_str());

  const double trades(short_trades + long_trades);
  const double active_symbols(prg.active_symbols());

  return -std::exp(-profit / 10000.0)
         - std::max(100.0 - trades, 0.0)
         - std::max(40.0 - active_symbols, 0.0);
}
