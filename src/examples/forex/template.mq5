/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#property copyright   "Copyright 2017, EOS di Manlio Morini"
#property link        "https://www.eosdev.it/"
#property version     "1.01"
#property description "Expert Advisor Template compiled by Vita"
#property description "Based on buy / sell pattern recognition"

// Input parameters
input int EA_Magic = 31415;

input ENUM_TIMEFRAMES medium_timeframe = PERIOD_H1;
input ENUM_TIMEFRAMES long_timeframe   = PERIOD_H4;

input double Lot =  0.1;

// Global variables
MqlTick latest_price;  // TIME, BID, ASK, LAST, VOLUME

struct timeframe_rates
{
  MqlRates bar[];  // TIME, OPEN, HIGH, LOW, CLOSE, TICKET_VOLUME, SPREAD,
                   // REAL_VOLUME
} rates[3];

double close(unsigned tf, unsigned bar)
{
  return rates[tf].bar[bar].close;
}

double open(unsigned tf, unsigned bar)
{
  return rates[tf].bar[bar].open;
}

double high(unsigned tf, unsigned bar)
{
  return rates[tf].bar[bar].high;
}

double low(unsigned tf, unsigned bar)
{
  return rates[tf].bar[bar].low;
}

bool black_candle(unsigned tf, unsigned bar)
{
  return close(tf, bar) < open(tf, bar);
}

bool white_candle(unsigned tf, unsigned bar)
{
  return close(tf, bar) > open(tf, bar);
}

bool long_candle(unsigned tf, unsigned bar)
{
  const double real_body = MathAbs(open(tf, bar) - close(tf, bar));

  double avg_body = 0.0;
  for (unsigned i = 1; i <= 5; ++i)
  {
    double new_bar = MathAbs(open(tf, bar + i) - close(tf, bar + i));
    avg_body += (new_bar - avg_body) / i;
  }

  return 3.0 * avg_body < real_body && real_body < 6.0 * avg_body;
}

bool long_black_candle(unsigned tf, unsigned bar)
{
  return black_candle(tf, bar) && long_candle(tf, bar);
}

bool long_white_candle(unsigned tf, unsigned bar)
{
  return white_candle(tf, bar) && long_candle(tf, bar);
}

bool doji(unsigned tf, unsigned bar)
{
  const double real_body = MathAbs(open(tf, bar) - close(tf, bar));
  const double shadow = high(tf, bar) - low(tf, bar);

  if (shadow > 0.0)
    return real_body / shadow < 0.025;

  return false;
}

bool harami(unsigned tf, unsigned bar)
{
  return long_candle(tf, bar + 1)
         && MathMax(open(tf, bar), close(tf, bar))
            < MathMax(open(tf, bar + 1), close(tf, bar + 1))
         && MathMin(open(tf, bar), close(tf, bar))
            > MathMin(open(tf, bar + 1), close(tf, bar + 1));
}

bool bearish_harami(unsigned tf, unsigned bar)
{
  return long_white_candle(tf, bar + 1) && black_candle(tf, bar)
         && close(tf, bar) >  open(tf, bar + 1)
         &&  open(tf, bar) < close(tf, bar + 1);
}

bool bullish_harami(unsigned tf, unsigned bar)
{
  return long_black_candle(tf, bar + 1) && white_candle(tf, bar)
         && close(tf, bar) <  open(tf, bar + 1)
         &&  open(tf, bar) > close(tf, bar + 1);
}

bool dark_cloud_cover(unsigned tf, unsigned bar)
{
  return white_candle(tf, bar + 1) && black_candle(tf, bar)
         && close(tf, bar) > open(tf, bar + 1)
         &&  open(tf, bar) >= high(tf, bar + 1);
}

int OnInit()
{
  if (Period() >= medium_timeframe || medium_timeframe >= long_timeframe)
    return INIT_PARAMETERS_INCORRECT;

  ArraySetAsSeries(rates[0].bar, true);
  ArraySetAsSeries(rates[1].bar, true);
  ArraySetAsSeries(rates[2].bar, true);

  return INIT_SUCCEEDED;
}

void OnDeinit(const int /*reason*/)
{
  int h = FileOpen("results.txt", FILE_WRITE|FILE_UNICODE|FILE_TXT|FILE_COMMON);

  if (h != INVALID_HANDLE)
  {
    FileWrite(h, TesterStatistics(STAT_PROFIT));
    FileWrite(h, TesterStatistics(STAT_SHORT_TRADES));
    FileWrite(h, TesterStatistics(STAT_LONG_TRADES));
    FileWrite(h, TesterStatistics(STAT_BALANCE_DD));
    FileWrite(h, TesterStatistics(STAT_BALANCE_DDREL_PERCENT));
    FileClose(h);
  }
}

