/**
 *
 *  \file distribution.h
 *
 *  \author Manlio Morini
 *  \date 2010/06/10
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(DISTRIBUTION_H)
#define      DISTRIBUTION_H

#include "vita.h"
#include "fitness.h"

namespace vita
{

  template<class T>
  class distribution
  {
  public:
    distribution();

    void clear();

    void add(T);

    T standard_deviation() const;

    bool check() const;

    boost::uint64_t count;
  
    T     mean;
    T variance;
    T      min;
    T      max;
    
  private:
    T delta;
    T    m2;
  };



  /**
   * distribution
   */
  template<class T>
  distribution<T>::distribution()
  {
    clear();
  }

  /**
   * clear
   *
   * Resets gathered statics.
   */
  template<class T>
  void
  distribution<T>::clear()
  {
    count = 0;

    delta = m2 = mean = variance = min = max = 0.0;
  }

  /**
   * add
   * \param[in] val New fitness upon which statistics are recalculated.
   *
   * Gathers a new fitness for the statistics.
   */
  template<class T>
  void
  distribution<T>::add(T val)
  {
    if (!count)
      min = max = val;
    else if (val < min)
      min = val;
    else if (max < val)
      max = val;

    ++count;
    
    // Calculate running variance and cumulative average fitness.
    delta = val - mean;
	
    mean = (mean/count)*(count-1) + val/count;
      
    // This expression uses the new value of mean.
    m2 += delta * (val-mean);

    variance = m2 / count;
  }    

  /**
   * standard_deviation
   * \return The standard deviation of the distribution.
   */
  template<class T>
  T
  distribution<T>::standard_deviation() const
  {
    return std::sqrt(variance);
  }

  /**
   * check
   * \return true if the object passes the internal consistency check.
   */
  template<class T>
  bool
  distribution<T>::check() const
  {
    return
      (is_bad(min) || is_bad(mean) || min <= mean+float_epsilon) &&
      (is_bad(max) || is_bad(mean) || mean <= max+float_epsilon) &&
      (isnan(variance) || 0.0 <= variance+float_epsilon);
  }

}  // Namespace vita

#endif // DISTRIBUTION_H
