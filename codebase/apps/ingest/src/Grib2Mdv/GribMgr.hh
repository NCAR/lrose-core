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
/////////////////////////////////////////////////////////
// GribMgr
////////////////////////////////////////////////////////
#ifndef _GRIB_MGR
#define _GRIB_MGR

#include <cassert>
#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include <grib/GDS.hh>
#include <grib/BMS.hh>
#include <grib/BDS.hh>
#include <cassert>
#include "Params.hh"

using namespace std;

//
// Forward class declarations
//

/**
 *  \class GribMgr
 *  \author Carl Drews
 *  This class is the base class for various types of grib file converters (ruc, avn, eta).
 *  GribMgr and its derivatives implement the Strategy design pattern.
 */
class GribMgr {

public:

  GribMgr();
  virtual ~GribMgr();

  /// Scans the GRIB record to determine what is in it.
  virtual int inventoryRecord( ui08 *gribPtr );

  /// Extracts the packed information from a single GRIB record.
  virtual int unpackRecord( ui08 *gribPtr, int nx, int ny );

  /// Retrieves the GDS as an Equidistant Cylindrical projection.
  /// This routine replaces the existing _gds object.
  /// \param sectionPtr where to start unpacking
  /// \return error code or 0 if success
  virtual int getEquidistantCylindrical(ui08 *sectionPtr);

  /// Retrieves the GDS as a Polar Stereographic projection.
  /// This routine replaces the existing _gds object.
  /// \param sectionPtr where to start unpacking
  /// \return error code or 0 if success
  virtual int getPolarStereographic(ui08 *sectionPtr);

  /// Some GRIB files have a header before the first 'GRIB' indicator.
  /// This header is illegal according to the formal specification in
  /// http://www.nco.ncep.noaa.gov/pmb/docs/on388/
  /// But we still have to support those files.
  /// This routine advances the file pointer until the first 'GRIB'.
  /// \param fp pointer to a newly opened grib file.  Possibly advanced upon return.
  virtual void findFirstRecord(FILE *fp)                { }

  inline void reset(void){ _forecastTime = -1; }

  inline int getForecastTime(){ return( _forecastTime ); }
  inline time_t getGenerateTime(){ return( _generateTime ); }

  inline int getParameterId(){ return( _pds->getParameterId() ); }
  inline int getLevelId(){ return( _pds->getLevelId() ); }

  /// Sometimes the level is based on the type and bottom-top boundaries.
  virtual int getLevel(){ return( _pds->getLevelVal() ); }

  // do we need to change something for ensemble data?
  virtual bool needsEnsembleChange();

  // reference to ensemble map of code-to-name
  virtual Params::code_name_map_t *getEnsembleMap();
  virtual int getEnsembleMapSize();

  // override these for param values beyond 127 that use a non-NCEP table
  virtual string getLongName();
  virtual string getName();
  virtual string getUnits();

  inline bool isOneLevelVal(){ return( _pds->isOneLevelVal() ); }
  inline int getLevelVal(){ return( _pds->getLevelVal() ); }
  inline int getLevelValTop(){ return( _pds->getLevelValTop() ); }
  inline int getLevelValBottom(){ return( _pds->getLevelValBottom() ); }

  inline fl32 *getData(){ return( _bds->getData() ); }
  inline int getNumValues(){ return( _bds->getNumValues() ); }
  inline const Pjg *getProjection(){ return &(_gds->getProjection()); }
  void setProjection(Pjg &newProjection)	{ _gds->setProjection(newProjection); }

  int getMdvVerticalLevelType();
  static inline fl32 getMissingVal(){ return( BDS::MISSING_DATA ); }

  inline double getFirstLat() {return (_gds->getFirstLat()); }
  inline double getFirstLon() {return (_gds->getFirstLon()); }
  inline double getOriginLat() {return (_gds->getOriginLat()); }
  inline double getOriginLon() {return (_gds->getOriginLon()); }
//  inline double getLov() {return (_gds->getLov()); }
  inline double getLatin1() { assert(_gds != NULL); return (_gds->getLatin1()); }
  inline double getLatin2() {return (_gds->getLatin2()); }

  /// Retrieve the grib record's orientation (South-to-North, West-to-East, etc.)
  GDS::grid_orientation_t getGridOrientation()	{ return _gds->getGridOrientation(); }

  /// Swap the GDS orientation from North-South to South-North
  virtual void swapGridOrientationNS_2_SN();

  /// Generate a unique field name by appending a suffix.
  /// \param name the base name that is non-unique
  /// \param level append some suffix based on this value
  virtual string uniqueFieldName(const string name, const int levelType);

  virtual void printPDS(FILE *stream)	{ _pds->print(stream); }

  /// Some GRIB formats have quasi-regular grids where the number of rows is not fixed,
  /// but decreases toward the poles.  These irregular grids are a problem for MDV and others.
  /// When we encounter these grids we re-map them to a straight rectangle.
  virtual void mapQuasiToRegular()	{ }

  // Pass in a reference to the run-time parameters.
  virtual void setParams(Params *newParams)	{ _paramsPtr = newParams; }

protected:

  IdSec *_id;
  PDS *_pds;
  GDS *_gds;
  BMS *_bms;
  BDS *_bds;

  //
  // These will only be used if the file type is ISOBARIC
  //
  double _pressureDelta;
  double _pressureLowest;

  int _forecastTime;
  time_t _generateTime;

  // run-time parameters
  Params *_paramsPtr;
};

#endif