bool place_order(ENUM_ORDER_TYPE type, MqlTradeResult &mresult)
{
  MqlTradeRequest mrequest;  // to be used for sending our trade requests
  ZeroMemory(mrequest);      // initialization of mrequest structure

  double ref_price = (type == ORDER_TYPE_BUY) ? latest_price.ask
                                              : latest_price.bid;
  int sgn = (type == ORDER_TYPE_BUY) ? +1 : -1;

  double lowest = low(0, 1), highest = high(0, 1);
  for (unsigned i = 2; i <= 5; ++i)
  {
    highest = MathMax(highest, high(0, i));
    lowest  = MathMin(lowest,   low(0, i));
  }
  double delta = MathMax(highest - lowest, 100.0 * _Point);

  mrequest.action = TRADE_ACTION_DEAL;  // immediate order execution
  mrequest.price = NormalizeDouble(ref_price, _Digits);
  mrequest.sl = NormalizeDouble(ref_price - sgn * delta, _Digits);
  mrequest.tp = NormalizeDouble(ref_price + sgn * delta, _Digits);
  mrequest.symbol = _Symbol;
  mrequest.volume = Lot;
  mrequest.magic = EA_Magic;
  mrequest.type = type;
  mrequest.deviation = 100;  // deviation from current price

  // Means that the deal can be executed exclusively with a specified volume at
  // the equal or better price than the order specified price. If there is no
  // sufficient volume of offers on the order symbol, the order will not be
  // executed.
  mrequest.type_filling = ORDER_FILLING_FOK;

  bool ret = OrderSend(mrequest, mresult)
             && (mresult.retcode == TRADE_RETCODE_PLACED
                 || mresult.retcode == TRADE_RETCODE_DONE);

  if (!ret)
    Alert("**** ERROR placing a ", (type == ORDER_TYPE_BUY ? "buy" : "sell"),
          " order request - error: ",
          GetLastError());

  return ret;
}

bool new_bar()
{
  static datetime old_time;

  datetime current_time[1];
  if (CopyTime(_Symbol, _Period, 0, 1, current_time) <= 0)
  {
    Alert("**** ERROR in copying historical times data - error: ",
          GetLastError());
    ResetLastError();
    return false;
  }

  bool is_new_bar = (old_time != current_time[0]);

  if (is_new_bar)
    old_time = current_time[0];

  return is_new_bar;
}

bool buy_pattern() {return false;}

bool sell_pattern() {return false;}

bool pick_data()
{
  if (!SymbolInfoTick(_Symbol, latest_price))
  {
    Alert("**** ERROR getting the latest price quote - error: ",
          GetLastError());
    return false;
  }

  const int Window_Size = 16;
  if (CopyRates(_Symbol, Period(), 0, Window_Size, rates[0].bar) < Window_Size
      || CopyRates(_Symbol, medium_timeframe, 0, Window_Size, rates[1].bar)
         < Window_Size
      || CopyRates(_Symbol, long_timeframe, 0, Window_Size, rates[2].bar)
         < Window_Size)
  {
    Alert("**** ERROR copying rates/history data - error: ",
          GetLastError());
    return false;
  }

  return true;
}

void OnTick()
{
  if (Bars(_Symbol, Period()) < 60)
  {
    Alert("**** We have less than 60 bars, EA disabled");
    return;
  }

  if (!new_bar())
    return;

  if (!pick_data())
    return;

  bool buy_condition = false, sell_condition = false;
  if (PositionSelect(_Symbol))  // we have an open position
  {
    if (PositionGetInteger(POSITION_TYPE) == POSITION_TYPE_BUY)
      sell_condition = sell_pattern();
    else  // POSITION_TYPE_SELL
      buy_condition = buy_pattern();
  }
  else
  {
    buy_condition = buy_pattern();
    if (!buy_condition)
      sell_condition = sell_pattern();
  }

  if (buy_condition != sell_condition)
  {
    ENUM_ORDER_TYPE type = buy_condition ? ORDER_TYPE_BUY : ORDER_TYPE_SELL;

    MqlTradeResult mresult;
    if (!place_order(type, mresult))
    {
      ResetLastError();
      return;
    }
  }
}
