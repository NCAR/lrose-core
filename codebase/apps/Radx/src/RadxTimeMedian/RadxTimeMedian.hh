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

#include "Params.hh"
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
   * @param[in] argc  Args count
   * @param[in] argv  Args
   * @param[in] cleanup  Method to call on exit
   * @param[in] outOfStore  Method to call  when not enough memory
   */  
  RadxTimeMedian (int argc, char **argv, void cleanup(int),
		  void outOfStore(void));

  /**
   * Destructor
   */
  virtual ~RadxTimeMedian(void);

  /**
   * Main run method.  
   * @return 0 for good, non-zero for problems
   */
  int Run(void);

  /**
   * compute method for a beam, needed in threading
   * @param[in] info  Info pointer
   * @param[in] alg   RadxTimeMedian pointer
   */
  static void compute(void *info);
  
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

  /**
   * @class RadxThreads
   * @brief Implements TaThreadDoubleQue 
   */
  class RadxThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Trivial constructor
     */
    inline RadxThreads(void) : TaThreadDoubleQue() {}
    /**
     * Trivial destructor
     */
    inline virtual ~RadxThreads(void) {}
    /**
     * @return pointer to TaThread created by method
     * @param[in] index  Index that may be used
     */
    TaThread *clone(int index);
  };


  RadxApp _alg;      /**< Library algorithm object */
  RadxVol _templateVol;  /**< Template */
  bool _first;           /**< True for first volume */
  Params _params;        /**< params */

  RayxMapping _rayMap;
  std::map<RadxAzElev, RayHisto> _store;/**< Map from az/elev to RayHisto */
  
  RadxThreads _thread;    /**< Threading */

  void _process(const time_t t, RadxVol &vol, const bool last);
  void _filter(const time_t &t, const RadxRay *ray);
  bool _filter_first(const time_t &t, const RadxRay &ray);
  bool _filter_last(const time_t &t, RadxRay &ray);
};

#endif
