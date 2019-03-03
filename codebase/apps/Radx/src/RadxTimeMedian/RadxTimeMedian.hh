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
 * @file RadxTimeMedian.hh
 * @brief Main algorithm
 * @class RadxTimeMedian
 * @brief Main algorithm
 */

#ifndef RADXTIMEMEDIAN_H
#define RADXTIMEMEDIAN_H

#include "Parms.hh"
#include "Volume.hh"
#include "RayHisto.hh"

#include <radar/RadxApp.hh>
#include <Radx/RayxMapping.hh>
#include <Radx/RadxAzElev.hh>
#include <Radx/RadxVol.hh>
#include <toolsa/TaThreadDoubleQue.hh>
#include <map>

class RadxTimeMedian
{
  
public:

  /**
   * Constructor
   * @param[in] parms Parms
   */  
  RadxTimeMedian (const Parms &parms);

  /**
   * Destructor
   */
  virtual ~RadxTimeMedian(void);

  /**
   * Initialize first time through
   * @param[in,out] volume  The data
   */
  void initFirstTime(Volume *volume);
  
  /**
   * Finish up after last volume processed
   * @param[in,out] volume  The data
   */
  void processLast(Volume *volume);

  /**
   * Initialize a particular ray
   * @param[in] ray  The ray
   * @param[out] r  The initialized data to use
   *
   * @return Pointer to the histo data to use for this ray
   */
  RayHisto *initRay(const RadxRay &ray, RayxData &r);

  /**
   * Process a particular ray
   * @param[in] r  The data to use
   * @param[in,out] h The histogram to update
   * @return True for success
   */
  bool processRay(const RayxData &r, RayHisto *h) const;

  int OK;  /**< True for good object */

  /**
   * @return true if this az/elev is shared by more than one beam in volume
   *  after taking into account tolerances.
   * @param[in] az
   * @param[in] elev
   */
  inline bool isMulti(const double az, const double elev) const
  {
    return _rayMap.isMulti(az, elev);
  }

  /**
   * @return the histogram ray for the input az/elev, or NULL
   * @param[in] az
   * @param[in] elev
   */
  RayHisto *matchingRayHisto(const double az, const double elev);

protected:
private:

  Parms _parms;             /**< Params */
  bool _first;              /**< True for first time called */
  Volume _templateVolume;   /**< Template to use at th end */

  RayxMapping _rayMap;      /**< Mapping object to map az/elev to fixed values*/

  /**
   * Mapping from az/elev to histogram
   */
  std::map<RadxAzElev, RayHisto> _store;/**< Map from az/elev to RayHisto */
  
  bool _filter_first(const RadxRay &ray);
  bool _filter_last(RadxRay &ray);
};

#endif
