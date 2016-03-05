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

template<class T>
double trade_simulator::run(const T &prg)
{
  enum {id_buy, id_sell};

  clear_status();

  const auto bars(td_->bars(short_tf) - 1);

  while (cur_bar_ < bars)
  {
    const auto type(order_.type());

    if (type == o_type::na)
    {
      bool signal[2];
      for (unsigned i(0); i < 2; ++i)
      {
        vita::interpreter<vita::i_mep> i_res(&prg[i]);

        auto a(i_res.run());
        signal[i] = !a.empty() && vita::any_cast<bool>(a);
      }

      if (signal[id_buy] && !signal[id_sell])
        order_send(o_type::buy, 0.01);
      else if (signal[id_sell] && !signal[id_buy])
        order_send(o_type::sell, 0.01);
    }
    else   // short/long position
    {
      vita::interpreter<vita::i_mep> intr(&prg[type == o_type::buy ?
                                               id_sell : id_buy]);
      auto a(intr.run());
      const bool v(!a.empty() && vita::any_cast<bool>(a));

      if (v)
        order_close();
      else
      {
        const double close_level(100.0);
        if (std::fabs(order_profit()) > close_level)
          order_close();
      }
    }

    inc_bar();

    const unsigned check_at(10);
    if (cur_bar_ == bars / check_at)
    {
      if (orders_history_total() == 0)
      {
        balance_ -= 10000.0;
        break;
      }

      /*
      if (balance_ <= 0.0)
      {
        balance_ -= std::fabs(balance_) / 10.0;
        balance_ *= check_at;
        break;
      }
      */

      /*
      if (orders_history_total() *
          trading_data::ratio[1] * trading_data::ratio[2] * 7 < cur_bar_)
      {
        balance_ -= std::fabs(balance_) / 20.0;
        balance_ *= check_at;
        break;
      }
      */
    }
  }

  assert(cur_bar_ <= bars);

  if (order_type() != o_type::na)
    order_close();

  //std::cout << "BALANCE: " << balance_ << " (" << orders_history_total()
  //          << " operations)\n";
  return balance_;
}
