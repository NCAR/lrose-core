// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  SimMath.hh
 * @brief  
 */

# ifndef    SIM_MATH_HH

#include <cmath>

#define LOC_PI 3.14159
#define RADIANS(deg) ((deg)*LOC_PI/180.)
#define RADIANS2DEGREES(r) ((r)*180.0/LOC_PI)

#define KNOTS_TO_MS 0.51444
#define KM_TO_METERS 1000.0
#define MINUTES_TO_SECONDS 60.0

// uniformly distributed in the interval [0,f]
#define RANDOMF2(f) (drand48()*(f))

// uniformly distributed in the interval [-f,f]
#define RANDOMF3(f) ((f)*(2.0*(drand48() - 0.5)))

// uniformly distributed in the interval [1.0-f, 1.0+f] 
#define RANDOMF(f) (1.0 + RANDOMF3(f))

/* true if a <= x <= b */
#define CART_SIM_BETWEEN(a, x, b) (((x)>=(a))&&((x)<=(b)))


#endif
