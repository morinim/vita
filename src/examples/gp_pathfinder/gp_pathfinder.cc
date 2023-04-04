/*
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2022-2023 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 *  \see https://github.com/morinim/vita/wiki/pathfinding_tutorial
 */

#include <filesystem>
#include <iostream>
#include <vector>

#include <SDL2/SDL.h>

#include "kernel/vita.h"

// Algorithms like A* are well known solutions to the pathfinding problem, but
// there is a distinction between *efficient and accurate* pathfinding
// algorithms and *realistic* pathfinding algorithms. The traditional definition
// of a "good" algorithm requires it be complete and quick. My definition of
// "realistic", in contrast, is that an algorithm be reasonably good,
// reasonably fast and reasonably fallable. That is, it should not be perfect
// and when it does fail, it should fail in much the same way a human fails.
//
// Rick Strom (project Hampton)

/*********************************************************************
 *  Direction
 *********************************************************************/
enum direction {north, east, south, west};

direction d_turn_right(direction d)
{
  return d == direction::west ? direction::north
                              : static_cast<direction>(d + 1);
}

direction d_turn_left(direction d)
{
  return d == direction::north ? direction::west
                               : static_cast<direction>(d - 1);
}

/********************************************************************
 *  Position
 ********************************************************************/
struct position
{
  position(int yv = -1, int xv = -1) : y(yv), x(xv) {}

  bool operator==(const position &rhs) const
  { return y == rhs.y && x == rhs.x; }
  bool operator!=(const position &rhs) const
  { return !(*this == rhs); }

  int y, x;

  static const position npos;
};

const position position::npos = position();

unsigned distance(position p1, position p2)
{
  return std::abs(p1.x - p2.x) + std::abs(p1.y - p2.y);
}

/*********************************************************************
 *  Terrain
 *********************************************************************/
//
// Moving to any square adds `1` to the total distance of the path. While
// squares marked 'X' cannot be moved to, all other squares can, with an
// associated cost. Stepping on a square gives a penalty to the unit except
// when the path is marked `0` (flat). Water gives a penalty of `5`, enemy
// squares give a penalty of `9` (enemy avoidance).
//
enum terrain : char {enemy = 'V', unpassable = 'X', water = '~',
                     flat = '0', nearly_flat = '1', gentle_slope = '2',
                     moderate_slope ='3', strong_slope = '4',
                     extreme_slope = '5', steep_slope = '6'};

SDL_Color t_color(terrain t)
{
  switch (t)
  {
  case enemy:          return {255, 0, 0, 0};
  case water:          return {0, 0, 255, 0};
  case flat:           return {86, 125, 70, 0};
  case nearly_flat:    return {255, 255, 153, 0};
  case gentle_slope:   return {255, 255, 102, 0};
  case moderate_slope: return {255, 255, 51, 0};
  case strong_slope:   return {181, 101, 29, 0};
  case extreme_slope:  return {101, 67, 33, 0};
  case steep_slope:    return {43, 29, 20, 0};
  default:
    return {0, 0, 0, 0};
  }
}

int t_penalty(terrain t)
{
  switch (t)
  {
  case enemy:          return 20;
  case water:          return 10;
  case flat:           return  0;
  case nearly_flat:    return  1;
  case gentle_slope:   return  2;
  case moderate_slope: return  3;
  case strong_slope:   return  4;
  case extreme_slope:  return  5;
  case steep_slope:    return  6;
  default:
    return 1000000;
  }
}

/*********************************************************************
 *  Map
 *********************************************************************/
//
// Maps are based on a grid of valid characters stored in a text file.
//
class map
{
public:
  explicit map(int h = 40, int w = 40) : grid_(h, w) {}

  explicit map(std::vector<std::string> r) : map(r.size(), r.front().size())
  {
    for (int y(0); y < height(); ++y)
      for (int x(0); x < width(); ++x)
        grid_(y, x) = static_cast<terrain>(r[y][x]);
  }

