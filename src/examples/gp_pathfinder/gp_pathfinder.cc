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

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <future>
#include <iostream>
#include <optional>
#include <queue>
#include <vector>

#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui_impl_sdl2.h"
#include "third_party/imgui/imgui_impl_sdlrenderer.h"
#include <SDL2/SDL.h>

#include "kernel/vita.h"

/*********************************************************************
 *  Thread safe queue
 *********************************************************************/
template <class T>
class ts_queue
{
public:
  void push(T item)
  {
    {
      std::lock_guard lock(mutex_);
      queue_.push(item);
    }
    cond_.notify_one();
  }

  T pop()
  {
    std::unique_lock lock(mutex_);

    // Wait until queue is not empty.
    cond_.wait(lock, [this] { return !queue_.empty(); });

    // Retrieve item.
    const T item(queue_.front());
    queue_.pop();

    return item;
  }

  bool empty() const
  {
    std::lock_guard lock(mutex_);
    return queue_.empty();
  }

  std::optional<T> try_pop()
  {
    using namespace std::chrono_literals;
    std::lock_guard lock(mutex_);

    if (queue_.empty())
      return {};

    // Retrieve item.
    const T item(queue_.front());
    queue_.pop();
    return item;
  }

private:
  std::queue<T> queue_ {};           // underlying queue
  mutable std::mutex mutex_ {};      // for thread synchronization
  std::condition_variable cond_ {};  // for signaling
};  // ts_queue

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

