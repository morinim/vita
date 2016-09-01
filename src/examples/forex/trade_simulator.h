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

#include "trading_data.h"

#if !defined(FOREX_TRADE_SIMULATOR_H)
#define      FOREX_TRADE_SIMULATOR_H

enum class o_type {na = -1, buy = 0, sell};

// ****************************************************************************
// order
// ****************************************************************************
class order
{
public:
  explicit order(o_type t = o_type::na, double a = 0.0, double o = 0.0,
                 std::size_t b = 0)
    : amount_(a), open_price_(o), type_(t), bar_(b)
  {
    Expects((t == o_type::na && a <= 0.0) || (t != o_type::na && a > 0.0));
    Expects((t == o_type::na && o <= 0.0) || (t != o_type::na && o > 0.0));
    Expects((t == o_type::na && b == 0) || (t != o_type::na && b > 0));
  }

  double amount() const { return amount_; }
  double open_price() const { return open_price_; }
  o_type type() const { return type_; }
  std::size_t bar() const { return bar_; }

private:
  double amount_;
  double open_price_;
  o_type type_;
  std::size_t bar_;
};

// ****************************************************************************
// trade_simulator
// ****************************************************************************
class trade_simulator
{
public:
  explicit trade_simulator(const trading_data &);

  // The balance, expressed in counter currency, not including the floating
  // profit/loss from any open trades.
  // Transactions in forex markets are quoted as pairs because traders buy one
  // currency and sell another. For example, EUR/USD is presented as the price
  // of how many US dollars should be spent to buy a Euro.
  // The **base currency** is located to the left and the **counter currency**
  // is located to the right.
  double account_balance() const { return balance_; }

  // If the current bid price for the EUR/USD currency pair is 1.5760 this
  // means that you can sell 1 Euro and get 1.5760$.
  double bid() const { return td_->open(short_tf, cur_bar_); }

  // If the current ask price for the EUR/USD currency pair is 1.5763 this
  // means that you can buy 1 EUR for 1.5763$.
  double ask() const { return bid() + spread_; }

  double close(timeframe tf, std::size_t i) const
  {
    assert(cur_bar_);
    return td_->close(tf, as_series(tf, i));
  }

  double high(timeframe tf, std::size_t i) const
  {
    assert(cur_bar_);
    return td_->high(tf, as_series(tf, i));
  }

  double low(timeframe tf, std::size_t i) const
  {
    assert(cur_bar_);
    return td_->low(tf, as_series(tf, i));
  }

  double open(timeframe tf, std::size_t i) const
  {
    assert(cur_bar_);
    return td_->open(tf, as_series(tf, i));
  }

  double volume(timeframe tf, std::size_t i) const
  {
    assert(cur_bar_);
    return td_->volume(tf, as_series(tf, i));
  }

  bool black_candle(timeframe, std::size_t) const;
  bool white_candle(timeframe, std::size_t) const;

  double order_amount() const { return order_.amount(); }
  double order_open_price() const { return order_.open_price(); }
  double order_profit() const;
  o_type order_type() const { return order_.type(); }

  bool order_send(o_type, double);
  bool order_close();

  unsigned orders_history_total() const;

  template<class T> double run(const T &);

private:
  // Private support functions.
  std::size_t as_series(timeframe tf, std::size_t i) const
  {
    std::size_t b(cur_bar_);

    if (tf != short_tf)
      b /= static_cast<unsigned>(td_->tf_duration[tf].count() /
                                 td_->tf_duration[short_tf].count());

    return b > i ? b - i : 0;
  }

  void clear_status();
  std::size_t inc_bar() { return ++cur_bar_; }

  // Private data members.
  const trading_data *td_;

  order order_ = order();

  double spread_ = 0.0001;  // 1 PIP, 10 Points
  double balance_ = 0.0;

  std::size_t cur_bar_ = 1;

  unsigned orders_history_total_ = 0;
};

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

#endif  // include guard
