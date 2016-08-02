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
// PidInterestMap.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////

/**
 * @file PidInterestMap.hh
 * @class PidInterestMap
 * @brief Handles interest mapping. Converts a data value into an
 *        interest value based on a linear function.
 */

#ifndef PidInterestMap_hh
#define PidInterestMap_hh

#include <string>
#include <vector>
using namespace std;

class PidInterestMap {
  
public:

  /**
   * @class ImPoint
   * @brief An object mapping a value to a particular interest score
   */
  class ImPoint {

  public:

    /**
     * Constructor
     * @param[in] val The value to find interest for
     * @param[in] interest The interest score mapped to this value
     */ 
    ImPoint(double val, double interest) {
      _val = val;
      _interest = interest;
    }

    /**
     * Get the value for this ImPoint
     * @return The value for this ImPoint
     */
    inline double getVal() const { return _val; }

    /**
     * Get the value for this ImPoint
     * @return The interest score for this ImPoint
     */
    inline double getInterest() const { return _interest; }

    /**
     * Set the value for this ImPoint
     * @param[in] The value for this ImPoint
     */
    inline void setVal(double val) { _val = val; }

    /**
     * Set the value for this ImPoint
     * @param[in] The interest score for this ImPoint
     */
    inline void setInterest(double interest) { _interest = interest; }

  private:

    double _val;      /**< The value for this ImPoint */
    double _interest; /**< The interest score for this ImPoint */
  };

  /**
   * Constructor
   * @param[in] label The label of this interest map (for debugging messages)
   * @param[in] minDbz The minimum dbz that this interest map is valid for
   * @param[in] maxDbz The maximum dbz that this interest map is valid for
   * @param[in] map The map of points defining a linear function used to generate
   *                the lookup table for this interest map 
   * @param[in] weight The weight of this interest map
   * @param[in] missingVal The value to use for missing data
   */
  PidInterestMap(const string &label,
		 double minDbz,
		 double maxDbz,
		 const vector<ImPoint> &map,
		 double weight,
		 double missingVal);
 
  /**
   * Destructor
   */ 
  ~PidInterestMap();

  /**
   * Get minimum dbz for this map
   * @return The minimum dbz for this map
   */
  inline double getMinDbz() const { return _minDbz; }

  /**
   * Get maximum dbz for this map
   * @return The maximum dbz for this map
   */
  inline double getMaxDbz() const { return _maxDbz; }

  /**
   * Get interest for a given val
   * @param[in] val The value to find interest for
   * @return The interest score for the given value
   */
  double getInterest(double val) const;
  
  /**
   * Get weighted interest for a given val
   * @param[in] val The value to find interest for
   * @param[out] interest The weighted interest score for the given value
   * @param[out] The weight used to generate the interest score
   */
  void getWeightedInterest(double val, double &interest, double &wt) const;
  
  /**
   * Accumulate weighted interest based on value
   * @param[in] val The value to find interest for
   * @param[out] sumInterest The accumulated weighted interest values
   * @param[out] sumWt The accumulated total weights
   */
  void accumWeightedInterest(double val,
                             double &sumInterest, double &sumWt) const;
  
  /**
   * Print this object
   * @param[out] out The stream to print to
   */ 
  void print(ostream &out) const;

protected:

private:
  
  static const int _nLut = 10001;   /** The number of mapped values in the lookup table */

  string _label;    /**< The label of this interest map (for debugging messages) */
  double _minDbz;   /**< The minimum dbz that this interest map is valid for */
  double _maxDbz;   /**< The maximum dbz that this interest map is valid for */

  vector<ImPoint> _map;  /**< The vector of lookup table points for this map */
  double _weight;        /**< The weight of this interest map */
  double _missingDouble; /**< The value to use for missing data */

  bool _mapLoaded;   /**< Flag to indicate whether an interest map has been generated */
  double _minVal;    /**< The minimum value used for this map */
  double _maxVal;    /**< The maximum value used for this map */
  double _dVal;      /**< The value resolution of the lookup table */

  double *_lut;          /** The array of values in the lookup table */
  double *_weightedLut;  /** The array of values in the weighted lookup table */

};

#endif

