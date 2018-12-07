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
 * @file RadxPersistentClutter.hh
 * @brief Base class for persistent clutter algorithm
 * @class RadxPersistentClutter
 * @brief Base class for persistent clutter algorithm
 */

#ifndef RADXPERSISTENTCLUTTER_H
#define RADXPERSISTENTCLUTTER_H

#include "Parms.hh"
#include "RayClutterInfo.hh"
#include <radar/RadxApp.hh>
#include <Radx/RadxAzElev.hh>
#include <Radx/RayxMapping.hh>
#include <toolsa/TaThreadDoubleQue.hh>
#include <toolsa/LogStream.hh>
#include <map>
#include <stdexcept>

class FrequencyCount;
class RadxVol;
class RayData;

class RadxPersistentClutter
{
public:

  /**
   * Constructor
   * @param[in] argc  Args count
   * @param[in] argv  Args
   * @param[in] cleanup  Method to call on exit
   * @param[in] outOfStore  Method to call  when not enough memory
   */
  RadxPersistentClutter(const Parms &parms, void cleanup(int));

  /**
   * Destructor
   */
  virtual ~RadxPersistentClutter(void);

  /**
   * Run the algorithm (calls some of the virtual methods)
   *
   * @return true for success
   */
  bool processVolume(RayData *volume, bool &first);

  /**
   * Compute method needed by threading
   * @param[in] info Pointer to Info
   */
  static void compute(void *info);


  #define MAIN
  #include "RadxPersistentClutterVirtualMethods.hh"
  #undef MAIN


  /**
   * Template method to find matching az/elev and return pointer
   * @tparam[in] T  Class with RayClutterInfo as a base class
   * @param[in] store  Vector of T objects
   * @param[in] az  Azimuth to match
   * @param[in] elev  Elevation to match
   *
   * @return pointer to something from store, or NULL
   */
  template <class T>
  const RayClutterInfo *matchInfoConst(const map<RadxAzElev, T> &store,
				       const double az,
				       const double elev) const
  {
    RadxAzElev ae = _rayMap.match(az, elev);
    if (ae.ok())
    {
      try
      {
	return dynamic_cast<const RayClutterInfo *>(&store.at(ae));
      }
      catch (std::out_of_range err)
      {
	LOG(WARNING) << ae.sprint() << " is out of range of mappings";
	return NULL;
      }
    }
    return NULL;
  }

  /**
   * Template method to find matching az/elev and return CONST pointer
   * @tparam[in] T  Class with RayClutterInfo as a base class
   * @param[in] store  Vector of T objects
   * @param[in] az  Azimuth to match
   * @param[in] elev  Elevation to match
   *
   * @return pointer to something from store, or NULL
   */
  template <class T>
  RayClutterInfo *matchInfo(map<RadxAzElev, T> &store, const double az,
			    const double elev)
  {
    RadxAzElev ae = _rayMap.match(az, elev);
    if (ae.ok())
    {
      try
      {
	return dynamic_cast<RayClutterInfo *>(&store.at(ae));
      }
      catch (std::out_of_range err)
      {
	LOG(WARNING) << ae.sprint() << " is out of range of mappings";
	return NULL;
      }
    }
    return NULL;
  }



  bool OK;

protected:

  /**
   * @class RadxThreads
   * @brief Instantiate TaThreadDoubleQue by implementing clone() method
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
     * @return pointer to TaThread created in the method
     * @param[in] index  Index value to maybe use
     */
    TaThread *clone(int index);
  };


  // RadxApp _alg;      /**< generic algorithm object */
  Parms _parms;         /**< The parameters */
  bool _first;          /**< True for first volume */
  RayxMapping _rayMap;

  /**
   * The storage of all info needed to do the computations, one object per
   * az/elev
   */
  std::map<RadxAzElev, RayClutterInfo> _store; 
  time_t _first_t;    /**< First time */
  time_t _final_t;    /**< Last time processed, which is the time at which
		       *   results converged */

  RadxThreads _thread;  /**< Threading */

  /**
   * Initialize a RadxRay by converting it into RayxData, and pointing to
   * the matchiing element of _store
   *
   * @param[in] ray  The data
   * @param[out] r  The converted data
   *
   * @return the matching pointer, or NULL for error
   */
  RayClutterInfo *_initRayThreaded(const RadxRay &ray, RayxData &r);

  /**
   * Process to set things for output into vol
   *
   * @param[in,out] vol  The data to replace fields in for output
   */
  void _processForOutput(RayData *vol);


  /**
   * @return number of points in _store that have a particular count
   *
   * @param[in] number the count to get the number of matching points for
   */
  double _countOfScans(const int number) const;

  /**
   * count up changes in clutter value, and update _store internal state
   *
   * @param[in] kstar  the K* value from the paper
   * @param[in,out] F  FrequencyCount object to fill in 
   *
   * @return number of points at which clutter yes/no toggled
   */
  int _updateClutterState(const int kstar, FrequencyCount &F);

private:

  /**
   * Process inputs
   * @param[in] t  Data time
   * @param[in,out] vol The data
   *
   * @return true if this is the last data to process
   */
  bool _process(RayData *vol);

  /**
   * Process inputs, first time through
   * @param[in] t  Data time
   * @param[in] vol The data
   */
  void _processFirst(const RayData *vol);

  /**
   * Process inputs
   * @param[in] t  Time of data
   * @param[in]  ray  A ray of data
   */
  void _processRay(const time_t &t, const RadxRay *ray);

  /**
   * Process to set things for output into a ray
   *
   * @param[in,out] ray  The data to replace fields in for output
   */
  bool _processRayForOutput(RadxRay &ray);

  /**
   * Initialize a RadxRay by converting it into RayxData, and pointing to
   * the matchiing element of _store
   *
   * @param[in] ray  The data
   * @param[out] r  The converted data
   *
   * @return the matching pointer, or NULL for error
   */
  RayClutterInfo *_initRay(const RadxRay &ray, RayxData &r);
};

#endif
