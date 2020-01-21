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
 * @file  CartSim.hh
 * @brief  The algorithm layer for CartSim
 * @class  CartSim
 * @brief  The algorithm layer for CartSim
 */

# ifndef    CART_SIM_H
# define    CART_SIM_H

#include "Params.hh"
#include "CartesianSpeck.hh"
#include "Data.hh"
#include "DeviantRay.hh"
#include "GustFront.hh"
#include "Microburst.hh"
#include "PolarSpeck.hh"
#include "Storm.hh"
#include "Turbulence.hh"
#include "VelCircle.hh"
#include "Clutter.hh"
#include <vector>

//----------------------------------------------------------------
class CartSim
{
public:

  /**
   * Default constructor
   * @param[in] parms  The algorithm parameters
   */
  CartSim(const Params &parms);
  
  /**
   * Destructor
   */
  virtual ~CartSim(void);

  /**
   * Set time to start time of simulation plus dt
   * @param[in] dt  Delta seconds since start
   */
  void setTime(const int dt); 

  /**
   * Process one point
   * @param[in] loc  Location from origin (meters)
   * @param[out] data  Data values get filled in
   */
  void process(const Xyz &loc, Data &data);

  /**
   * @return the current time
   */
  inline time_t currentTime(void) const {return _time;}

protected:
private:  

  Params _parms;  /**< Params */
  time_t _t0;     /**< Start time */
  time_t _time;   /**< Current time */
  Data _ambient;  /**< Ambient background values */
 
  std::vector<CartesianSpeck> _cspeck;
  std::vector<DeviantRay> _dray;
  std::vector<GustFront> _gf;
  std::vector<Microburst> _mb;
  std::vector<PolarSpeck> _pspeck;
  std::vector<Storm> _storm;
  std::vector<Turbulence> _turb;
  std::vector<VelCircle> _vcircle;
  std::vector<Clutter> _clutter;

  void _addDeviantRay(int seconds, const Xyz &loc, Data &data);
  void _addGf(int seconds, const Xyz &loc, Data &data);
  void _addMb(int seconds, const Xyz &loc, Data &data);
  void _addSpecks(int seconds, const Xyz &loc, Data &data);
  void _addStorm(int seconds, const Xyz &loc, Data &data);
  void _addTurb(int seconds, const Xyz &loc, Data &data);
  void _addVelCircle(int seconds, const Xyz &loc, Data &data);
  void _addClutter(int seconds, const Xyz &loc, Data &data);
};

# endif 
