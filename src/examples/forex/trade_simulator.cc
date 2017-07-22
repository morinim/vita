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

#include <stdexcept>

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
  return merge_path(config_dir_, fn);
}

///
/// \param[in] example_dir directory containing configuration files
///
trade_simulator::trade_simulator(const std::string &config_dir)
  : config_dir_(config_dir)
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
  std::string f_ea_template(coalesce(e, "template1.mq5"));
  std::ifstream from(full_path(f_ea_template));
  if (!from)
    throw std::runtime_error("Error opening EA template: " + f_ea_template);

  std::stringstream buffer;
  buffer << from.rdbuf();
  ea_template_ = buffer.str();

  driver_  = coalesce(files.FirstChildElement("driver").ToElement(),
                      "vboxdriver.py");
  ea_name_ = coalesce(files.FirstChildElement("name").ToElement(), "gpea.mq5");
  results_name_ = coalesce(files.FirstChildElement("results").ToElement(),
                           "results.txt");
  tmp_dir_ = coalesce(files.FirstChildElement("tmpdir").ToElement(), "/tmp/");
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
                     "bool sell_pattern() {return false; }",
                     "bool sell_pattern() {return " + ss[1].str() + ";}");

  const auto fo(merge_path(tmp_dir_, ea_name_));
  std::ofstream o(fo);
  if (!o)
    throw std::runtime_error("Error creating EA file: " + fo);
  o << ea;
  o.close();

  static std::string init(" --init");
  const std::string driver(full_path(driver_) + init);
  init = "";

  std::system(driver.c_str());

  const auto fr(merge_path(tmp_dir_, results_name_));
  std::ifstream results(fr);
  if (!results)
    throw std::runtime_error("Error opening results file: " + fr);

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

  const double trades(short_trades + long_trades);
  const double active_symbols(prg.active_symbols());

  return -std::exp(-profit / 10000.0)
         - std::max(100.0 - trades, 0.0)
         - std::max(40.0 - active_symbols, 0.0);
}
