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
 * @file App.hh
 * @brief Actions at the top level common to Radx algorithms
 * @class App
 * @brief Actions at the top level common to Radx algorithms
 */
# ifndef    RADX_APP_HH
# define    RADX_APP_HH

#include <ctime>
#include <string>
#include <vector>
#include <Radx/RayxData.hh>
#include "AppArgs.hh"
#include "AppConfig.hh"
#include <didss/LdataInfo.hh>
class RadxVol;
class RadxFile;
class RadxRay;
class RayxData;

//------------------------------------------------------------------
class App
{
public:

  /**
   * Constructor
   */
  App(void);

  /**
   * Destructor
   */
  virtual ~App(void);

  /**
   * Set local members using inputs
   * @param[in] a  Args object with start/end times, filelist copied in
   * @param[in] appName  copied in
   * @param[in] parmPath  Not used
   * @param[in] p  Parameter object copied into local state
   */
  void setValues(const AppArgs &a, const std::string &appName, 
		 const std::string &parmPath, const AppParams &p);

  /**
   *
   * Set infrastructure settings for logging, signalling, debugging,
   * triggering, process registration, using inputs and _params values
   *
   * @param[in] cleanup   Method to call when signalled for quit/interupt
   * @param[in] outOfStore Method to call when out of memory
   * @param[in] inputFields  field names checked against primary and
   *                         secondary groups for match.
   *
   * @return true unless inputFields are not all found in param groups
   */
  bool init(void cleanup(int), void outOfStore(void),
	    const std::vector<std::string> &inputFields);

  /**
   * Inform any monitoring software that the app is terminating.
   */
  void finish(void);

  /**
   * default triggering method. Waits till new data triggers a return.
   *
   * @param[out] v   Volume of Radx data that was read in
   * @param[out] t   time that was triggered.
   * @param[out] last  true if this is the last data
   *
   * @return true if a time was triggered, false for no more triggering.
   */
  bool trigger(RadxVol &v, time_t &t, bool &last);

  /**
   * Rewind so next call to trigger() will return the first file
   * @return true if successful
   */
  bool rewind(void);

  /**
   * Write volume to input url.
   * @param[in] vol Volume to write
   * @param[in] t  Time to write
   * @param[in] url  The url to write to
   *
   * @return true if successful
   */
  bool write(RadxVol &vol, const time_t &t, const std::string &url);

  /**
   * Write volume to parameterized url.
   * @param[in] vol Volume to write
   * @param[in] t  Time to write
   *
   * @return true if successful
   */
  bool write(RadxVol &vol, const time_t &t);

  /**
   * @return  reference to local _params object
   */
  inline const AppConfig &parmRef(void) const {return _params;}

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
			  std::vector<RayxData> &data, RayxData &r);

  /**
   * return RayxData of a particular name
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[out] r  The data (a fresh copy)
   * @param[in] showError  If true, lack of data generates a printed error
   *
   * @return true if found the named data in the ray
   */
  static bool retrieveRay(const std::string &name, const RadxRay &ray,
			  RayxData &r, const bool showError=true);

  /**
   * Take some RayxData and modify it based on other inputs
   * @param[in,out] r  RayxData to be modified
   * @param[in] name  Name to give the data
   * @param[in] units  Units to give the data (if non empty)
   * @param[in] missing  Missing data value to give the dat (if units non empty)
   */
  static void modifyRayForOutput(RayxData &r, const std::string &name,
				 const std::string &units="", 
				 const double missing=0);


  /**
   * add data for some RayxData to a RadxRay, then clear out all other fields
   * 
   * @param[in] r  Data to add
   * @param[in,out] ray  Object to add to
   */
  static void updateRay(const RayxData &r, RadxRay &ray);

  /**
   * add data for multiple RayxData's to a RadxRay, clearing out all other fields
   * 
   * @param[in] r  Data to add
   * @param[in,out] ray  Object to add to
   */
  static void updateRay(const vector<RayxData> &r, RadxRay &ray);

protected:
private:  

  std::string _appName;  /**< Name of the app */
  time_t _start;         /**< Start time in ARCHIVE mode */
  time_t _end;           /**< End time in ARCHIVE mode */
  std::vector<std::string> _fileList; /**< paths in FILELIST mode from args */

  AppConfig _params; /**< Algorithm parameters */
  std::vector<std::string> _paths; /**< paths in ARCHIVE or FILELIST mode*/
  int _pathIndex;  /**< Next file to process (index) (ARCHIVE or FILELIST) */
  LdataInfo _ldata;  /**< Triggering mechanism for REALTIME */
  AppConfig::Group _activeGroup;  /**< Used when reading stuff */


  /**
   * Process one file to create a volume
   * @param[in] path  File to process
   * @param[out] vol The volume
   * @param[out] t
   *
   * @return true for success
   */
  bool _processFile(const std::string &path, RadxVol &vol, time_t &t);

  /**
   * Set read request for primary data into the RadxFile object
   * @param[in] file  Object to modify
   */
  void _setupRead(RadxFile &file);

  /**
   * Set read request for secondary data into the RadxFile object
   * @param[in] file  Object to modify
   */
  void _setupSecondaryRead(RadxFile &file);

  /**
   * Set write request into the RadxFile object
   * @param[in] file  Object to modify
   */
  void _setupWrite(RadxFile &file);

  /**
   * Merge the primary and seconday volumes, using the primary
   * volume to hold the merged data
   * 
   * @param[in,out] primaryVol
   * @param[in] secondaryVol
   *
   * @return true for success
   */
  bool _mergeVol(RadxVol &primaryVol, const RadxVol &secondaryVol);

  /**
   * Merge the primary and seconday rays, using the primary
   * ray to hold the merged data
   * 
   * @param[in,out] primaryRay
   * @param[in] secondaryRay
   *
   * @return true for success
   */
  void _mergeRay(RadxRay &primaryRay, const RadxRay &secondaryRay);
};

# endif
