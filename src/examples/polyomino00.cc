/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/polyomino_tutorial
 */

#include <iostream>
#include <string>

#include "kernel/vita.h"
#include "utility/matrix.h"

const std::size_t board_height = 8;
const std::size_t board_width  = 8;

using shape = vita::matrix<int>;

std::vector<std::vector<shape>> piece_masks;

shape put(const shape &piece, std::size_t y, std::size_t x)
{
  shape ret(board_height, board_width);  // initially filled with `0`s

  if (y + piece.rows() > ret.rows())
    return ret;

  if (x + ret.cols() > ret.cols())
    return ret;

  for (std::size_t row(0); row < piece.rows(); ++row)
    for (std::size_t col(0); col < piece.cols(); ++col)
      ret(y + row, x + col) += piece(row, col);

  return ret;
}

///
/// Checks if a location is surrounded.
///
/// A location is surrounded if locations at North, South, East and West are
/// occupied or out of the frame.
///
bool circled(const shape &board, std::size_t y, std::size_t x)
{
  const auto v(board(y, x));

  if (y > 0 && board(y - 1, x) == v)
    return false;
  if (y + 1 < board.rows() && board(y + 1, x) == v)
    return false;
  if (x > 0 && board(y, x - 1) == v)
    return false;
  if (x + 1 < board.cols() && board(y, x + 1) == v)
    return false;

  return true;
}

///
/// Counts how many surrounded empty location are present on the board.
///
unsigned circled_zero(const shape &board)
{
  unsigned n(0);

  for (std::size_t y(0); y < board.rows(); ++y)
    for (std::size_t x(0); x < board.cols(); ++x)
      if (board(y, x) == 0 && circled(board, y, x))
        ++n;

  return n;
}

std::size_t add_piece_variants(const shape &piece)
{
  std::set<shape> ms;

  const shape empty(board_height, board_width);  // filled with `0`s

  for (unsigned reflection(0); reflection <= 1; ++reflection)
    for (unsigned rotation(0); rotation <= 3; ++rotation)
      for (unsigned y(0); y < board_height; ++y)
        for (unsigned x(0); x < board_width; ++x)
        {
          shape flipped(reflection ? vita::fliplr(piece) : piece);
          shape piece1(vita::rot90(flipped, rotation));

          shape piece_on_board(put(piece1, y, x));
          if (piece_on_board != empty && !circled_zero(piece_on_board))
            ms.insert(piece_on_board);
        }

  piece_masks.emplace_back(ms.begin(), ms.end());

  return ms.size();
}

void fill_piece_masks()
{
  std::cout << "Calculating variants...";

  add_piece_variants({ {'A', 'A', 'A'},
                       { 0 , 'A',  0 } });

  add_piece_variants({ {'B', 'B', 'B'},
                       {'B',  0 , 'B'} });

  add_piece_variants({ {'C', 'C'},
                       {'C', 'C'} });

  add_piece_variants({ {'D',  0 , 'D'},
                       {'D', 'D', 'D'},
                       {'D',  0 , 'D'} });

  add_piece_variants({ { 0 , 'E',  0 },
                       {'E', 'E', 'E'},
                       {'E',  0 ,  0 } });

  add_piece_variants({ { 0 , 'F', 'F'},
                       { 0 , 'F',  0 },
                       {'F', 'F',  0 } });

  add_piece_variants({ { 0 , 'G'},
                       {'G', 'G'},
                       {'G',  0 } });

  add_piece_variants({ { 0 ,  0 , 'H', 'H'},
                       {'H', 'H', 'H',  0 } });

  add_piece_variants({ { 0 , 'I'},
                       { 0 , 'I'},
                       { 0 , 'I'},
                       {'I', 'I'} });

  add_piece_variants({ {'J', 'J', 'J'},
                       { 0 ,  0 , 'J'},
                       { 0 ,  0 , 'J'} });

  add_piece_variants({ {'K', 'K', 'K'},
                       {'K',  0 ,  0 } });

  add_piece_variants({ {'L', 'L', 'L'},
                       {'L', 'L', 'L'} });

  add_piece_variants({ { 0 , 'M',  0 ,  0 },
                       {'M', 'M', 'M', 'M'} });

  std::cout << "ok\n";
}

void print_board(const shape &board)
{
  std::size_t i(0);

  for (const auto &e : board)
  {
    if (std::isprint(e))
      std::cout << char(e);
    else if (e == 0)
      std::cout << '.';
    else
      std::cout << '+';

    ++i;

    if (i % board.cols() == 0)
      std::cout << '\n';
    else
      std::cout << ' ';
  }

  std::cout << std::endl;
}

///
/// Blind brute force search of the best configuration.
///
void rec_put(const shape &base, unsigned piece_id)
{
  int max = 0;

  if (piece_id == piece_masks.size())
  {
    // Number of non-empty squares.
    int filled(std::count_if(base.begin(), base.end(),
                             [](unsigned v) { return v != 0; }));

    if (filled > max)
    {
      max = filled;
      std::cout << filled << '\n';
      print_board(base);
    }
  }

  for (unsigned i(0); i < piece_masks[piece_id].size(); ++i)
  {
    shape new_base(base);
    new_base += piece_masks[piece_id][i];

    rec_put(new_base, piece_id + 1);
  }
}

///
/// Repeatedly fills the board with random a random configuration.
///
void random_put(const shape &base)
{
  int max = 0;

  while (max < 64)
  {
    shape board(base);

    for (const auto &piece : piece_masks)
      board += vita::random::element(piece);

    // Number of non-empty squares.
    int filled(std::count_if(board.begin(), board.end(),
                             [](unsigned v) { return v != 0; }));

    if (filled > max)
    {
      max = filled;
      std::cout << filled << '\n';
      print_board(board);
    }
  }
}

int main()
{
  fill_piece_masks();

  shape empty(board_height, board_width);
  random_put(empty);
}
