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
 * @file DataHandler.hh
 * @brief Virtual class to handle data input/output
 * @class DataHandler
 * @brief Virtual class to handle data input/output
 */

# ifndef    DATA_HANDLER_H
# define    DATA_HANDLER_H


#include "Params.hh"
class Xyz;
class Data;

//----------------------------------------------------------------
class DataHandler
{
public:

  /**
   * Default constructor
   *
   * @param[in] p  The parameters to put into state
   *
   */
  DataHandler(const Params &p);
  
  /**
   *  Destructor
   */
  virtual ~DataHandler(void);

  /**
   * @return a created copy of the derived class, down cast to base class
   */
  virtual DataHandler *clone(void) const = 0;

  /**
   * Increment and return location of next grid point (x,y,z meters)
   * @param[out] loc  The location
   * @return true if there was a next point to return, false if at end
   */
  bool nextPoint(Xyz &loc);

  /**
   * Initialize prior to processing
   * @return true if successful
   */
  virtual bool init(void)=0;

  /**
   * Reset internal state to first grid point (rewind)
   */
  virtual void reset(void)=0;

  /**
   * Store input data to the current grid point locally.
   * @param[in] data  The data to store
   */
  virtual void store(const Data &data) = 0;

  /**
   * Write current grids to output
   * @param[in] t  Time to give data 
   */
  virtual void write(const time_t &t) = 0;

protected:

  /**
   * The Algorithm parameters, kept as internal state
   */
  Params _parms;

  /**
   * True for polar data, false for flat
   */
  bool _polar;

  double _cx;  /**< Current native grid coordinates x location */
  double _cy;  /**< Current native grid coordinates y location */
  double _cz;  /**< Current native grid coordinates z location */
  bool _first;  /**< True if indexing not yet initiated */

  /** 
   * Reset so _first=true and indexing is re-initialized
   */
  void _reset(void);

  /**
   * @return the radial velocity as computed from inputs and _cx,_cy,_cz
   * @param[in] data  Data object with vx, vy, vz values in Cartesian coords
   */
  double _radialVel(const Data &data) const;

  /**
   * Initialize the indexing for the derived class to the start point
   */
  virtual void _initIndexing(void) = 0;

  /**
   * Compute and return the current grid location in Cartesian coordinates
   *
   * @return grid location in meters
   *
   * A side effect set _cx, _cy, _cz for use in computing radial velocity
   */
  virtual Xyz _computeXyzMeters(void) = 0;

  /**
   * Increment the indexing into the grids
   * @return true if was able to increment to the next index
   */
  virtual bool _increment(void) = 0;

private:  

};

# endif 
