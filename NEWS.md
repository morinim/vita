# News
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).

## [Unreleased]

### Added
- Support for concurrent access to the fitness cache (hash table). The current solution uses a global [(shared) lock](https://en.cppreference.com/w/cpp/thread/shared_lock).

  This approach is simple and allows preliminary parallel search extensions; furthermore performance of single-thread code isn't affected. It lacks [granularity](https://en.wikipedia.org/wiki/Lock_%28computer_science%29#Granularity).

  In future an array of mutexes could be used to minimize contention.

### Changed
- **BREAKING CHANGE**. `hits` and `probes` statistical information were hardly used and have been removed. The removal implies a (minor) change in the saving format of the cache.
- **BREAKING CHANGE**. `gene::arg_locus` has been renamed to `locus_of_argument`. This should only affect secondarly the end user since the function is mainly used in the meanderings of the `i_mep` class.
- **BREAKING CHANGE**. Implementing user defined functions is simpler. Instead of:

  ```C++
  value_t eval(core_interpreter *i) const final
  {
    const auto arg(static_cast<interpreter<i_mep> *>(i)->fetch_arg(0));
    if (has_value(a)) { /* do something with `arg` */ }
    // ...
  }
  ```

  you can simply write:

  ```C++
  value_t eval(symbol_params &args) const final
  {
    const auto arg(args[0]);
    if (has_value(a)) { /* do something with `arg` */ }
	// ...
  }
  ```

  Note:
  - `symbol_params` gives access to a subset of the interface of a `core_interpreter` so the user doesn't have to know unnecessary details about interpreters;
  - `symbol_params::operator[](unsigned)` is equivalent to `symbol_params::fetch_arg(unsigned)` (syntactic sugar).

## [2.1.0] - 2020-11-23

### Added
- Base support for user-provided datasets. Part of the classes/functions for symbolic regression can now work with a generic STL container. The new features are described in the extended [symbolic regression example](https://github.com/morinim/vita/wiki/symbolic_regression).

### Changed
- `sum_of_errors_evaluator` now uses a running average technique (numerically more stable). This could produce minor differences in a specific test case (because of floating point arithmetic) but the general behavior doesn't change.

### Deprecated
- Support for Genetic Programming with Adaptive representations will be removed and replaced with (a sort of) Straight Line Program structure. The new approach is simpler and fits easily in the current architecture.

## [2.0.0] - 2020-11-02

### Added
- New `dataframe::front()` public member function which allows simple access to the first element of a dataframe.
- `dataframe::read_csv` now supports a header row. Also the output column can be specified (before it always was the first column):

  ```C++
  dataframe d;

  dataframe::params p;
  p.has_header = true;
  p.output_index = 2;

  d.read_csv("filename.csv", p);
  ```

### Changed
- **BREAKING CHANGE**. Sources require a C++17 compatible compiler.
- **BREAKING CHANGE**. The `interpreter` class performs calculation using `std::variant` insted of `std::any`.

  This gives a performance improvements without important limitations of use cases.

- **BREAKING CHANGE**. `dataframe::get_column` and `dataframe::columns` have been removed / merged. The `dataframe::columns` public data member contains information about all the columns of the dataframe.

  So:

  1. `d.columns.size()` replaces `d.columns()`
  2. `d.columns[3].name` replaces `d.get_column(3).name`
  3. `d.columns[3].category_id` replaces `d.get_column(3).category_id`



## [1.1.0] - 2019-12-11

**This is the last C++14 compatible version. Next versions of the framework will assume a C++17 compiler.**

### Changed
- **BREAKING CHANGE**. Fluent interface for the search / src_search / ga_search classes.

  Insted of:

  ```C++
  search<...> engine(...);
  engine.set_training_evaluator<...>(...);
  engine.set_validation_evaluator<...>(...);
  engine.set_validator<...>(...);
  ```

  you should write:

  ```C++
  search<...> engine(...)
              .training_evaluator<...>(...)
			  .validation_evaluator<...>(...)
			  .validation_strategy<...>(...);
  ```

  This benefits the class user by reducing the amount he has to type and makes the interface more natural.

- **BREAKING CHANGE**. Removed `read_csv(filename)` / `read_xrff(filename)` from the interface of the dataframe class.

  Both the member functions can be replaced with the already existing `dataframe::read(filename)` which uses the filename extension to identify the data format.

- Clearer format when exporting individuals.

  **Part** of the redundant parentheses has been removed.

  So, instead of `((sin(X1)) + (ln(((X1)+(X2)))` you get `(sin(X1) + ln(X1+X2))`



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


[Unreleased]: https://github.com/morinim/vita/compare/v2.1.0...HEAD
[2.1.0]: https://github.com/morinim/vita/compare/v2.0.0...v2.1.0
[2.0.0]: https://github.com/morinim/vita/compare/v1.1.0...v2.0.0
[1.1.0]: https://github.com/morinim/vita/compare/v1.1.0...v1.0.0
[1.0.0]: https://github.com/morinim/vita/compare/v0.9.14...v1.0.0
[0.9.14]: https://github.com/morinim/vita/compare/v0.9.13...v0.9.14
[0.9.13]: https://github.com/morinim/vita/compare/v0.9.12...v0.9.13
