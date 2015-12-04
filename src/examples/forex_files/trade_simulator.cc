/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2015 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "trade_simulator.h"

void trade_simulator::clear_status()
{
  order_ = order();
  spread_ = 0.0010;
  balance_ = 0.0;
  cur_bar_ = 1;
  orders_history_total_ = 0;
}

// A lot is the basic trade size. It translates to 100000 units of the base
// currency (the currency on the left of the currancy pair).
// Also used are mini lot (10000 units) and micro lots (1000 units).
bool trade_simulator::order_send(int type, double lots)
{
  assert(type == order::buy || type == order::sell);
  assert(lots >= 0.01);

  double open_price;

  const double amount(lots * 100000.0);
  if (type == order::buy)  // buying base currency
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
  assert(order_.type() != order::na);

  if (order_.type() == order::buy)
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

double trade_simulator::order_profit() const
{
  switch (order_.type())
  {
  case order::buy:   return order_amount() * (bid() - order_open_price());
  case order::sell:  return order_amount() * (order_open_price() - ask());
  default:           return 0.0;
  }
}

template<class T>
double trade_simulator::run(const T &prg)
{
  clear_status();

  const auto bars(td_.bars(0) - 1);

  while (cur_bar_ < bars)
  {
    const auto type(order_.type());

    if (type == order::na)
    {
      vita::interpreter<vita::i_mep> i_buy(&prg[0]);
      vita::interpreter<vita::i_mep> i_sell(&prg[1]);

      auto a_buy(i_buy.run());
      const bool buy(!a_buy.empty() && vita::any_cast<bool>(a_buy));

      auto a_sell(i_sell.run());
      const bool sell(!a_sell.empty() && vita::any_cast<bool>(a_sell));

      if (buy && !sell)
        order_send(order::buy, 0.01);
      else if (sell && !buy)
        order_send(order::sell, 0.01);
    }
    else   // short/long position
    {
      const double close_level(10.0);

      if (std::fabs(order_profit()) > close_level)
        order_close();
    }

    inc_bar();

    /*
    if (cur_bar_ % 100000 == 0)
      std::cout << "  Bar " << cur_bar_ << "  balance " << balance_
                << "          \r" << std::flush;
    */

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

  if (order_type() != order::na)
    order_close();

  //std::cout << "BALANCE: " << balance_ << " (" << orders_history_total()
  //          << " operations)\n";
  return balance_;
}
