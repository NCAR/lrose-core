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
 * @file  Storm.hh
 * @brief  Sphere with specified reflectivity
 * @class  Storm
 * @brief  Sphere with specified reflectivity
 */

# ifndef   STORM_HH
# define   STORM_HH
#include "Xyz.hh"
#include "Thing.hh"
#include "Params.hh"
#include <rapmath/FuzzyF.hh>
#include <vector>

class Data;

//----------------------------------------------------------------
class Storm : public Thing
{
public:

  /**
   * Default constructor
   */
  Storm(const Params &P, const Params::Storm_t &mP);
  
  /**
   * Destructor
   */
  virtual ~Storm(void);

protected:

  virtual void _addToData(const Xyz &loc, Data &data) const;

private:  

  bool _override; /**< true to override all other dbz */
  double _noise;   /**< noise to give the storm */
  double _r;       /**< radius of storm..(meters) */
  double _min_z;   /**< minimum height of storm above ground */
  Xyz   _motion;  /**< motion vector (m/s) (storms can move) */
  std::vector<Xyz>  _loc; /**< center point locations of storm (meters) */
  FuzzyF _dist2dbz; /**< mapping from distance from center to dbz */

  void _addPoint(const Xyz &loc, Data &data) const;
  void _addLines(const Xyz &loc, Data &data) const;
  double _singlePointValue(const Xyz &loc, const Xyz &loc0) const;
  void _add(double value, Data &data) const;

};

# endif 
