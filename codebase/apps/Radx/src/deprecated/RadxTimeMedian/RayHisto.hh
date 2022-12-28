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
 * @file RayHisto.hh
 * @brief Histograms for a ray
 * @class RayHisto
 * @brief Histograms for a ray
 */

# ifndef    RAY_HISTO_HH
# define    RAY_HISTO_HH

#include <string>
#include <vector>
#include "Histo.hh"
class RayxData;

//------------------------------------------------------------------
class RayHisto
{
public:

  /**
   * constructor, empty
   */
  RayHisto(void);

  /**
   * constructor, initializes state
   *
   * @param[in] az   Ray azimuth angle
   * @param[in] elev Ray elevation angle
   * @param[in] x0   Ray closest point
   * @param[in] dx   Ray delta between points
   * @param[in] nx   Ray number of points
   * @param[in] minBin  Center of smallest bin 
   * @param[in] deltaBin Distance between bin centers
   * @param[in] maxBin  Center of largest bin 
   */
  RayHisto(const double az, const double elev, const double x0,
	   const double dx, const int nx, const double minBin,
	   const double deltaBin, const double maxBin);

  /**
   * Destructor
   */
  virtual ~RayHisto(void);

  /**
   * @return true if azimuth/elevation equals local values
   * @param[in] az
   * @param[in] elev
   */
  inline bool match(const double az, const double elev) const
  {
    return az == _az && elev == _elev;
  }

  /**
   * Store the information from this ray into the histograms storage
   * @param[in] r
   */
  bool update(const RayxData &r);

  /**
   * Compute the median at all points and store results into the ray
   * @param[out] r
   *
   * @return true for success
   */
  bool computeMedian(RayxData &r) const;

  /** 
   * Debug print 
   */
  void print(void) const;


protected:
private:  
  
  double _az;     /**< Ray azimuth angle */
  double _elev;   /**< Ray elevaton angle */
  double _x0;     /**< Ray closest point */
  double _dx;     /**< Ray delta between points */
  int _nx;        /**< Ray Number of points */
  std::vector<Histo> _histograms;  /**< A Histo at each point */

};

# endif
