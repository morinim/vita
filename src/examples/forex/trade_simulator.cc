/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015-2016 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "trade_simulator.h"

trade_simulator::trade_simulator(const trading_data &data)
  : td_(&data), order_(), spread_(0.0001), balance_(0.0), cur_bar_(1),
    orders_history_total_(0)
{
}

void trade_simulator::clear_status()
{
  *this = trade_simulator(*td_);
}

// A lot is the basic trade size. It translates to 100000 units of the base
// currency (the currency on the left of the currancy pair).
// Also used are mini lot (10000 units) and micro lots (1000 units).
bool trade_simulator::order_send(o_type type, double lots)
{
  assert(type == o_type::buy || type == o_type::sell);
  assert(lots >= 0.01);

  double open_price;

  const double amount(lots * 100000.0);
  if (type == o_type::buy)  // buying base currency
  {
    open_price = ask();
    balance_ -= amount * ask();
  }
  else  // selling base currency to buy counter currency
  {
    open_price = bid();
    balance_ += amount * bid();
  }

  order_ = order(type, amount, open_price, cur_bar_);
  ++orders_history_total_;

  return true;
}

bool trade_simulator::order_close()
{
  assert(order_.type() != o_type::na);

  if (order_.type() == o_type::buy)
    // Having bought base currency, we now want back counter currency.
    balance_ += order_amount() * bid();
  else
    // Having sold base currency to buy counter currency, we now want back
    // base currency.
    balance_ -= order_amount() * ask();

  order_ = order();
  return true;
}

// Returns the number of closed orders in the simulation history.
unsigned trade_simulator::orders_history_total() const
{
  return orders_history_total_;
}

// The profits and losses in the Foreign Exchange market (aka Forex) are
// determined by the currency's pips. A pip is the fourth decimal point in a
// currency pair (0.0001).
// If the current exchange rate in EURUSD (Euro-Dollar) is 1.2305, it means
// 1 Euro is worth 1.230*5* Dollars where the number 5 represents the pip in
// EURUSD.
// If EURUSD price was 1.2305 and it's now 1.2306, the pair gained 1 pip.
// To calculate the value of a pip, we must first make a note of size of trade.
// The minimum trade size in forex trading platforms are 1000 units of the
// base currency (1000 Euro) or 0.01 lots (aka microlot, we will use that as
// an example).
// So a change of one pip in EURUSD means 1000 x 0.0001 = 0.10$, i.e. the
// value of each pip in a trade size of 1 microlot is 10 cents (the value
// of a pip in a trade size of 1 lot is 10$).
// For further details see:
// <https://www.ddmarkets.com/pips-calculation-in-the-forex-market/>
double trade_simulator::order_profit() const
{
  switch (order_.type())
  {
  case o_type::buy:   return order_amount() * (bid() - order_open_price());
  case o_type::sell:  return order_amount() * (order_open_price() - ask());
  default:             return 0.0;
  }
}

bool trade_simulator::black_candle(timeframe tf, std::size_t i) const
{
  return ::black_candle(*td_, tf, i);
}

bool trade_simulator::white_candle(timeframe tf, std::size_t i) const
{
  return ::white_candle(*td_, tf, i);
}