  explicit map(const std::filesystem::path &p)
  {
    std::ifstream f(p);

    std::vector<std::string> lines;
    for (std::string l; std::getline(f, l); )
      lines.push_back(l);

    *this = map(lines);
  }

  bool is_valid(position p) const
  {
    return 0 <= p.x && p.x < width()
           && 0 <= p.y && p.y < height();
  }

  terrain operator[](position p) const
  {
    return is_valid(p) ? grid_(p.y, p.x) : terrain::unpassable;
  }

  position ahead(position p, direction dir, int steps) const
  {
    position p1(p.y + delta_y(dir) * steps, p.x + delta_x(dir) * steps);

    return is_valid(p1) ? p1 : position::npos;
  }

  int height() const { return grid_.rows(); }
  int width() const { return grid_.cols(); }

private:
  static int delta_x(direction d)
  {
    switch (d)
    {
    case direction::east:  return +1;
    case direction::west:  return -1;
    default:               return  0;
    }
  }

  static int delta_y(direction d)
  {
    switch (d)
    {
    case direction::north:  return -1;
    case direction::south:  return +1;
    default:                return  0;
    }
  }

  vita::matrix<terrain> grid_;
};

/*********************************************************************
 *  Agent
 *********************************************************************/
class agent
{
public:
  explicit agent(position p) : p_(p) {}

  direction dir() const { return dir_; }
  position pos() const { return p_; }
  void pos(position p) { p_ = p; }

  void move_to(position p)
  {
    trajectory_.emplace_back(pos(), dir());
    pos(p);
  }

  void turn_right() { dir_ = d_turn_right(dir_); }
  void turn_left() { dir_ = d_turn_left(dir_); }

  const std::vector<std::pair<position, direction>> &trajectory() const
  {
    return trajectory_;
  }

private:
  position p_ {};
  direction dir_ {direction::east};

  std::vector<std::pair<position, direction>> trajectory_ {};
};

/*********************************************************************
 *  Simulation
 *********************************************************************/
class simulation
{
public:
  explicit simulation(const map &m, position p = position::npos,
                      position g = position::npos)
    : map_(m), agent_(p), goal_(g) {}

  position ahead(position p, direction d, int steps) const
  {
    return map_.ahead(p, d, steps);
  }

  position ahead(int steps) const
  {
    return ahead(pos(), dir(), steps);
  }

  terrain terrain_at(position p) const
  {
    return map_[p];
  }

  void move_forward()
  {
    if (const position p1 = ahead(1);
        map_.is_valid(p1) && terrain_at(p1) != unpassable)
      agent_.move_to(p1);
  }

  void turn_right() { agent_.turn_right(); }
  void turn_left()  {  agent_.turn_left(); }

  direction dir() const { return agent_.dir(); }
  position pos() const { return agent_.pos(); }
  position goal() const { return goal_; }

  void pos(position p) { agent_.pos(p); }
  void goal(position p) { goal_ = p; }

  const ::map &map() const { return map_; }
  const ::agent &agent() const { return agent_; }

private:
  ::map map_;

  ::agent agent_;

  position goal_;
};

simulation *active_sim = nullptr;

/*********************************************************************
 *  Action
 *********************************************************************/

// An agent has the following moves: turn right, turn left, move forward.
// We could instead define the set of terminals to be move forward, move left
// and move right (wherein the agent physically moves to the right or left) but
// our function set limits information gathering to squares ahead. If we used
// the latter set, then we would force the agent to act blindly whenever it
// opts to move.
class action : public vita::terminal
{
protected:
  explicit action(const std::string &n) : vita::terminal(n) {}
};

class move_forward : public action
{
public:
  move_forward() : action("move_forward") {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    active_sim->move_forward();
    return {};
  }
};

class turn_right : public action
{
public:
  turn_right() : action("turn_right") {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    active_sim->turn_right();
    return {};
  }
};

