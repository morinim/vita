/**
 *
 *  \file sudoku.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/primitive/factory.h"
#include "kernel/primitive/int.h"

class square : public vita::integer::number
{
public:
  square(int index) : vita::integer::number(index + 1, 1, 9) {}
};

class fixed_square : public vita::constant
{
public:
  fixed_square(int index, int value) : vita::constant(value, index + 1) {}
};

class error : public vita::function
{
public:
  error()
    : vita::function("ERROR",
                     0,
                     { 1, 2, 3, 4, 5, 6, 7, 8, 9,
                      10,11,12,13,14,15,16,17,18,
                      19,20,21,22,23,24,25,26,27,
                      28,29,30,31,32,33,34,35,36,
                      37,38,39,40,41,42,43,44,45,
                      46,47,48,49,50,51,52,53,54,
                      55,56,57,58,59,60,61,62,63,
                      64,65,66,67,68,69,70,71,72,
                      73,74,75,76,77,78,79,80,81})
  {}

  boost::any eval(vita::interpreter *it) const
  {
    // The Sudoku matrix.
    int matrix[81];
    for (unsigned i(0); i < 81; ++i)
      matrix[i] = boost::any_cast<int>(it->eval(i));

    const int box_err(
      box_errors(matrix, 0) + box_errors(matrix, 3) + box_errors(matrix, 6) +
      box_errors(matrix,27) + box_errors(matrix,30) + box_errors(matrix,33) +
      box_errors(matrix,54) + box_errors(matrix,57) + box_errors(matrix,60));

    int row_err(0), col_err(0);
    for (unsigned i(0); i < 9; ++i)
    {
      row_err += row_errors(matrix, i);
      col_err += col_errors(matrix, i);
    }

    return box_err + row_err + col_err;
  }

private:
  static int box_errors(const int m[], int i)
  {
    // Extracts the 9 elements of a 3 x 3 box of a Sudoku matrix.
    int v[9] = {m[i     ], m[i +  1], m[i +  2],
                m[i +  9], m[i + 10], m[i + 11],
                m[i + 18], m[i + 19], m[i + 20]};

    return vector_errors(v);
  }

  static int col_errors(const int m[], const unsigned col)
  {
    int v[9];
    for (unsigned i(col); i < 81; i += 9)
      v[i % 9] = m[i];

    return vector_errors(v);
  }

  static int row_errors(const int m[], const unsigned row)
  {
    int v[9];
    for (unsigned i(row * 9); i < 81; ++i)
      v[i - row * 9] = m[i];

    return vector_errors(v);
  }

  // Checks how many different elements there are in the vector.
  // Ideally there should be 9 different elements (no duplicates).
  static int vector_errors(const int v[9])
  {
    unsigned count[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    int err(0);
    for (unsigned i(0); i < 9; ++i)
      if (++count[v[i]] > 1)
        ++err;

    return err;
  }
};

class fitness : public vita::evaluator
{
  vita::score_t operator()(const vita::individual &solution)
  {
    const int err(boost::any_cast<int>((vita::interpreter(solution))()));

    const vita::fitness_t f(-err);

    // 8 maximum number of errors in a box / column / row.
    // 9 number of boxes / columns / rows.
    // 3 measurements (box, column, row).
    const double perc(1.0 - f / (8.0 * 9.0 * 3.0));

    return vita::score_t(f, perc);
  }
};

int main(int argc, char *argv[])
{
  vita::environment env(true);

  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = 1;

  int matrix[] = {0, 0, 6, 0, 4, 0, 5, 0, 0,
                  0, 0, 0, 1, 0, 0, 0, 0, 0,
                  0, 7, 9, 5, 0, 0, 3, 2, 0,
                  0, 0, 0, 9, 0, 0, 2, 5, 0,
                  2, 0, 0, 0, 8, 0, 0, 0, 7,
                  0, 8, 4, 0, 0, 2, 0, 0, 0,
                  0, 4, 2, 0, 0, 3, 1, 9, 0,
                  0, 0, 0, 0, 0, 6, 0, 0, 0,
                  0, 0, 5, 0, 7, 0, 8, 0, 0};

  for (unsigned i(0); i < 81; ++i)
    if (matrix[i] > 0)
      env.insert(std::make_shared<fixed_square>(i, matrix[i]));
    else
      env.insert(std::make_shared<square>(i));
  env.insert(std::make_shared<error>());

  std::unique_ptr<vita::evaluator> eva(new fitness());

  vita::evolution(env, eva.get())(true, 1);

  return EXIT_SUCCESS;
}
