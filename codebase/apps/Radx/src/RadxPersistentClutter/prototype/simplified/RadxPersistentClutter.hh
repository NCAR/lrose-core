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

#include "Args.hh"
#include "Params.hh"
#include "RayClutterInfo.hh"
#include "RayMapping.hh"
#include "RayAzElev.hh"
#include <Radx/RadxFile.hh>
#include <toolsa/TaThreadDoubleQue.hh>
#include <toolsa/LogStream.hh>
#include <didss/LdataInfo.hh>
#include <map>
#include <stdexcept>

class FrequencyCount;
class RadxVol;

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
  RadxPersistentClutter (int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~RadxPersistentClutter(void);

  /**
   * Run the algorithm (calls some of the virtual methods)
   *
   * @return true for success
   */
  bool run(const string &label);

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
  const RayClutterInfo *matchInfoConst(const map<RayAzElev, T> &store,
				       const double az,
				       const double elev) const
  {
    RayAzElev ae = _rayMap.match(az, elev);
    if (ae.ok())
    {
      try
      {
	return dynamic_cast<const RayClutterInfo *>(&store.at(ae));
      }
      catch (const std::out_of_range &err)
      {
        LOG(DEBUG_EXTRA) << ae.sprint() << " is out of range of mappings";
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
  RayClutterInfo *matchInfo(map<RayAzElev, T> &store, const double az,
			    const double elev)
  {
    RayAzElev ae = _rayMap.match(az, elev);
    if (ae.ok())
    {
      try
      {
	return dynamic_cast<RayClutterInfo *>(&store.at(ae));
      }
      catch (const std::out_of_range &err)
      {
        LOG(DEBUG_EXTRA) << ae.sprint() << " is out of range of mappings";
	return NULL;
      }
    }
    return NULL;
  }

  /**
   * @return portion of input string past the last '/'
   * @param[in] name  Full path
   */
  static std::string nameWithoutPath(const std::string &name);

  /**
   * return RadxData of a particular name
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[in] data Zero or more RadxData objects, one of which might have the
   *                 name
   * @param[out] r  The data (a fresh copy)
   * @return true if found the named data in either the ray or in the data
   */
  static bool retrieveRay(const std::string &name, const RadxRay &ray,
			  std::vector<RayData> &data, RayData &r);

  /**
   * return RayData of a particular name
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[out] r  The data (a fresh copy)
   * @param[in] showError  If true, lack of data generates a printed error
   *
   * @return true if found the named data in the ray
   */
  static bool retrieveRay(const std::string &name, const RadxRay &ray,
			  RayData &r, const bool showError=true);

  /**
   * Take some RayData and modify it based on other inputs
   * @param[in,out] r  RayData to be modified
   * @param[in] name  Name to give the data
   * @param[in] units  Units to give the data (if non empty)
   * @param[in] missing  Missing data value to give the dat (if units non empty)
   */
  static void modifyRayForOutput(RayData &r, const std::string &name,
				 const std::string &units="", 
				 const double missing=0);

  /**
   * add data for some RayData to a RadxRay, then clear out all other fields
   * 
   * @param[in] r  Data to add
   * @param[in,out] ray  Object to add to
   */
  static void updateRay(const RayData &r, RadxRay &ray);

  /**
   * add data for multiple RayData's to a RadxRay, clearing out all other fields
   * 
   * @param[in] r  Data to add
   * @param[in,out] ray  Object to add to
   */
  static void updateRay(const vector<RayData> &r, RadxRay &ray);

  bool OK;

protected:

  /**
   * @class ComputeThread
   * @brief Instantiate TaThreadDoubleQue by implementing clone() method
   */
  class ComputeThread : public TaThreadDoubleQue
  {
  public:
    /**
     * Trivial constructor
     */
    inline ComputeThread(void) : TaThreadDoubleQue() {}
    /**
     * Trivial destructor
     */
    inline virtual ~ComputeThread(void) {}

    /**
     * @return pointer to TaThread created in the method
     * @param[in] index  Index value to maybe use
     */
    TaThread *clone(int index);
  };


  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // App _alg;      /**< generic algorithm object */
  bool _first;           /**< True for first volume */
  RayMapping _rayMap;

  time_t _start;         /**< Start time in ARCHIVE mode */
  time_t _end;           /**< End time in ARCHIVE mode */
  std::vector<std::string> _fileList; /**< paths in FILELIST mode from args */

  std::vector<std::string> _paths; /**< paths in ARCHIVE or FILELIST mode*/
  int _pathIndex;  /**< Next file to process (index) (ARCHIVE or FILELIST) */
  LdataInfo _ldata;  /**< Triggering mechanism for REALTIME */
  
  /**
   * The storage of all info needed to do the computations, one object per
   * az/elev
   */
  std::map<RayAzElev, RayClutterInfo> _store; 
  time_t _first_t;    /**< First time */
  time_t _final_t;    /**< Last time processed, which is the time at which
		       *   results converged */

  ComputeThread _thread;  /**< Threading */

  // set up derived params
  
  int _initDerivedParams();

  /**
   * Initialize a RadxRay by converting it into RayData, and pointing to
   * the matchiing element of _store
   *
   * @param[in] ray  The data
   * @param[out] r  The converted data
   *
   * @return the matching pointer, or NULL for error
   */
  RayClutterInfo *_initRayThreaded(const RadxRay &ray, RayData &r);

  /**
   * Process to set things for output into vol
   *
   * @param[in,out] vol  The data to replace fields in for output
   */
  void _processForOutput(RadxVol &vol);


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

  /**
   * Process inputs
   * @param[in] t  Data time
   * @param[in,out] vol The data
   *
   * @return true if this is the last data to process
   */
  bool _process(const time_t t, RadxVol &vol);

  /**
   * Process inputs, first time through
   * @param[in] t  Data time
   * @param[in] vol The data
   */
  void _processFirst(const time_t t, const RadxVol &vol);

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
   * Initialize a RadxRay by converting it into RayData, and pointing to
   * the matchiing element of _store
   *
   * @param[in] ray  The data
   * @param[out] r  The converted data
   *
   * @return the matching pointer, or NULL for error
   */
  RayClutterInfo *_initRay(const RadxRay &ray, RayData &r);


  /**
   * default triggering method. Waits till new data triggers a return.
   *
   * @param[out] v   Volume of Radx data that was read in
   * @param[out] t   time that was triggered.
   * @param[out] last  true if this is the last data
   *
   * @return true if file was read properly, false if read error occurred.
   */
  bool _trigger(RadxVol &v, time_t &t, bool &done);

  /**
   * Rewind so next call to trigger() will return the first file
   * @return true if successful
   */
  bool _rewind(void);

  /**
   * Write volume to input url.
   * @param[in] vol Volume to write
   * @param[in] t  Time to write
   * @param[in] url  The url to write to
   *
   * @return true if successful
   */
  bool _write(RadxVol &vol, const time_t &t, const std::string &dir);

  /**
   * Read one file to create a volume
   * @param[in] path  File to process
   * @param[out] vol The volume
   * @param[out] t
   *
   * @return true for success
   */
  bool _readFile(const std::string &path, RadxVol &vol, time_t &t);

  /**
   * Set read request for primary data into the RadxFile object
   * @param[in] file  Object to modify
   */
  void _setupRead(RadxFile &file);

  /**
   * Set write request into the RadxFile object
   * @param[in] file  Object to modify
   */
  void _setupWrite(RadxFile &file);

private:

};

#endif