class turn_left : public action
{
public:
  turn_left() : action("turn_left") {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    active_sim->turn_left();
    return {};
  }
};

class do_both : public vita::function
{
public:
  explicit do_both() : vita::function("do_both", 2) {}

  vita::value_t eval(vita::symbol_params &p) const final
  {
    p.fetch_opaque_arg(0);
    p.fetch_opaque_arg(1);
    return {};
  }
};

/*********************************************************************
 *  Sensor
 *********************************************************************/
//
// An agent can sense the following:
// - water within 3 squares ahead;
// - enemies within 1 square ahead;
// - trees from 5 squares ahead;
// - if the forward square is more or less steep than the current square;
// - if the goal is ahead, behind, to the right or to the left.
//
class sensor : public vita::function
{
protected:
  explicit sensor(const std::string &n) : vita::function(n, 0, {0, 0}) {}
};

class is_something_happening : public sensor
{
protected:
  is_something_happening(const std::string &m, std::function<bool()> c)
    : sensor(m), check_(std::move(c))
  {
  }

  vita::value_t eval(vita::symbol_params &p) const final
  {
    if (check_())
      p.fetch_opaque_arg(0);
    else
      p.fetch_opaque_arg(1);

    return {};
  }

private:
  std::function<bool()> check_;
};

// Common code for is_water_ahead, is_blocked_ahead...
template<terrain T, int D> class is_something_ahead
  : public is_something_happening
{
protected:
  explicit is_something_ahead(const std::string &m)
    : is_something_happening(
        m,
        []
        {
          return active_sim->terrain_at(active_sim->ahead(D)) == T;
        })
  {
  }
};

// Common code for is_water_ahead1, is_water_ahead2, is_water_ahead3.
template<int D> class is_water_ahead
  : public is_something_ahead<terrain::water, D>
{
protected:
  is_water_ahead() : is_water_ahead::is_something_ahead("is_water_ahead"
                                                        + std::to_string(D))
  {
  }
};

class is_water_ahead1 : public is_water_ahead<1> {};
class is_water_ahead2 : public is_water_ahead<2> {};
class is_water_ahead3 : public is_water_ahead<3> {};

// Detects enemy units one square ahead.
class is_enemy_ahead : public is_something_ahead<terrain::enemy, 1>
{
public:
  is_enemy_ahead() : is_something_ahead<terrain::enemy, 1>("is_enemy_ahead")
  {
  }
};

// Common code for is_blocked_ahead1, is_blocked_ahead2, is_blocked_ahead3.
template<int D> class is_blocked_ahead
  : public is_something_ahead<terrain::unpassable, D>
{
protected:
  is_blocked_ahead() : is_blocked_ahead::is_something_ahead("is_blocked_ahead")
  {
  }
};

class is_blocked_ahead1 : public is_blocked_ahead<1> {};
class is_blocked_ahead2 : public is_blocked_ahead<2> {};
class is_blocked_ahead3 : public is_blocked_ahead<3> {};

// `true` if penalty ahead greater than current penalty.
class is_steeper_ahead : public is_something_happening
{
public:
  is_steeper_ahead()
    : is_something_happening(
        "is_steeper_ahead",
        []
        {
          return t_penalty(active_sim->terrain_at(active_sim->ahead(1)))
                 > t_penalty(active_sim->terrain_at(active_sim->pos()));
        })
      {
      }
};

// `true` if penalty ahead is less than current penalty.
class is_less_steep_ahead : public is_something_happening
{
public:
  is_less_steep_ahead()
    : is_something_happening(
        "is_less_steep_ahead",
        []
        {
          return t_penalty(active_sim->terrain_at(active_sim->ahead(1)))
                 < t_penalty(active_sim->terrain_at(active_sim->pos()));
        })
      {
      }
};

