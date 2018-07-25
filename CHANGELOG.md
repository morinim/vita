# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Removed
- standalone Makefile-based build system. Now the only supported build system is CMake. This greatly simplify multi-platform testing and documentation.

### Changed
- **BREAKING CHANGE**. `example::tag` method has been renamed to `label`; `example::cast_output` has been renamed to `label_as`. Both are now free functions.

  The new naming improves consistency and adopts standard terminology.


- **BREAKING CHANGE**. Simpler method to specify GAs parameters.

  Instead of:
  ```C++
  for (unsigned i(0); i < n_jobs; ++i)
    prob.sset.insert(ga::parameter(i, {-0.5, 23.5}));
  ```

  use:

  ```C++
  for (unsigned i(0); i < n_jobs; ++i)
    prob.sset.insert<ga::real>( range(-0.5, 23.5) );
  ```

  Note that:
  - you can omit the parameter index;
  - the range is clearly stated.

  The `ga::parameter` help function has been removed.

## [0.9.14] - 2017-12-11

### Added
- Code of conduct for contributors.
- Complete control of the generational-gap between subset-selection (DSS
  algorithm).
- Simpler setup for GA problems.

### Changed
- symbol_set::roulette() function has the same probability of extracting a
  terminal / a function regardeless of the specific symbols' weights.

  If all symbols have the same probability to appear into a chromosome, there
  could be some problems.
  For instance, if our problem has many variables (let's say 100) and the
  function set has only 4 symbols we cannot get too complex trees because the
  functions have a reduced chance to appear in the chromosome (e.g. it
  happens in the Forex example).

- **BREAKING CHANGE**. Enable/disable logging of specific information is a bit
  simpler.

  Instead of
  ```C++
  env.dynamic = true;
  env_dyn_name = "dynamic.txt";
  ```

  this is enough:

  ```C++
  env.dynamic_file = "dynamic.txt"
  ```

- **BREAKING CHANGE**. `i_mep::compress` function renamed to `i_mep::cse`
  (Common Subexpression Elimination).

- **BREAKING CHANGE**. Serialization file format for cache and fitness has
  changed.

  The new format produces files up to 20% smaller omitting:
  1. the `seal` value which is assumed equal to the `cache::seal` value
  2. the fitness vector size (and assuming that all the components of the
     vector are on the same line)

- **BREAKING CHANGE**. Instead of:

  ```C++
  symbol_set sset;
  environment e(&sset, initialization::standard);

  evolution<i_mep, std_es> evo(env, *eva);
  ```

  you should write:

  ```C++
  problem p(initialization::standard);

  evolution<i_mep, std_es> evo(p, *eva);
  ```

  The `problem` object aggregates an `environment` and a `symbol_set` (which
  are almost always used together).

  This also permits to remove the pointer from the environment to the
  symbol_set.

- **BREAKING CHANGE**. Replaced the `search::set_evaluator` method:
  Instead of:

  ```C++
  std::unique_ptr<E> eva(f);
  std::search engine(prob);
  engine.set_evaluator(std::move(eva));
  ```

  you can write:

  ```C++
  std::search engine(prob);
  engine.set_evaluator<E>(f);
  ```

- Uniform output results for make / cmake build systems.


## [0.9.13] - 2017-09-15

### Added
- New printing system for i_mep individuals. You can now export C / C++ / Python / MQL code.
- Self-adaptive crossover operator.
- Forex example.

### Changed
- More robust implementation for real functions.
- New uniform crossover implementation.
- ALPS can remove multiple identical layers.

### Fixed
- Various compilation warning with recent version of gcc (7.x).


[Unreleased]: https://github.com/morinim/vita/compare/v0.9.14...HEAD
[0.9.14]: https://github.com/morinim/vita/compare/v0.9.13...v0.9.14
[0.9.13]: https://github.com/morinim/vita/compare/v0.9.12...v0.9.13
