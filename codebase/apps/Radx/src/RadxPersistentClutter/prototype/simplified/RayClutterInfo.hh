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
 * @file RayClutterInfo.hh
 * @brief Clutter information for a ray
 * @class RayClutterInfo
 * @brief Clutter information for a ray
 *
 * Information about the ray location (az,elev,gates), and two
 * sets of data, the binary clutter yes/no (based on best threshold)
 * and the counts (number of scans that have data >= threshold)
 */

# ifndef    RAY_CLUTTER_INFO_HH
# define    RAY_CLUTTER_INFO_HH

#include <string>
#include <vector>
#include "RayData.hh"

class FrequencyCount;

//------------------------------------------------------------------
class RayClutterInfo
{
public:

  /**
   * Constructor
   */
  RayClutterInfo(void);

  /**
   * Constructor
   * @param[in] az    Ray azimuth
   * @param[in] elev  Ray elevation
   * @param[in] x0    Ray first gate
   * @param[in] dx    Ray distance between gates
   * @param[in] nx    Ray number of gates
   */
  RayClutterInfo(const double az, const double elev, const double x0,
		 const double dx, const int nx);

  /**
   * Destructor
   */
  virtual ~RayClutterInfo(void);

  /**
   * @return true if az and elev equal local values
   * @param[in] az
   * @param[in] elev
   */
  inline bool match(const double az, const double elev) const
  {
    return az == _az && elev == _elev;
  }

  /**
   * Store the information from this ray into the local state
   *
   * @param[in] r  Input ray data
   * @param[in] threshold  Clutter threshold (data >= threshold = clutter)
   *
   * @return true if input ray matched local ray information, and internal
   * state has been updated (_counts)
   */
  bool update(const RayData &r, const double threshold);

  /**
   * At all points where _counts >= k, set value in RayData to 1, at all other
   * points set value to 0
   *
   * @param[in] r  Data to fill in
   * @param[in] k  Threshold count
   *
   * @return true if input ray matched local ray information, and r has been
   * been updated
   */
  bool equalOrExceed(RayData &r, const int k) const;

  /**
   * At each point in r, set the value to the count/n (normalized counts)
   *
   * @param[in] r  Data to fill in
   * @param[in] n Normalization value
   *
   * @return true if input ray matched local ray information, and r has been
   * been updated
   */
  bool loadNormalizedFrequency(RayData &r, const int n) const;

  /** 
   * Debug print 
   */
  void print(void) const;

  /**
   * @return number of points at which _counts = number
   * @param[in] number  The number to check for
   */
  double numWithMatchingCount(const int number) const;

  /**
   *
   * Update internal state for _clutter (yes/no) using number.
   * At all points where counts >= number, it is clutter. Keep track of
   * which points had a change in this state, and return the number of points
   *
   * @param[in]  number  The threshold for clutter
   * @param[in,out] nclutter Incremented with # of points that show clutter
   * @param[in,out] F  Updated using counts values from local state
   */
  int updateClutter(const int number, int &nclutter, FrequencyCount &F);

  /**
   * @return elevation degrees
   */
  inline double getElevation() const {return _elev;}

  /**
   * @return azimuth degrees
   */
  inline double getAzimuth() const {return _az;}

protected:

  double _x0;  /**<  Ray first gate */
  double _dx;  /**< Ray delta between gates */
  int _nx;     /**< Ray number of gates */

  /**
   * Clutter yes/no values using current best threshold and _counts
   */
  RayData _clutter;

  /**
   * Determine number of effective points by comparing input ray with
   * local state, returning that number.
   *
   * @param[in] r  Input ray to compare to local
   *
   * @return Number of points to analyze, -1 for a mis match between input
   *         and local state.
   */
  int _initNpt(const RayData &r) const;

private:  
  
  double _az;     /**< Ray azimuth */
  double _elev;   /**< Ray elevation */

  /**
   * Number of scans with data >= clutter threshold 
   */
  RayData _counts;

};

# endif
