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

#include "random.h"

namespace vita
{

  random::base_gen random::_rng(28071973u);

  /**
   * ring.
   * \param base[in]
   * \param width[in]
   * \param n[in]
   * \return
   */
  unsigned
  random::ring(unsigned base, unsigned width, unsigned n)
  {
    assert(width && n>1);

    if (base >= n)
      return random::between<unsigned>(0,n);

    if (width > n)
      width = n;

    const unsigned offset(n+base - width/2);
    
    return (offset + random::between<unsigned>(0,width)) % n;
  }

}  // Namespace vita
