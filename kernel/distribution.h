/**
 *
 *  \file distribution.h
 *
 *  \author Manlio Morini
 *  \date 2011/04/20
 *
 *  This file is part of VITA
 *
 */
  
#if !defined(DISTRIBUTION_H)
#define      DISTRIBUTION_H

#include <cmath>
#include <map>

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
    double entropy() const;

    bool check() const;

    boost::uint64_t count;
  
    T     mean;
    T variance;
    T      min;
    T      max;
    
  private:
    void update_variance(T);

    std::map<T, unsigned> freq;

    T delta;
    T    m2;
  };


  template<class T>
  distribution<T>::distribution()
  {
    clear();
  }

  ///
  /// Resets gathered statics.
  ///
  template<class T>
  void
  distribution<T>::clear()
  {
    count = 0;

    delta = m2 = mean = variance = min = max = 0.0;

    freq.clear();
  }

  ///
  /// \param[in] val new fitness upon which statistics are recalculated.
  ///
  /// Gathers a new fitness for the statistics.
  ///
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
    ++freq[val];

    update_variance(val);
  }    

  ///
  /// \return the entropy of the distribution.
  ///
  /// \fH(X)=-\sum_{i=1}^n p(x_i) \dot log_b(p(x_i))
  ///
  template<class T>
  double
  distribution<T>::entropy() const
  {
    const double c(1.0/std::log(2.0));

    double h(0.0);
    for (typename std::map<T, unsigned>::const_iterator j(freq.begin());
         j != freq.end();
         ++j)
    {
      const double p(double(j->second) / count);
      h -= p * std::log(p) * c;
    }

    return h;
  }

  ///
  /// \param[in] val new fitness upon which statistics are recalculated.
  ///
  /// Calculate running variance and cumulative average fitness. The
  /// algorithm used is due to Knuth (Donald E. Knuth - The Art of Computer
  /// Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232. 
  /// Addison-Wesley)
  ///
  template<class T>
  void
  distribution<T>::update_variance(T val)
  {   
    delta = val - mean;
    mean += delta/count;
      
    // This expression uses the new value of mean.
    m2 += delta * (val-mean);

    variance = m2 / count;
  }    

  ///
  /// \return The standard deviation of the distribution.
  ///
  template<class T>
  T
  distribution<T>::standard_deviation() const
  {
    return std::sqrt(variance);
  }

  ///
  /// \return true if the object passes the internal consistency check.
  ///
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
