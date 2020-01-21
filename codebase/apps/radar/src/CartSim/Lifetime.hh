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
 * @file  Lifetime.hh
 * @brief  Simple management of an event's start/stop times
 * @class  Lifetime
 * @brief  Simple management of an event's start/stop times
 */

# ifndef    LIFETIME_H
# define    LIFETIME_H

#include "Params.hh"
#include "SimMath.hh"
#include <rapmath/FuzzyF.hh>

//----------------------------------------------------------------
class Lifetime
{
public:

  /**
   * Empty constructor
   */
  inline Lifetime(void) :  _start(0), _lifetime(0), _stop(0) {}
  
  /**
   * Default constructor
   * @param[in] startMin  Starting time (minutes since start of sim)
   * @param[in] lifetimeMin  Total minutes the event lives
   */
  inline Lifetime(const int startMin, const int lifetimeMin) : 
    _start(static_cast<int>(startMin*MINUTES_TO_SECONDS)),
    _lifetime(static_cast<int>(lifetimeMin*MINUTES_TO_SECONDS))
  {
    _stop = _start + _lifetime;
  }
  
  /**
   * Destructor
   */
  inline virtual ~Lifetime(void) {}

  /**
   * @return true if input value indicates 'not alive'
   *
   * @param[in] seconds  Number of seconds since simulation start
   */
  inline bool outOfRange(const int seconds) const
  {
    return (seconds < _start || seconds > _stop);
  }

  /**
   * @return number of seconds into the events life for input value
   * sim start
   *
   * @param[in] seconds  Number of seconds since simulation start
   */
  int elapsedSeconds(const int seconds) const
  {
    return seconds - _start;
  }

protected:
private:  

  int _start;    /* seconds from start of simulation till begin*/
  int _lifetime; /* seconds the event lives */
  int _stop;     /* seconds from start of simulation till event stop */

};

# endif 
