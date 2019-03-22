# News
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased]

## [1.0.0] - 2019-03-22

### Added
- Simpler way to setup Genetic Algorithm / Differential Evolution tasks. We now have the `ga_problem` and `de_problem` *facades*. Their constructors cover the standard situation of a sequence of uniform parameters (possibly in distinct numeric intervals).

  Examples are been rewritten to take advantage of the new classes.

### Removed
- `problem::chromosome` help function. It was a bit of a stretch, now instead of:

  ```C++
  class ad_hoc_integer_like_class { /* ... */ };

  vita::problem prob;
  prob.chromosome<ad_hoc_integer_like_class>(nr_of_parameters);
  ```

  you should write:

  ```C++
  vita::ga_problem prob(nr_of_parameters, {min, max});
  ```
- standalone Makefile-based build system. Now the only supported build system is CMake. This greatly simplify multi-platform testing and documentation.

### Changed
- **BREAKING CHANGE**. GA-individuals match integer vectors. This allow a faster management, a simpler access to the genome and simpler code.

  Where we wrote:

  ```C++
  genome[i].as<int>()
  ```

  we now have:

  ```C++
  genome[i]
  ```

- **BREAKING CHANGE**. Completely revised validation strategies. The `search`
  class now allows specifying two evaluation (a.k.a fitness) functions via the
  `set_training_evaluator` and `set_validation_evaluator` methods.

  This:
  - gives users the option to adopt distinct training / validation
    strategies;
  - allows a simpler implementation.

  More details are contained in the the `validation.md` wiki page.

- **BREAKING CHANGE**. Removed switch to default initialize an `environment` object.

  So, instead of:

  ```C++
  environment e(initialization::standard);
  ```

  you should write:

  ```C++
  environment e;
  e.init();  // initialization
  ```

  Usually the initialization is only needed in debug code. The common approach is:
  - left parameters in the environment struct unitialized (auto-tune);
  - leave to the `search::run()` function the task of identifying the
    "best" values.

- **BREAKING CHANGE**. `dataframe::load...` renamed to `dataframe::read...` for uniformity with Pands' dataframe.

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
  // problem prob;
  for (unsigned i(0); i < n_jobs; ++i)
    prob.sset.insert<ga::real>( range(-0.5, 23.5) );
  ```

  or equivalently:
  ```C++
  // de_problem prob;
  for (unsigned i(0); i < n_jobs; ++i)
    prob.insert( range(-0.5, 23.5) );
  ```

  or, even better:
  ```C++
  de_problem prob(n_jobs, {-0.5, 23.5});
  ```

  Note that:
  - the parameter index doesn't appear anymore;
  - the range is clearly stated.

  The same approach applies to GAs.

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


[Unreleased]: https://github.com/morinim/vita/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/morinim/vita/compare/v0.9.14...v1.0.0
[0.9.14]: https://github.com/morinim/vita/compare/v0.9.13...v0.9.14
[0.9.13]: https://github.com/morinim/vita/compare/v0.9.12...v0.9.13