// Tests relative direction of goal.
class is_goal_somewhere : public is_something_happening
{
protected:
  is_goal_somewhere(const std::string &n, std::function<position()> new_pos)
    : is_something_happening(
        n,
        [this]
        {
          const auto current_distance(distance(active_sim->pos(),
                                               active_sim->goal()));
          const auto new_distance(distance(new_pos_(), active_sim->goal()));

          return new_distance < current_distance;
        }),
      new_pos_(std::move(new_pos))
  {
  }

private:
  std::function<position()> new_pos_;
};

class is_goal_left : public is_goal_somewhere
{
public:
  is_goal_left()
    : is_goal_somewhere(
        "is_goal_left",
        []
        {
          return active_sim->ahead(active_sim->pos(),
                                   d_turn_left(active_sim->dir()),
                                   1);
        })
  {
  }
};

class is_goal_right : public is_goal_somewhere
{
public:
  is_goal_right()
    : is_goal_somewhere(
        "is_goal_right",
        []
        {
          return active_sim->ahead(active_sim->pos(),
                                   d_turn_right(active_sim->dir()),
                                   1);
        })
  {
  }
};

class is_goal_ahead : public is_goal_somewhere
{
public:
  is_goal_ahead()
    : is_goal_somewhere("is_goal_ahead",
                        []
                        {
                          return active_sim->ahead(1);
                        })
  {
  }
};

class is_goal_behind : public is_goal_somewhere
{
public:
  is_goal_behind()
    : is_goal_somewhere("is_goal_behind",
                        []
                        {
                          return active_sim->ahead(-1);
                        })
  {
  }
};

/*********************************************************************
 *  Graphics
 *********************************************************************/
class framework
{
public:
  framework(unsigned width, unsigned height)
  {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(width, height, 0, &win, &renderer);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
  }

  operator SDL_Renderer *() { return renderer; }
  operator bool() const { return win && renderer; }

  int height() const
  {
    int ret;
    SDL_GetWindowSize(win, nullptr, &ret);
    return ret;
  }

  int width() const
  {
    int ret;
    SDL_GetWindowSize(win, &ret, nullptr);
    return ret;
  }

  ~framework()
  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
  }

private:
  SDL_Window *win {nullptr};
  SDL_Renderer *renderer {nullptr};
};

void draw_goal(framework &fw, const simulation &s)
{
  const int cell_h(fw.height() / s.map().height());
  const int cell_w(fw.width() / s.map().width());

  SDL_Rect r({cell_w * s.goal().x, cell_h * s.goal().y,
              cell_w, cell_h});

  int cidx(0);
  SDL_Color ca[] = {{0, 0, 0, 255}, {255, 255, 0, 255}};

  for (int i(0); r.w > 0 && r.h > 0; ++i)
  {
    SDL_SetRenderDrawColor(fw, ca[cidx].r, ca[cidx].g, ca[cidx].b,
                           ca[cidx].a);
    SDL_RenderFillRect(fw, &r);

    cidx = (cidx + 1) % 2;
    ++r.x;
    ++r.y;
    r.h -= 2;
    r.w -= 2;
  }
}

void draw_trajectory(
  framework &fw,
  const simulation &s,
  const std::vector<std::pair<position, direction>> &trajectory)
{
  const int cell_h(fw.height() / s.map().height());
  const int cell_w(fw.width() / s.map().width());

  for (unsigned i(1); i < trajectory.size(); ++i)
  {
    const auto &[p0, d0] = trajectory[i - 1];
    const auto &[p1, d1] = trajectory[i];

    if (p0 == s.goal())
      break;

    const SDL_Point start_p{cell_w / 2 + cell_w * p0.x,
                            cell_h / 2 + cell_h * p0.y};


    SDL_Point end_p(start_p);

    const int scale(p0 != p1 ? 1 : 2);

    switch (d0)
    {
    case north:
      end_p.y -= cell_h / scale;
      break;

    case east:
      end_p.x += cell_w / scale;
      break;

    case south:
      end_p.y += cell_h / scale;
      break;

    case west:
      end_p.x -= cell_w / scale;
      break;
    }

    SDL_SetRenderDrawColor(fw, 255, 255, 255, 255);
    SDL_RenderDrawLine(fw, start_p.x, start_p.y, end_p.x, end_p.y);

    SDL_SetRenderDrawColor(fw, 255, 0, 0, 255);
    SDL_Rect r({end_p.x - 1, end_p.y - 1, 3, 3});
    SDL_RenderFillRect(fw, &r);

    SDL_RenderPresent(fw);
    SDL_Delay(30);
  }
}

