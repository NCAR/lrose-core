// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// PidImapManager.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2008
//
///////////////////////////////////////////////////////////////

/**
 * @file PidImapManager.hh
 * @class PidImapManager
 * @brief Manages a series of interest maps for different dbz ranges
 */

#ifndef PidImapManager_hh
#define PidImapManager_hh

#include <string>
#include <vector>
#include <cmath>
#include <radar/PidInterestMap.hh>
using namespace std;

class PidImapManager {
  
public:

  /**
   * Constructor
   * @param[in] particleLabel The label for the particle
   * @param[in] particleDescr A description of the particle
   * @param[in] field The radar variable to analyze
   * @param[in] weight The weight of the radar variable in determining particle interest
   * @param[in] missingVal The value used for missing data
   */
  PidImapManager(const string particleLabel,
		 const string particleDescr,
		 const string &field,
		 double weight,
		 double missingVal);
  
  /**
   * Destructor
   */
  ~PidImapManager();

  // add a map

  /**
   * Add an interest mapping function
   * @param[in] mindbz The minimum dbz that this map is valid for
   * @param[in] maxdbz The maximum dbz that this map is valid for
   * @param[in] map A vector of point vertices for this interest map
   */
  void addInterestMap(double mindbz,
		      double maxdbz,
		      const vector<PidInterestMap::ImPoint> &map);

  /**
   * Get the weight for the map
   * @return The weight for the map
   */
  inline double getWeight() const { return _weight; }

  /**
   * Get interest for a given val
   * @return The interest for a given value
   */
  double getInterest(double dbz, double val) const;
  
  /**
   * Get weighted interest for a given val
   * @param[in] dbz The reflectivity associated with the given value
   * @param[in] val The value of the radar variable beind analyzed
   * @param[out] interest The weighted interest score
   * @param[out] wt The weight used to generate the interest score
   */
  void getWeightedInterest(double dbz, double val, double &interest, double &wt) const;
  
  /**
   * Accumulate weighted interest based on value
   * @param[in] dbz The reflectivity associated with the given value
   * @param[in] val The value of the radar variable beind analyzed
   * @param[in][out] sumWtInterest The accumulated weighted interest values
   * @param[in][out] sumWt The accumulated total weights
   */
  inline void accumWeightedInterest(double dbz,
				    double val,
				    double &sumWtInterest,
				    double &sumWt) const {
    
    if (fabs(_weight) < 0.0001) {
      return;
    }
    
    int index = getIndex(dbz);
    const PidInterestMap *map = _mapLut[index];
    if (map == NULL) {
      sumWtInterest += 0.0;
      sumWt += _weight;
      return;
    }
    
    map->accumWeightedInterest(val, sumWtInterest, sumWt);
    
  }
 
  /** 
   * Compute index into the lookup table pointer array from dbz
   * @param[in] dbz The dbz value to use
   * @return The index into the lookup table array. This index can be
   *         used to access the lookup table for the given dbz value
   */
  inline int getIndex(double dbz) const {
    int index = (int) (dbz * 10.0 + _lutOffset);
    if (index < 0) {
      return 0;
    } else if (index > _nLut - 1) {
      return _nLut - 1;
    }
    return index;
  }
  
  /**
   * Print this object
   * @param[out] out The stream to print to
   */
  void print(ostream &out);
  
protected:
private:
  
  string _particleLabel;   /**< The label of the particle to calculate interest for */
  string _particleDescr;   /**< The description of the particle to calculate interest for */
  string _field;           /**< The radar variable to analyze for particle identification interest */
  double _weight;          /**< The weight to assign this radar variable's interest score */
  double _missingDouble;   /**< The value to use for missing data */

  vector<PidInterestMap*> _maps;  /**< Vector of current maps */

  // Lookup table of map pointers, for quickly accessing maps based on DBZ value.
  // Lookup table resolution is 0.1 dBZ, with 1000 points below and above 0.
  // The valid range for dbz values is -100 to +100.
  // Values outside this range will map to these limits.

  const static int _lutOffset = 1000;  
  const static int _nLut = 2000;      /**< The number of lookup table pointers */
  PidInterestMap* _mapLut[_nLut];     /**< A pointer to an array of interest maps - each 0.1 dbz value has 
                                           a pointer to a specific lookup table */

};

#endif

