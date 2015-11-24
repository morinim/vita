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

#include <algorithm>

#if !defined(FOREX_TRADE_SIMULATOR_H)
#define      FOREX_TRADE_SIMULATOR_H

// ****************************************************************************
// order
// ****************************************************************************
class order
{
public:
  explicit order(int t = na, double a = 0.0, double o = 0.0, unsigned b = 0)
    : amount_(a), open_price_(o), type_(t), bar_(b)
  {
    assert(t == na || t == buy || t == sell);
    assert((t == na && a <= 0.0) || (t != na && a > 0.0));
    assert((t == na && o <= 0.0) || (t != na && o > 0.0));
    assert((t == na && b == 0) || (t != na && b > 0));
  }

  double amount() const { return amount_; }
  double open_price() const { return open_price_; }
  int type() const { return type_; }
  unsigned bar() const { return bar_; }

  enum {na = -1, buy = 0, sell};

private:
  double amount_;
  double open_price_;
  int type_;
  unsigned bar_;
};

// ****************************************************************************
// trade_simulator
// ****************************************************************************
// More details:
// - https://en.wikipedia.org/wiki/Currency_pair
class trade_simulator
{
public:
  explicit trade_simulator(const trading_data &data)
    : td_(data) { clear_status(); }

  // The balance, expressed in counter currency, not including the floating
  // profit/loss from any open trades.
  // Transactions in forex markets are quoted as pairs because traders buy one
  // currency and sell another. For example, EUR/USD is presented as the price
  // of how many US dollars should be spent to buy a Euro.
  // The **base currency** is located to the left and the **counter currency**
  // is located to the right.
  double account_balance() const { return balance_; }

  // If the current bid price for the EUR/USD currency pair is 1.5760 this
  // means that you can sell EUR/USD at 1.5760
  double bid() const { return td_.open(cur_bar_); }

  // If the current ask price for the EUR/USD currency pair is 1.5763 this
  // means that you can buy EUR/USD at 1.5763
  double ask() const { return bid() + spread_; }

  double close(unsigned tf, unsigned i) const
  {
    assert(tf < sup_tf);
    assert(cur_bar_);

    return td_.close(tf, as_series(i));
  }

  double high(unsigned tf, unsigned i) const
  {
    assert(tf < sup_tf);
    assert(cur_bar_);

    return td_.high(tf, as_series(i));
  }

  double low(unsigned tf, unsigned i) const
  {
    assert(tf < sup_tf);
    assert(cur_bar_);

    return td_.low(tf, as_series(i));
  }

  double open(unsigned tf, unsigned i) const
  {
    assert(tf < sup_tf);
    assert(cur_bar_);

    return td_.open(tf, as_series(i));
  }

  double volume(unsigned tf, unsigned i) const
  {
    assert(tf < sup_tf);
    assert(cur_bar_);

    return td_.volume(tf, as_series(i));
  }

  double order_amount() const { return order_.amount(); }
  double order_open_price() const { return order_.open_price(); }
  double order_profit() const;
  int order_type() const { return order_.type(); }

  bool order_send(int, double);
  bool order_close();

  unsigned orders_history_total() const;

  template<class T> double run(const T &);

private:  // Private support functions
  unsigned as_series(unsigned i) const {return cur_bar_ > i ? cur_bar_ - i : 0;}
  void clear_status();
  unsigned inc_bar() { return ++cur_bar_; }

private:  // Private data members
  const trading_data &td_;

  order order_ = order();

  double spread_ = 0.0002;
  double balance_ = 0.0;

  unsigned cur_bar_ = 1;

  unsigned orders_history_total_ = 0;
};

#endif  // include guard