void draw_agent(framework &fw, const simulation &s, position p, direction d)
{
  const int cell_h(fw.height() / s.map().height());
  const int cell_w(fw.width() / s.map().width());
  const SDL_Point tl{cell_w * p.x, cell_h * p.y};

  SDL_Point start_p(tl), end_p(tl);
  switch (d)
  {
  case north:
    start_p.x += cell_w / 2;
    start_p.y += cell_h;
    end_p.x += cell_w / 2;
    if (p == s.goal())
      end_p.y += cell_h / 2;
    break;

  case east:
    start_p.y += cell_h / 2;
    if (p == s.goal())
      end_p.x += cell_w / 2;
    else
      end_p.x += cell_w;
    end_p.y += cell_h / 2;
    break;

  case south:
    start_p.x += cell_w /2 ;
    end_p.x += cell_w / 2;
    if (p == s.goal())
      end_p.y += cell_h / 2;
    else
      end_p.y += cell_h;
    break;

  case west:
    start_p.x += cell_w;
    start_p.y += cell_h / 2;
    end_p.y += cell_h / 2;
    if (p == s.goal())
      end_p.x += cell_w / 2;
    break;
  }

  SDL_SetRenderDrawColor(fw, 255, 255, 255, 255);
  SDL_RenderDrawLine(fw, start_p.x, start_p.y, end_p.x, end_p.y);

  SDL_SetRenderDrawColor(fw, 255, 0, 0, 255);
  SDL_Rect r({end_p.x - 1, end_p.y - 1, 3, 3});
  SDL_RenderFillRect(fw, &r);

  SDL_RenderPresent(fw);
  SDL_Delay(3);
}

bool render_simulation(const vita::i_mep *prg = nullptr)
{
  const int map_h(active_sim->map().height());
  const int map_w(active_sim->map().width());

  const int cell_size(15);

  const int h(map_h * cell_size), w(map_w * cell_size);

  static framework fw(w, h);

  for (int y(0); y < map_h; ++y)
    for (int x(0); x < map_w; ++x)
    {
      SDL_Rect r({x * cell_size, y * cell_size,
                  cell_size, cell_size});

      SDL_Color c(t_color(active_sim->terrain_at(position(y, x))));

      SDL_SetRenderDrawColor(fw, c.r, c.g, c.b, c.a);
      SDL_RenderFillRect(fw, &r);

      if (active_sim->terrain_at(position(y, x)) == enemy)
      {
        r = {r.x + r.w /4, r.y + r.h / 4,
             r.w / 2, r.h / 2};
        SDL_SetRenderDrawColor(fw, 0, 0, 0, 255);
        SDL_RenderFillRect(fw, &r);
      }
    }

  draw_goal(fw, *active_sim);

  //for (const auto &[p, d] : active_sim->agent().trajectory())
  //  draw_agent(fw, *active_sim, p, d);

  draw_trajectory(fw, *active_sim, active_sim->agent().trajectory());

  //static Mat program_image;
  //if (prg)
  //{
  //  program_image = Mat::zeros(w, h, CV_8UC3);
  //  std::ostringstream out;
  //  out << vita::out::c_language << *prg;

  //  cv::putText(program_image, out.str(),
  //              Point(0, 30),
  //              cv::FONT_HERSHEY_SIMPLEX,
  //              1,
  //              Scalar(0, 255, 0),
  //              1);
  //}

  SDL_RenderPresent(fw);

  SDL_Event event;
  SDL_PollEvent(&event);
  return event.type != SDL_QUIT;
}

