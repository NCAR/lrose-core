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
 * @file  GustFront.hh
 * @brief  Gust Fronts
 * @class  GustFront
 * @brief  Gust Fronts
 */

# ifndef    GUST_FRONT_H
# define    GUST_FRONT_H

#include "Xyz.hh"
#include "Params.hh"
#include "Thing.hh"

class Data;
class LineGustFront;

//----------------------------------------------------------------
class GustFront : public Thing
{
public:

  /**
   * Default constructor
   * @param[in] P  general params
   * @param[in] mP  params for this front
   */
  GustFront(const Params &P, const Params::GustFront_t &mP);
  
  /**
   * Destructor
   */
  virtual ~GustFront(void);

  /*
   * Check if point x is outside the thin line cylinder or not, 
   * based on notes.
   *
   * @param[in] x  Distance in front (meters)
   * @param[in] z  Height (meters)
   *
   * @return true if point (x,z) is inside the thin line cylinder
   */
  bool insideThinLine(double x, double z) const;

  /*
   * Apply the gust front model at x, z and compute wind magnitude and direction
   *
   * @param[in] x  Distance in front (meters)
   * @param[in] z  Height (meters)
   * @param[out] mag  Magnitude of winds (m/s)
   * @param[out] dir  Direction vector
   */
  void model(const double x, const double z, double &mag, Xyz &dir) const;

  /**
   * @return side decay parameter value
   */
  inline double getSideDecay(void) const {return _side_decay;}

  /**
   * @return thin line dbz parameter value
   */
  inline double getDbzThinLine(void) const {return _dbzt;}

protected:

  virtual void _addToData(const Xyz &loc, Data &data) const;

private:  


  double _mag;           /**< mag of winds (m/s) */
  Xyz _motion;           /**< motion vector */
  double _zt;            /**< z of thin line */
  double _xt;            /**< x of thin line */
  double _rt;            /**< radius of thin line */
  double _dbzt;          /**< dbz of thin line */
  double _z1;            /**< model param */
  double _z2;            /**< model param */
  double _z3;            /**< model param */
  double _x1;            /**< model param */
  double _x2;            /**< model param */
  double _xb;            /**< model param */
  double _side_decay;    /**< meters to decay to 0.1 of max*/
  bool _is_wave;         /** True for wave (only possible if more than 2
			  *  endpoints) */
  std::vector<Xyz> _loc; /**< Endpoints of front (2 or more) */


  /**
   * Portion of wind field behind things, where its linearly decaying nicely
   */
  void _region1(double x, double z, double &mag, Xyz &dir) const;

  /**
   * Portion of windfield that has Turbulence at top, normal flow below
   */
  void _region2(double x, double z, double &mag, Xyz &dir) const;

  /**
   * Portion of windfield where you get the circular rolls.
   */
  void _region3(double x, double z, double &mag, Xyz &dir) const;

  /**
   * Portion of wind field that is turbulent.
   */
  void _turbulentRegion(double max, double &mag, Xyz &dir) const;

  /**
   * Portion of wind field outside the big circle
   */
  void _regionOutside3(double x, double centerx, double z,
		       double centerz, double &mag, Xyz &dir) const;

  bool _setup(const Xyz &loc, int &nloc, std::vector<LineGustFront> &w) const;

  bool _buildSegmentWinds(bool is_inside, int nloc,
			    std::vector<LineGustFront> &w, Xyz &wind) const;

  bool _buildSegmentThinLine(std::vector<LineGustFront> &w,
			     const Xyz &loc, int nloc, double &ref) const;

#ifdef WAVE
  void _setupWave(int elapsedt, int nloc, std::vector<LineGustFront> &w) const;
  void _waveInteriorPoint(const Xyz &loc0, const Xyz &loc1, 
			  double meters_move, Xyz &outloc0,
			  Xyz &outloc1) const;

  /**
   * @param[in] outloc0  2nd output point
   * @param[in] outloc1  3rd output point
   * @param[in] inloc0   0th input point
   * @param[in] inloc1   1th input point
   * @parma[in] meters_move  Meters to move
   * @param[in] is_ok  if false, need to reverse endpoints for motion direction
   * @param[out] ootloc  Returnd 0th output point
   */
  void _waveEndpoint(const Xyz &outloc0, const Xyz &outloc1, 
		     const Xyz &inloc0, const Xyz &inloc1,
		     double meters_move, bool is_ok,
		     Xyz &outloc) const;
#endif

};

# endif 