ImColor t_color(terrain t)
{
  switch (t)
  {
  case enemy:          return {255, 0, 0};
  case water:          return {0, 0, 255};
  case flat:           return {86, 125, 70};
  case nearly_flat:    return {255, 255, 153};
  case gentle_slope:   return {255, 255, 102};
  case moderate_slope: return {255, 255, 51};
  case strong_slope:   return {181, 101, 29};
  case extreme_slope:  return {101, 67, 33};
  case steep_slope:    return {43, 29, 20};
  default:             return {0, 0, 0};
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
  default:             return 1000000;
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
    pos(p);
    trajectory_.emplace_back(pos(), dir());
  }

  void turn_right()
  {
    dir_ = d_turn_right(dir_);
    trajectory_.emplace_back(pos(), dir());
  }

  void turn_left()
  {
    dir_ = d_turn_left(dir_);
    trajectory_.emplace_back(pos(), dir());
  }

  using trajectory_t = std::vector<std::pair<position, direction>>;
  const trajectory_t &trajectory() const
  {
    return trajectory_;
  }

private:
  position p_ {};
  direction dir_ {direction::east};

  trajectory_t trajectory_ {};
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

  position ahead(int steps = 1) const
  {
    return ahead(pos(), dir(), steps);
  }

  terrain terrain_at(position p) const
  {
    return map_[p];
  }

  void move_forward()
  {
    if (const position p1 = ahead();
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
  explicit action(const std::string &n, simulation &s) : vita::terminal(n),
                                                         sim_(&s)
  {}

  simulation *sim_ {nullptr};
};

class move_forward : public action
{
public:
  explicit move_forward(simulation &s) : action("move_forward", s) {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    sim_->move_forward();
    return {};
  }
};

class turn_right : public action
{
public:
  explicit turn_right(simulation &s) : action("turn_right", s) {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    sim_->turn_right();
    return {};
  }
};

class turn_left : public action
{
public:
  explicit turn_left(simulation &s) : action("turn_left", s) {}

  vita::value_t eval(vita::symbol_params &) const final
  {
    sim_->turn_left();
    return {};
  }
};

class do_both : public vita::function
{
public:
  do_both() : vita::function("do_both", 2) {}

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
  sensor(const std::string &n, simulation &s) : vita::function(n, 0, {0, 0}),
                                                sim_(&s)
  {}

  simulation *sim_ {nullptr};
};

class is_something_happening : public sensor
{
protected:
  is_something_happening(const std::string &m, simulation &s,
                         std::function<bool()> c)
    : sensor(m, s), check_(std::move(c))
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
  is_something_ahead(const std::string &m, simulation &s)
    : is_something_happening(
        m, s,
        [this]
        {
          return sim_->terrain_at(sim_->ahead(D)) == T;
        })
  {
  }
};

// Common code for is_water_ahead1, is_water_ahead2, is_water_ahead3.
template<int D> class is_water_ahead
  : public is_something_ahead<terrain::water, D>
{
protected:
  explicit is_water_ahead(simulation &s)
    : is_water_ahead::is_something_ahead("is_water_ahead" + std::to_string(D),
                                         s)
  {
  }
};

class is_water_ahead1 : public is_water_ahead<1>
{
public:
  explicit is_water_ahead1(simulation &s) : is_water_ahead(s) {}
};

class is_water_ahead2 : public is_water_ahead<2>
{
public:
  explicit is_water_ahead2(simulation &s) : is_water_ahead(s) {}
};

class is_water_ahead3 : public is_water_ahead<3>
{
public:
  explicit is_water_ahead3(simulation &s) : is_water_ahead(s) {}
};

// Detects enemy units one square ahead.
class is_enemy_ahead : public is_something_ahead<terrain::enemy, 1>
{
public:
  explicit is_enemy_ahead(simulation &s)
    : is_something_ahead<terrain::enemy, 1>("is_enemy_ahead", s)
  {
  }
};

// Common code for is_blocked_ahead1, is_blocked_ahead2, is_blocked_ahead3.
template<int D> class is_blocked_ahead
  : public is_something_ahead<terrain::unpassable, D>
{
protected:
  explicit is_blocked_ahead(simulation &s)
    : is_blocked_ahead::is_something_ahead("is_blocked_ahead", s)
  {
  }
};

class is_blocked_ahead1 : public is_blocked_ahead<1>
{
public:
  explicit is_blocked_ahead1(simulation &s) : is_blocked_ahead(s) {}
};

class is_blocked_ahead2 : public is_blocked_ahead<2>
{
public:
  explicit is_blocked_ahead2(simulation &s) : is_blocked_ahead(s) {}
};

class is_blocked_ahead3 : public is_blocked_ahead<3>
{
public:
  explicit is_blocked_ahead3(simulation &s) : is_blocked_ahead(s) {}
};

// `true` if penalty ahead greater than current penalty.
class is_steeper_ahead : public is_something_happening
{
public:
  explicit is_steeper_ahead(simulation &s)
    : is_something_happening(
        "is_steeper_ahead", s,
        [this]
        {
          return t_penalty(sim_->terrain_at(sim_->ahead()))
                 > t_penalty(sim_->terrain_at(sim_->pos()));
        })
      {
      }
};

// `true` if penalty ahead is less than current penalty.
class is_less_steep_ahead : public is_something_happening
{
public:
  explicit is_less_steep_ahead(simulation &s)
    : is_something_happening(
        "is_less_steep_ahead", s,
        [this]
        {
          return t_penalty(sim_->terrain_at(sim_->ahead()))
                 < t_penalty(sim_->terrain_at(sim_->pos()));
        })
      {
      }
};

// Tests relative direction of goal.
class is_goal_somewhere : public is_something_happening
{
protected:
  is_goal_somewhere(const std::string &n, simulation &s,
                    std::function<position()> new_pos)
    : is_something_happening(
        n, s,
        [this]
        {
          const auto current_distance(distance(sim_->pos(),
                                               sim_->goal()));
          const auto new_distance(distance(new_pos_(), sim_->goal()));

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
  explicit is_goal_left(simulation &s)
    : is_goal_somewhere(
        "is_goal_left", s,
        [this]
        {
          return sim_->ahead(sim_->pos(), d_turn_left(sim_->dir()), 1);
        })
  {
  }
};

class is_goal_right : public is_goal_somewhere
{
public:
  explicit is_goal_right(simulation &s)
    : is_goal_somewhere(
        "is_goal_right", s,
        [this]
        {
          return sim_->ahead(sim_->pos(), d_turn_right(sim_->dir()), 1);
        })
  {
  }
};

class is_goal_ahead : public is_goal_somewhere
{
public:
  explicit is_goal_ahead(simulation &s)
    : is_goal_somewhere("is_goal_ahead", s,
                        [this]
                        {
                          return sim_->ahead();
                        })
  {
  }
};

class is_goal_behind : public is_goal_somewhere
{
public:
  explicit is_goal_behind(simulation &s)
    : is_goal_somewhere("is_goal_behind", s,
                        [this]
                        {
                          return sim_->ahead(-1);
                        })
  {
  }
};

/*********************************************************************
 *  Vita related code
 *********************************************************************/
class simulation_problem : public vita::problem
{
public:
  explicit simulation_problem(const map &m) : sim_(m)
  {
    // Sensors.
    insert<is_water_ahead1>(sim_);
    insert<is_water_ahead2>(sim_);
    insert<is_water_ahead3>(sim_);
    insert<is_enemy_ahead>(sim_);
    insert<is_blocked_ahead1>(sim_);
    insert<is_blocked_ahead2>(sim_);
    insert<is_blocked_ahead3>(sim_);
    insert<is_steeper_ahead>(sim_);
    insert<is_less_steep_ahead>(sim_);
    insert<is_goal_left>(sim_);
    insert<is_goal_right>(sim_);
    insert<is_goal_ahead>(sim_);
    insert<is_goal_behind>(sim_);

    // Terminals.
    insert<move_forward>(sim_);
    insert<turn_left>(sim_);
    insert<turn_right>(sim_);

    insert<do_both>();  // ... and the "special" `do_both`
  }

  struct execution_result
  {
    // A sequence of {trajectory, goal} pairs.
    std::vector<std::pair<agent::trajectory_t, position>> trace {};

    // The attained fitness considering multiple attempts.
    vita::fitness_t                                     fitness {};
  };

  execution_result execute_program(const vita::i_mep &);

  execution_result best() const { return best_; }

private:
  simulation sim_;

  execution_result best_ {};
};

simulation_problem::execution_result simulation_problem::execute_program(
  const vita::i_mep &prg)
{
  const simulation backup_sim(sim_);

  static std::vector<std::pair<position, position>> start_goal;
  if (start_goal.empty())
    for (unsigned i(0); i < 100; ++i)
    {
      position start, goal;
      do
      {
        start = position(vita::random::sup(sim_.map().height()),
                         vita::random::sup(sim_.map().width()));
        goal = position(vita::random::sup(sim_.map().height()),
                        vita::random::sup(sim_.map().width()));
      } while (start == goal
               || sim_.terrain_at(start) == unpassable
               || sim_.terrain_at(goal) == unpassable);

      start_goal.emplace_back(start, goal);
    }

  execution_result res;

  double total_cost(0.0);

  for (const auto &[start, goal] : start_goal)
  {
    sim_.pos(start);
    sim_.goal(goal);

    const unsigned max_cycles(sim_.map().width() * sim_.map().height());
    for (unsigned j(0); j < max_cycles && sim_.pos() != goal; ++j)
    {
      vita::run(prg);

      unsigned same_state(0);
      for (const auto &[p, d] : sim_.agent().trajectory())
        if (p == sim_.pos() && d == sim_.dir())
          ++same_state;
      if (same_state >= 4)
        break;
    }

    if (sim_.pos() == goal)
    {
      for (const auto &[p, d] : sim_.agent().trajectory())
        total_cost -= t_penalty(sim_.terrain_at(p));
    }
    else
    {
      const auto distance_to_goal(distance(sim_.pos(), goal));
      total_cost -= static_cast<double>(50000 + distance_to_goal);
    }

    res.trace.push_back({sim_.agent().trajectory(), goal});

    sim_ = backup_sim;
  }

  res.fitness = {total_cost};

  if (res.fitness > best().fitness)
    best_ = res;

  return res;
}

class path_evaluator : public vita::evaluator<vita::i_mep>
{
public:
  explicit path_evaluator(simulation_problem &sp) : sp_(&sp) {}

  [[nodiscard]] vita::fitness_t operator()(const vita::i_mep &x) override
  {
    return sp_->execute_program(x).fitness;
  }

private:
  simulation_problem *sp_;
};

struct info
{
  simulation_problem::execution_result er {};
  std::string best_program {};
  unsigned generation {};
};

ts_queue<info> s_queue;

// It's important to note that we are not evolving the path (which would be a
// GA problem) from start to finish, but the algorithm which finds the path. In
// other words, the evolved solution is the program which solves the problem.
// Consequently, a good solution should find an efficient path regardless of
// the map it is run on.
//
// The agent is unaware of the map as a whole, so the "best path" is the
// shortest given the knowledge available.
//
vita::summary<vita::i_mep> search(map m)
{
  simulation_problem prob(m);

  vita::search<vita::i_mep, vita::alps_es> s(prob);
  s.training_evaluator<path_evaluator>(prob);

  vita::i_mep best_so_far;
  s.after_generation([&](const auto &, const auto &stat)
                     {
                       if (stat.best.solution != best_so_far)
                       {
                         best_so_far = stat.best.solution;

                         info i;
                         i.er = prob.best();

                         std::ostringstream out;
                         out << vita::out::c_language << stat.best.solution;
                         i.best_program = out.str();

                         i.generation = stat.gen;

                         s_queue.push(i);
                       }
                     });

  prob.env.generations = 10000;
  return s.run();
}

/*********************************************************************
 *  Graphics
 *********************************************************************/
class framework
{
public:
  framework(unsigned width, unsigned height)
  {
    const auto flags(static_cast<SDL_WindowFlags>(
                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI));

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_CreateWindowAndRenderer(width, height, flags, &win_, &renderer_);
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);
  }

  operator SDL_Renderer *() { return renderer_; }
  operator bool() const { return win_ && renderer_; }

  SDL_Window *window() { return win_; }

  int height() const
  {
    int ret;
    SDL_GetWindowSize(win_, nullptr, &ret);
    return ret;
  }

  int width() const
  {
    int ret;
    SDL_GetWindowSize(win_, &ret, nullptr);
    return ret;
  }

  ~framework()
  {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window());
    SDL_Quit();
  }

private:
  SDL_Window *win_ {nullptr};
  SDL_Renderer *renderer_ {nullptr};
};

class imgui_framework : public framework
{
public:
  imgui_framework(unsigned width, unsigned height)
    : framework(width, height)
  {
#if !SDL_VERSION_ATLEAST(2,0,17)
#  error This backend requires SDL 2.0.17+
#endif

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io(ImGui::GetIO());
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window(), *this);
    ImGui_ImplSDLRenderer_Init(*this);
  }

  void render()
  {
    ImGui::Render();

    ImGuiIO &io(ImGui::GetIO());
    SDL_RenderSetScale(*this,
                       io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    SDL_SetRenderDrawColor(*this,
                           (Uint8)(clear_color.x * 255),
                           (Uint8)(clear_color.y * 255),
                           (Uint8)(clear_color.z * 255),
                           (Uint8)(clear_color.w * 255));

    SDL_RenderClear(*this);
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(*this);
  }

  ~imgui_framework()
  {
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }
};

struct running_options_t
{
  map m {};
  int run_program {-1};
} running_options;

void render_ui(const std::vector<info> &iv)
{
  if (!ImGui::Begin("Data"))
  {
    // Early out if the window is collapsed, as an optimization.
    ImGui::End();
    return;
  }

  if (ImGui::CollapsingHeader("Programs"))
  {
    for (std::size_t i(iv.size()); i-- > 0;)
    {
      if (ImGui::TreeNode(std::to_string(iv[i].generation).c_str()))
      {
        ImGui::TextWrapped("%s", iv[i].best_program.c_str());
        ImGui::TreePop();
      }

      if (running_options.run_program < 0)
      {
        ImGui::SameLine();
        ImGui::SmallButton("Run");
        if (ImGui::IsItemClicked())
          running_options.run_program = i;
      }
    }

    ImGui::TreePop();
  }

  ImGui::End();

  ImGui::ShowDemoWindow();
}

void render_prg(const agent::trajectory_t &trj, std::size_t up_to,
                position goal)
{
  ImGui::Begin("Map");
  ImDrawList *draw_list(ImGui::GetWindowDrawList());

  const map &m(running_options.m);
  const int map_h(m.height());
  const int map_w(m.width());

  const auto cur_pos(ImGui::GetCursorScreenPos());
  const auto win_size(ImGui::GetWindowSize());

  const float cell_w((win_size.x - cur_pos.x) / map_w);
  const float cell_h((win_size.y - cur_pos.y) / map_h);

  // MAP with enemy
  for (int y(0); y < map_h; ++y)
    for (int x(0); x < map_w; ++x)
    {
      const auto px(cur_pos.x + x * cell_w), py(cur_pos.y + y * cell_h);
      draw_list->AddRectFilled(ImVec2(px, py),
                               ImVec2(px + cell_w, py + cell_h),
                               t_color(m[position(y, x)]));

      if (m[position(y, x)] == enemy)
      {
        draw_list->AddRectFilled(ImVec2(px + cell_w / 4, py + cell_h / 4),
                                 ImVec2(px + cell_w / 2, py + cell_h / 2),
                                 t_color(m[position(y, x)]));
      }
    }

  // GOAL
  if (goal != position::npos)
  {
    const ImColor col[] = {{0, 0, 0}, {255, 255, 0}};

    unsigned cidx(0);

    ImVec2 tl(cur_pos.x + goal.x * cell_w, cur_pos.y + goal.y * cell_h);
    ImVec2 br(cur_pos.x + goal.x * cell_w + cell_w,
              cur_pos.y + goal.y * cell_h + cell_h);

    while (br.x > tl.x && br.y > tl.y)
    {
      draw_list->AddRectFilled(tl, br, col[cidx]);

      cidx ^= 1;
      ++tl.x;
      ++tl.y;
      --br.x;
      --br.y;
    }
  }

  // TRAJECTORY
  for (const auto &[p, d] : trj)
  {
    if (up_to > 0)
      --up_to;
    else
      break;

    const ImVec2 start_p(cur_pos.x + cell_w / 2 + cell_w * p.x,
                         cur_pos.y + cell_h / 2 + cell_h * p.y);

    ImVec2 end_p(start_p);

    const auto p1(m.ahead(p, d, 1));

    const int scale(m.is_valid(p1) && m[p1] != unpassable && p1 != p ? 1 : 2);

    switch (d)
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

    const ImColor white({255, 255, 255});
    const ImColor red({255, 0, 0});
    draw_list->AddLine(start_p, end_p, white);
    draw_list->AddRectFilled(ImVec2(end_p.x - 1, end_p.y - 1),
                             ImVec2(end_p.x + 1, end_p.y + 1),
                             red);
  }
}

void render_frame(const std::vector<info> &iv, const agent::trajectory_t &trj,
                  std::size_t up_to, position goal)
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
    ImGui_ImplSDL2_ProcessEvent(&event);

  ImGui_ImplSDLRenderer_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  render_ui(iv);

  if (running_options.run_program >= 0)
    render_prg(trj, up_to, goal);

  ImGui::End();
}

void render(imgui_framework &fw, const std::vector<info> &iv)
{
  if (running_options.run_program >= 0)
  {
    for (const auto &[trj, goal] : iv[running_options.run_program].er.trace)
    {
      for (std::size_t up_to(0); up_to < trj.size(); ++up_to)
      {
        render_frame(iv, trj, up_to, goal);
        fw.render();
      }
    }
    running_options.run_program = -1;
  }
  else
  {
    render_frame(iv, {}, 0, position::npos);
    fw.render();
  }
}

int main()
{
  using namespace std::chrono_literals;

  running_options.m = map("map.txt");

  auto result(std::async(std::launch::async, search, running_options.m));

  imgui_framework fw_info(600, 600);

  std::vector<info> info_vec;

  while (result.wait_for(0ms) != std::future_status::ready
         || !s_queue.empty())
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_QUIT)
        return 0;
      if (event.type == SDL_WINDOWEVENT
          && event.window.event == SDL_WINDOWEVENT_CLOSE
          && event.window.windowID == SDL_GetWindowID(fw_info.window()))
        return 0;
    }

    if (const auto i(s_queue.try_pop()); i)
      info_vec.push_back(i.value());;

    render(fw_info, info_vec);
  }

  return 0;
}