/*********************************************************************
 *  Vita related code
 *********************************************************************/
vita::fitness_t execute_program(const vita::i_mep &prg, bool render)
{
  simulation *const base_sim(active_sim);

  double total_cost(0.0);

  static std::vector<std::pair<position, position>> start_goal;
  if (start_goal.empty())
    for (unsigned i(0); i < 100; ++i)
    {
      position start, goal;
      do
      {
        start = position(vita::random::sup(base_sim->map().height()),
                         vita::random::sup(base_sim->map().width()));
        goal = position(vita::random::sup(base_sim->map().height()),
                        vita::random::sup(base_sim->map().width()));
      } while (start == goal
               || base_sim->terrain_at(start) == unpassable
               || base_sim->terrain_at(goal) == unpassable);

      start_goal.emplace_back(start, goal);
    }

  for (const auto &[start, goal] : start_goal)
  {
    simulation sim(*base_sim);
    active_sim = &sim;

    sim.pos(start);
    sim.goal(goal);

    unsigned max_cycles(1000), same_state(0);
    for (unsigned j(0);
         j < max_cycles && sim.pos() != sim.goal() && same_state < 4;
         ++j)
    {
      vita::run(prg);

      for (const auto &[p, d] : sim.agent().trajectory())
        if (p == sim.pos() && d == sim.dir())
          ++same_state;
    }

    if (sim.pos() == sim.goal())
    {
      for (const auto &[p, d] : sim.agent().trajectory())
        total_cost -= t_penalty(sim.terrain_at(p));
    }
    else
    {
      const auto distance_to_goal(distance(sim.pos(), sim.goal()));
      total_cost -= static_cast<double>(10000 + distance_to_goal);
    }

    if (render)
      render = render_simulation(&prg);

    active_sim = base_sim;
  }

  return {total_cost};
}

class evaluator : public vita::evaluator<vita::i_mep>
{
public:
  vita::fitness_t operator()(const vita::i_mep &x) override
  {
    return execute_program(x, false);
  }
};

// It's important to note that we are not evolving the path (which would be a
// GA problem) from start to finish, but the algorithm which finds the path. In
// other words, the evolved solution is the program which solves the problem.
// Consequently, a good solution should find an efficient path regardless of
// the map it is run on.
//
// The agent is unaware of the map as a whole, so the "best path" is the
// shortest given the knowledge available.
//
int main()
{
  vita::problem prob;

  prob.insert<is_water_ahead1>();
  prob.insert<is_water_ahead2>();
  prob.insert<is_water_ahead3>();
  prob.insert<is_enemy_ahead>();
  prob.insert<is_blocked_ahead1>();
  prob.insert<is_blocked_ahead2>();
  prob.insert<is_blocked_ahead3>();
  prob.insert<is_steeper_ahead>();
  prob.insert<is_less_steep_ahead>();
  prob.insert<is_goal_left>();
  prob.insert<is_goal_right>();
  prob.insert<is_goal_ahead>();
  prob.insert<is_goal_behind>();

  prob.insert<move_forward>();
  prob.insert<turn_left>();
  prob.insert<turn_right>();

  prob.insert<do_both>();

  vita::search<vita::i_mep, vita::alps_es> s(prob);
  s.training_evaluator<evaluator>();

  vita::i_mep best_so_far;
  s.after_generation([&best_so_far](const auto &, const auto &stat)
                     {
                       if (stat.best.solution != best_so_far)
                       {
                         best_so_far = stat.best.solution;
                         execute_program(best_so_far, true);
                       }
                     });

  simulation sim(map("map.txt"));
  active_sim = &sim;

  prob.env.generations = 10000;
  const auto result(s.run());

  std::cout << "\nCANDIDATE SOLUTION\n"
            << vita::out::c_language << result.best.solution
            << "\n\nFITNESS\n" << result.best.score.fitness << '\n';
}
