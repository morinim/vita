/**
 *  \file
 *  \copyright Copyright (C) 2016 EOS di Manlio Morini
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#property copyright "2016, EOS di Manlio Morini"
#property strict

double gp_spread = 0.0001;
double gp_lots = 0.01;
double gp_close_level = 10;
int gp_tf[3] = {PERIOD_M5, PERIOD_M30, PERIOD_H1};

const int magic = 3141492;

bool black_candle(int tf, int bar)
{
  return iClose(NULL, tf, bar) < iOpen(NULL, tf, bar);
}

bool white_candle(int tf, int bar)
{
  return iClose(NULL, tf, bar) > iOpen(NULL, tf, bar);
}

bool long_candle(int tf, int bar)
{
  double avg_body = 0.0;
  for (int i(1); i <= 5; ++i)
    avg_body += MathAbs(iOpen(NULL, tf, bar - i) - iClose(NULL, tf, bar - i));

  double real_body = MathAbs(iOpen(NULL, tf, bar) - iClose(NULL, tf, bar));
  return real_body > 3.0 * avg_body;
}

bool long_black_candle(int tf, int bar)
{
  return black_candle(tf, bar) && long_candle(tf, bar);
}

bool long_white_candle(int tf, int bar)
{
  return white_candle(tf, bar) && long_candle(tf, bar);
}

/// \brief Expert initialization function
int OnInit()
{
  if (IsTesting())
  {
    if (gp_spread != MarketInfo(Symbol(), MODE_SPREAD) / MathPow(10, Digits))
      return INIT_FAILED;

    if (gp_tf[0] != Period())
      return INIT_FAILED;
  }

  return INIT_SUCCEEDED;
}

/// \brief Expert deinitialization function
void OnDeinit(const int reason)
{
}

bool buy() { return High[1] > Open[2]; }

bool sell() { return false; }

int entry_strategy()
{
  const bool buy_sig = buy();
  const bool sell_sig = sell();
  
  int ticket = -1;
    
  if (buy_sig != sell_sig)
  {
    const double price = buy_sig ? Ask : Bid;
    const int cmd = buy_sig ? OP_BUY : OP_SELL;
   
    const double pip = gp_close_level * MathPow(10.0, Digits) / (gp_lots * 100000.0);
          
    // ticket = OrderSend(Symbol(), cmd, gp_lots, price, 50, price - pip*Point, price + pip*Point, NULL, magic);
    ticket = OrderSend(Symbol(), cmd, gp_lots, price, 50, 0, 0, NULL, magic);
  }
  
  return ticket;
}

void management_strategy(int &ticket)
{
  if (!OrderSelect(ticket, SELECT_BY_TICKET) || OrderMagicNumber() != magic)
    return;

  if (OrderCloseTime() > 0)
  {
    ticket = -1;
    return;
  }

  const double amount = 100000.0 * gp_lots;
  double delta;
  if (OrderType() == OP_BUY)
    delta = amount * (Bid - OrderOpenPrice());
  else if (OrderType() == OP_SELL)
    delta = amount * (OrderOpenPrice() - Ask);
  const double price = (OrderType() == OP_BUY) ? Bid : Ask;

  if (MathAbs(delta) > gp_close_level)
  {
    if (!OrderClose(ticket, OrderLots(), price, 3))
      Alert("Cannot close order");
  }

  bool close_order = (OrderType() == OP_BUY) ? sell() : buy();
  
  if (close_order)
  {
    if (!OrderClose(ticket, OrderLots(), price, 3))
      Alert("Cannot close order");  
  }
}

/// \brief Expert tick function
void OnTick()
{
  static int ticket = -1;
  static datetime last_bar = 0;

  if (Time[0] == last_bar)
    return;

  last_bar = Time[0];

  if (ticket < 0)
    ticket = entry_strategy();
  else
    management_strategy(ticket);
}
