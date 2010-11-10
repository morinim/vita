/**
 *
 *  \file random.h
 *
 *  \author Manlio Morini
 *  \date 2009/09/14
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(RANDOM_H)
#define      RANDOM_H

#include <cstdlib>

#include <boost/random.hpp>

#include "vita.h"

namespace vita
{

  class random
  {
  public:
    template<class T> static T between(T,T);

    static unsigned ring(unsigned, unsigned, unsigned);

    static bool boolean(double);
    static bool boolean();

    static void seed(unsigned long);

  private:
    typedef boost::mt19937 base_gen;

    static base_gen _rng;
  };

  /**
   * seed
   * \param s
   */
  inline
  void
  random::seed(unsigned long s)
  {
    _rng.seed(s);
  }

  /**
   * between.
   * \param min
   * \param sup
   */
  template<>
  inline
  double
  random::between(double min, double sup)
  {
    assert(min < sup);
    
    typedef boost::uniform_real<> dist_t;
    typedef boost::variate_generator<base_gen &, dist_t> gen_t;

    gen_t die(_rng,dist_t(min,sup));
    const double ret(die());
    assert(min <= ret && ret < sup);

    return ret;
  }

  template<class T>
  inline
  T
  random::between(T min, T sup)
  {
    assert(min < sup);
    
    typedef boost::uniform_int<> dist_t;
    typedef boost::variate_generator<base_gen &, dist_t> gen_t;

    gen_t die(_rng,dist_t(min,sup-1));
    const T ret(die());
    assert(min <= ret && ret < sup);

    return ret;
  }

  /**
   * boolean.
   * \param p
   */
  inline
  bool
  random::boolean(double p)
  {
    assert(0 <= p && p <= 1);

    return random::between<double>(0,1) < p;
  }

  /**
   * boolean.
   */
  inline
  bool
  random::boolean()
  {
    return random::between<unsigned>(0,2) != 0;
  }

}  // namespace vita

#endif  // RANDOM_H
