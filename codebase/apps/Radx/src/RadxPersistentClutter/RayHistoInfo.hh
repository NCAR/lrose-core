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
/**
 * @file RayHistoInfo.hh
 * @brief Histograms for a ray
 * @class RayHistoInfo
 * @brief Histograms for a ray
 *
 * This is derived from RayClutterInfo, and has a map from index value to Histo
 * because only some points (clutter points) need it, and it is too expensive
 * (memory) to have Histo for every point
 */

# ifndef    RAY_HISTO_INFO_HH
# define    RAY_HISTO_INFO_HH

#include <map>
#include "RayClutterInfo.hh"
#include "Histo.hh"

class Params;

//------------------------------------------------------------------
class RayHistoInfo : public RayClutterInfo
{
public:

  /**
   * Constructor
   *
   */
  RayHistoInfo(void);

  /**
   * Constructor
   *
   * @param[in] r  The base class data to use
   * @param[in] p  Parameters, used to initialize Histo objects
   */
  RayHistoInfo(const RayClutterInfo &r, const Params &p);

  /**
   * Destructor
   */
  virtual ~RayHistoInfo(void);

  /**
   * Store the information from this ray into the histogram storage
   * @param[in] r  The data to add in
   *
   * At clutter points (determined by base class), the histograms are
   * updated to include the data value from the input
   */
  bool updateSecondPass(const RayxData &r);

  /**
   * Store clutter values into the input ray data
   * @param[out] r  The object to write to
   * @param[in] percentile  The histogram percentile to use
   * @param[in] missingClutterValue  The value to set at a point when
   *                                 the percentile'th data value is
   *                                 missing. This is so that you can
   *                                 distinguish clutter identified points
   *                                 from non-clutter points, even in this
   *                                 situation.  Typically this is a very low
   *                                 value
   */
  bool setClutter(RayxData &r, const double percentile,
		  const double missingClutterValue) const;

  /** 
   * Debug print 
   */
  void print(void) const;

  
protected:
private:  

  /**
   * _histograms[index] is the Histo for each index at which the base
   * class says there is clutter
   */
  std::map<int, Histo> _histograms;
  

  /**
   * @return A pointer to the Histo associated with an index in the _histogram,
   *         NULL if input index is not in the _histogram map.
   *
   * @param[in] i  Index
   *
   * This is a 'const []' operator on the map
   */
  const Histo *_histoForIndex(const int i) const;


  /**
   * @return the output clutter value at index i, using histograms if it
   * is a clutter point
   * 
   * @param[in] i  Index
   * @param[in] percentile  Histogram percentile to use
   * @param[in] missingClutterValue  The value to set at a point when
   *                                 the percentile'th data value is
   *                                 missing. 
   * @param[in] missingValue  The data missing value
   */
  double _outputValue(const int i, const double percentile, 
		      const double missingClutterValue,
		      const double missingValue) const;
};

# endif
