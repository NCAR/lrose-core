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
 * @file  Thing.hh
 * @brief  Stuff shared by all the simulated products
 * @class  Thing
 * @brief  Stuff shared by all the simulated products
 */

# ifndef    THING_H
# define    THING_H

#include "Params.hh"
#include "Lifetime.hh"
#include "Xyz.hh"
#include <rapmath/FuzzyF.hh>
class Data;

//----------------------------------------------------------------
class Thing : public Lifetime
{
public:

  /**
   * Default constructor
   */
  Thing(const Params &P, int minutes2intensity_index, int minutes2size_index,
	double startMin, double lifetimeMin);
  
  /**
   * Destructor
   */
  virtual ~Thing(void);

  /**
   * Compute and set data values at a point in space/time
   *
   * @param[in] seconds  Seconds since simulation start
   * @param[in] loc  Location (meters)
   * @param[out] data  Data values
   *
   * Calls pure virtual method _addToData()
   */
  void addToData(const int seconds, const Xyz &loc, Data &data);

  /**
   * Set endpoints based on index into endpoint data in params
   *
   * @param[in] P params
   * @param[in] index  Index into endpoint choices, (endpoint units = km)
   * @param[out]  loc  The endpoints, units = meters
   */
  static bool endpoints(const Params &P, int index, std::vector<Xyz> &loc);


protected:

  FuzzyF    _seconds2intensity; /* mapping from time to intensity */
  FuzzyF    _seconds2size;      /* mapping from time to size */

  double _intensity;  /**< Current intensity */
  int _elapsedSeconds;  /**< Current time since simulation start */

  void _setFuzzy(const Params &P, const int i,  FuzzyF &f);
  void _setFuzzyMinutesToInterest(const Params &P, const int i,  FuzzyF &f);
  void _setFuzzyKmToInterest(const Params &P, const int i,  FuzzyF &f);
  void _setFuzzy(int n, const Params::Xy_t *v, FuzzyF &f);

  /**
   * Compute and set data values at a point in space/time
   *
   * @param[in] seconds  Seconds since simulation start
   * @param[in] loc  Location (meters)
   * @param[out] data  Data values
   */
  virtual void _addToData(const Xyz &loc, Data &data) const = 0;

private:  

  /**
   * Set the intensity member value.
   *
   * @param[in] seconds  Seconds since start of simulation
   *
   * @return true if intensity > 0 on exit
   */
  bool _setIntensity(const int seconds);

};

# endif 
