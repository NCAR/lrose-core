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
 * @file RadxApp.hh
 * @brief Actions at the top level common to Radx algorithms
 * @class RadxApp
 * @brief Actions at the top level common to Radx algorithms
 */
# ifndef    RADX_APP_HH
# define    RADX_APP_HH

#include <rapmath/MathParser.hh>
#include <Radx/RayxData.hh>

class MathData;
class VolumeData;
class RadxAppVolume;
class RadxAppParms;
class RadxAppParams;
class RadxRay;

//------------------------------------------------------------------
class RadxApp
{
public:

  /**
   * Constructor
   *
   * @param[in] sweepData  Sweep data example
   * @param[in] rayData    Ray data example
   * @param[in] vdata      Volume data example
   */
  RadxApp(const MathData &sweepData, const MathData &rayData,
	  const VolumeData &vdata);

  /**
   * Constructor
   * 
   * @param[in] parms  Parameters shared by all apps
   * @param[in] sweepData  Sweep data example
   * @param[in] rayData    Ray data example
   * @param[in] vdata      Volume data example
   */
  RadxApp(const RadxAppParms &parms, const MathData &sweepData,
	  const MathData &rayData, const VolumeData &vdata);

  /**
   * Destructor
   */
  virtual ~RadxApp(void);

  /**
   * @return true if object well formed
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Print all the operators derived and fixed to stdout
   */
  void printOperators(void) const;

  /**
   * Standard Initialization for RadxApp
   *
   * @return true if successful
   *
   * @param[in] appName  Name of app for PMU calls
   * @param[in] p  Parameters common to all radx apps
   * @param[in] cleanup  Cleanup method
   */
  static bool algInit(const std::string &appName, const RadxAppParams &p, 
		      void cleanup(int));

  /**
   * Standard finishing step for RadxApp
   */
  static void algFinish(void);

  /**
   * Process a volume
   *
   * @param[in] P   Algorithm parameters
   * @param[in] volume  The volume data, inputs modified to include outputs
   * @return true for success
   */
  bool update(const RadxAppParms &P, RadxAppVolume *volume);

  /**
   * Write volume to configured URL
   * @param[in] vol Volume to write
   *
   * @return true if successful
   */
  bool write(RadxAppVolume *vol);

  /**
   * Write volume to specified URL
   * @param[in] vol Volume to write
   * @param[in] url
   *
   * @return true if successful
   */
  bool write(RadxAppVolume *vol, const std::string &url);

  /**
   * return a copy of RayxData of a particular name
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[in] showError  If true, lack of data generates a printed error
   *
   * @return pointer to a new object that is a copy, or NULL
   *
   * Calling routine owns the returned pointer
   */
  static 
  RayxData *retrieveRayPtr(const std::string &name, const RadxRay &ray,
			   const bool showError=true);


  /**
   * return RadxData of a particular name
   *
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[in] data Zero or more RadxData objects, one of which might have the
   *                 name
   * @param[out] isNew set True if the returned object was newly created
   *                   and is owned by calling routine, false if it points
   *                   to an existing object and is not to be freed by caller
   * @param[in] showError  If true, lack of data generates a printed error
   * @return pointer to object, or NULL
   */
  static RayxData *retrieveRayPtr(const std::string &name, const RadxRay &ray,
				  std::vector<RayxData> &data, 
				  bool &isNew, bool showError=true);

  /**
   * return RayxData, any field
   *
   * @param[in] ray  Input data ray
   *
   * @return pointer to a new object that is a copy, or NULL
   *
   * Calling routine owns the returned pointer
   */
  static RayxData *retrieveAnyRayPtr(const RadxRay &ray);

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
   * return RadxData of a particular name
   * @param[in] name  The name to match
   * @param[in] ray  Input data ray (which might have the named data)
   * @param[in] data Zero or more RadxData objects, one of which might have the
   *                 name
   * @param[out] r  The data (a fresh copy)
   * @param[in] showError  If true, lack of data generates a printed error
   * @return true if found the named data in either the ray or in the data
   */
  static bool retrieveRay(const std::string &name, const RadxRay &ray,
  			  const std::vector<RayxData> &data, RayxData &r,
  			  bool showError=true);

  /**
   * return RayxData, any field
   * @param[in] ray  Input data ray
   * @param[out] r  The data (a fresh copy)
   *
   * @return true if found example data in the ray
   */
  static bool retrieveAnyRay(const RadxRay &ray, RayxData &r);

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
   * @param[in,out] ray  Object to add to/clear
   */
  static void updateRay(const RayxData &r, RadxRay &ray);

  /**
   * add data for multiple RayxData's to a RadxRay, clearing out all other
   * fields
   * @param[in] r  Data to add
   * @param[in,out] ray  Object to add to/clear
   */
  static void updateRay(const vector<RayxData> &r, RadxRay &ray);

protected:
private:  

  bool _ok;             /**< True if object well formed */
  MathParser _p;        /**< The math handler, executes all filter steps */

  void _setupUserUnaryOps(const MathData &sweepData, const MathData &rayData,
			  const VolumeData &vdata);
};

# endif
