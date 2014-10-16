/**
 *
 *  \file rubik.cc
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <memory>

#include "kernel/environment.h"
#include "kernel/evolution.h"
#include "kernel/interpreter.h"
#include "kernel/primitive/factory.h"

///
///       t t t                 t: top
///       t t t                 l: left
///       t t t                 f: front
/// l l l f f f r r r b b b     r: right
/// l l l f f f r r r b b b     b: back
/// l l l f f f r r r b b b     d: down
///       d d d
///       d d d
///       d d d
///
///       0 1 2
///       3 4 5
///       6 7 8
/// 0 1 2 0 1 2 0 1 2 0 1 2
/// 3 4 5 3 4 5 3 4 5 3 4 5
/// 6 7 8 6 7 8 6 7 8 6 7 8
///       0 1 2
///       3 4 5
///       6 7 8
class rubik_cube
{
public:
  enum color {blue, green, orange, red, white, yellow};
  enum face {back, down, front, left, right, top};

  rubik_cube();

  void rotate_b() { rotate_c(back); }
  void rotate_b1() { rotate_a(back); }
  void rotate_d() { rotate_c(down); }
  void rotate_d1() { rotate_a(down); }
  void rotate_f() { rotate_c(front); }
  void rotate_f1() { rotate_a(front); }
  void rotate_l() { rotate_c(left); }
  void rotate_l1() { rotate_a(left); }
  void rotate_r() { rotate_c(right); }
  void rotate_r1() { rotate_a(right); }
  void rotate_t() { rotate_c(top); }
  void rotate_t1() { rotate_a(top); }

  unsigned diff_back_ok() const { return diff_face_ok(back); }
  unsigned diff_down_ok() const { return diff_face_ok(down); }
  unsigned diff_front_ok() const { return diff_face_ok(front); }
  unsigned diff_left_ok() const { return diff_face_ok(left); }
  unsigned diff_right_ok() const { return diff_face_ok(right); }
  unsigned diff_top_ok() const { return diff_face_ok(top); }

  unsigned diff_front_plus() const;

  unsigned diff_down_t() const;
  unsigned diff_left_t() const;
  unsigned diff_right_t() const;
  unsigned diff_top_t() const;

  unsigned diff_down_double() const;
  unsigned diff_left_double() const;
  unsigned diff_right_double() const;
  unsigned diff_top_double() const;

  void display() const;

private:
  static char color_to_char(color);

  void rotate_c(face);
  void rotate_a(face f) { rotate_c(f); rotate_c(f); rotate_c(f); }

  unsigned diff_face_ok(face) const;

  color square[6][9];
};

rubik_cube::rubik_cube()
{
  for (unsigned f(0); f < 6; ++f)
    for (unsigned i(0); i < 9; ++i)
      square[f][i] = static_cast<color>(f);

  while (vita::random::between<unsigned>(0, 100) > 0)
    rotate_c(static_cast<face>(vita::random::between<unsigned>(0, 6)));
}

char rubik_cube::color_to_char(color c)
{
  switch (c)
  {
  case blue:   return 'B';
  case green:  return 'G';
  case orange: return 'O';
  case red:    return 'R';
  case white:  return 'W';
  default:     return 'Y';  // case yellow:
  }
}

void rubik_cube::display() const
{
  std::cout
    << "       "
    << square[  top][0] << ' ' << square[  top][1] << ' ' << square[  top][2]
    << square[  top][3] << ' ' << square[  top][4] << ' ' << square[  top][5]
    << square[  top][6] << ' ' << square[  top][7] << ' ' << square[  top][8]
    << square[ left][0] << ' ' << square[ left][1] << ' ' << square[ left][2]
    << square[front][0] << ' ' << square[front][1] << ' ' << square[front][2]
    << square[right][0] << ' ' << square[right][1] << ' ' << square[right][2]
    << square[ back][0] << ' ' << square[ back][1] << ' ' << square[ back][2]
    << square[ left][3] << ' ' << square[ left][4] << ' ' << square[ left][5]
    << square[front][3] << ' ' << square[front][4] << ' ' << square[front][5]
    << square[right][3] << ' ' << square[right][4] << ' ' << square[right][5]
    << square[ back][3] << ' ' << square[ back][4] << ' ' << square[ back][5]
    << square[ left][6] << ' ' << square[ left][7] << ' ' << square[ left][8]
    << square[front][6] << ' ' << square[front][7] << ' ' << square[front][8]
    << square[right][6] << ' ' << square[right][7] << ' ' << square[right][8]
    << square[ back][6] << ' ' << square[ back][7] << ' ' << square[ back][8]
    << "       "
    << square[ down][0] << ' ' << square[ down][1] << ' ' << square[ down][2]
    << square[ down][3] << ' ' << square[ down][4] << ' ' << square[ down][5]
    << square[ down][6] << ' ' << square[ down][7] << ' ' << square[ down][8]
    << std::endl;
}

unsigned rubik_cube::diff_face_ok(face f) const
{
  unsigned diff_count(0);
  for (unsigned i(0); i < 9; ++i)
    if (square[f][i] != static_cast<color>(f))
      ++diff_count;

  return diff_count;
}

unsigned rubik_cube::diff_front_plus() const
{
  const color c(static_cast<color>(front));

  unsigned diff_count(0);
  diff_count += (square[front][1] != c);
  diff_count += (square[front][3] != c);
  diff_count += (square[front][4] != c);
  diff_count += (square[front][5] != c);
  diff_count += (square[front][7] != c);

  return diff_count;
}

unsigned rubik_cube::diff_down_t() const
{
  const color c(static_cast<color>(down));

  unsigned diff_count(0);
  diff_count += (square[down][0] != c);
  diff_count += (square[down][1] != c);
  diff_count += (square[down][2] != c);
  diff_count += (square[down][4] != c);

  return diff_count;
}

unsigned rubik_cube::diff_left_t() const
{
  const color c(static_cast<color>(left));

  unsigned diff_count(0);
  diff_count += (square[left][2] != c);
  diff_count += (square[left][4] != c);
  diff_count += (square[left][5] != c);
  diff_count += (square[left][8] != c);

  return diff_count;
}

unsigned rubik_cube::diff_right_t() const
{
  const color c(static_cast<color>(right));

  unsigned diff_count(0);
  diff_count += (square[right][0] != c);
  diff_count += (square[right][3] != c);
  diff_count += (square[right][4] != c);
  diff_count += (square[right][6] != c);

  return diff_count;
}

unsigned rubik_cube::diff_top_t() const
{
  const color c(static_cast<color>(top));

  unsigned diff_count(0);
  diff_count += (square[top][6] != c);
  diff_count += (square[top][7] != c);
  diff_count += (square[top][4] != c);
  diff_count += (square[top][8] != c);

  return diff_count;
}

unsigned rubik_cube::diff_down_double() const
{
  const color c(static_cast<color>(down));

  unsigned diff_count(0);
  diff_count += (square[down][0] != c);
  diff_count += (square[down][1] != c);
  diff_count += (square[down][2] != c);
  diff_count += (square[down][3] != c);
  diff_count += (square[down][4] != c);
  diff_count += (square[down][5] != c);

  return diff_count;
}

unsigned rubik_cube::diff_left_double() const
{
  const color c(static_cast<color>(left));

  unsigned diff_count(0);
  diff_count += (square[left][1] != c);
  diff_count += (square[left][2] != c);
  diff_count += (square[left][4] != c);
  diff_count += (square[left][5] != c);
  diff_count += (square[left][7] != c);
  diff_count += (square[left][8] != c);

  return diff_count;
}

unsigned rubik_cube::diff_right_double() const
{
  const color c(static_cast<color>(right));

  unsigned diff_count(0);
  diff_count += (square[right][0] != c);
  diff_count += (square[right][1] != c);
  diff_count += (square[right][3] != c);
  diff_count += (square[right][4] != c);
  diff_count += (square[right][6] != c);
  diff_count += (square[right][7] != c);

  return diff_count;
}

unsigned rubik_cube::diff_top_double() const
{
  const color c(static_cast<color>(top));

  unsigned diff_count(0);
  diff_count += (square[top][3] != c);
  diff_count += (square[top][4] != c);
  diff_count += (square[top][5] != c);
  diff_count += (square[top][6] != c);
  diff_count += (square[top][7] != c);
  diff_count += (square[top][8] != c);

  return diff_count;
}

void rubik_cube::rotate_c(face f)
{
  rubik_cube orig(*this);

  square[f][0] = orig.square[f][6];
  square[f][1] = orig.square[f][3];
  square[f][2] = orig.square[f][0];
  square[f][3] = orig.square[f][7];
  // square[f][4] = orig.square[f][4];
  square[f][5] = orig.square[f][1];
  square[f][6] = orig.square[f][8];
  square[f][7] = orig.square[f][5];
  square[f][8] = orig.square[f][2];

  switch (f)
  {
  case back:
    square[down][6] = orig.square[left][0];
    square[down][7] = orig.square[left][3];
    square[down][8] = orig.square[left][6];
    square[left][0] = orig.square[top][2];
    square[left][3] = orig.square[top][1];
    square[left][6] = orig.square[top][0];
    square[right][2] = orig.square[down][8];
    square[right][5] = orig.square[down][7];
    square[right][8] = orig.square[down][6];
    square[top][0] = orig.square[right][2];
    square[top][1] = orig.square[right][5];
    square[top][2] = orig.square[right][8];
    break;
  case down:
    square[back][6] = orig.square[right][6];
    square[back][7] = orig.square[right][7];
    square[back][8] = orig.square[right][8];
    square[front][6] = orig.square[left][6];
    square[front][7] = orig.square[left][7];
    square[front][8] = orig.square[left][8];
    square[left][6] = orig.square[back][6];
    square[left][7] = orig.square[back][7];
    square[left][8] = orig.square[back][8];
    square[right][6] = orig.square[front][6];
    square[right][7] = orig.square[front][7];
    square[right][8] = orig.square[front][8];
    break;
  case front:
    square[down][0] = orig.square[right][6];
    square[down][1] = orig.square[right][3];
    square[down][2] = orig.square[right][0];
    square[left][2] = orig.square[down][0];
    square[left][5] = orig.square[down][1];
    square[left][8] = orig.square[down][2];
    square[right][0] = orig.square[top][6];
    square[right][3] = orig.square[top][7];
    square[right][6] = orig.square[top][8];
    square[top][6] = orig.square[left][8];
    square[top][7] = orig.square[left][5];
    square[top][8] = orig.square[left][2];
    break;
  case left:
    square[back][2] = orig.square[down][6];
    square[back][5] = orig.square[down][3];
    square[back][8] = orig.square[down][0];
    square[down][0] = orig.square[front][0];
    square[down][3] = orig.square[front][3];
    square[down][6] = orig.square[front][6];
    square[front][0] = orig.square[top][0];
    square[front][3] = orig.square[top][3];
    square[front][6] = orig.square[top][6];
    square[top][0] = orig.square[back][8];
    square[top][3] = orig.square[back][5];
    square[top][6] = orig.square[back][2];
    break;
  case right:
    square[back][0] = orig.square[top][8];
    square[back][3] = orig.square[top][5];
    square[back][6] = orig.square[top][2];
    square[down][2] = orig.square[back][6];
    square[down][5] = orig.square[back][3];
    square[down][8] = orig.square[back][0];
    square[front][2] = orig.square[down][2];
    square[front][5] = orig.square[down][5];
    square[front][8] = orig.square[down][8];
    square[top][2] = orig.square[front][2];
    square[top][5] = orig.square[front][5];
    square[top][8] = orig.square[front][8];
    break;
  default:  // case top:
    square[back][0] = orig.square[left][0];
    square[back][1] = orig.square[left][1];
    square[back][2] = orig.square[left][2];
    square[front][0] = orig.square[right][0];
    square[front][1] = orig.square[right][1];
    square[front][2] = orig.square[right][2];
    square[left][0] = orig.square[front][0];
    square[left][1] = orig.square[front][1];
    square[left][2] = orig.square[front][2];
    square[right][0] = orig.square[back][0];
    square[right][1] = orig.square[back][1];
    square[right][2] = orig.square[back][2];
    break;
  }
}

/*
class X : public vita::terminal
{
public:
  X() : vita::terminal("X", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Y : public vita::terminal
{
public:
  Y() : vita::terminal("Y", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

class Z : public vita::terminal
{
public:
  Z() : vita::terminal("Z", 0, true) {}

  boost::any eval(vita::interpreter *) const { return val; }

  static double val;
};

double X::val;
double Y::val;
double Z::val;

class fitness : public vita::evaluator
{
  vita::score_t operator()(const vita::individual &ind)
  {
    vita::interpreter agent(ind);

    vita::fitness_t fit(0.0);
    for (double x(0); x < 10; ++x)
      for (double y(0); y < 10; ++y)
        for (double z(0); z < 10; ++z)
        {
          X::val = x;
          Y::val = y;
          Z::val = z;

          const boost::any res(agent());

          if (!res.empty())
          {
            const double dres(boost::any_cast<double>(res));
            assert(std::isfinite(dres));
            fit += std::exp(-std::fabs(dres - (x*x + y*y - z*z)));
          }
        }

    return vita::score_t(fit, -1.0);
  }
};

int main(int argc, char *argv[])
{
  vita::environment env;

  env.individuals = argc > 1 ? atoi(argv[1]) : 100;
  env.code_length = argc > 2 ? atoi(argv[2]) : 100;
  env.g_since_start = argc > 3 ? atoi(argv[3]) : 100;

  vita::symbol_factory &factory(vita::symbol_factory::instance());
  env.insert(std::make_shared<X>());
  env.insert(std::make_shared<Y>());
  env.insert(std::make_shared<Z>());
  env.insert(factory.make("ADD", vita::d_double));
  env.insert(factory.make("SUB", vita::d_double));
  env.insert(factory.make("MUL", vita::d_double));
  env.insert(factory.make("IFL", vita::d_double));
  env.insert(factory.make("IFE", vita::d_double));

  std::unique_ptr<vita::evaluator> eva(new fitness());

  vita::evolution(env, eva.get())(true, 1);

  return EXIT_SUCCESS;
}
*/
