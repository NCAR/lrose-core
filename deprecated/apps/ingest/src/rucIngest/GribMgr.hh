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
//
// $Id: GribMgr.hh,v 1.8 2016/03/07 01:23:11 dixon Exp $
//
////////////////////////////////////////////////////////
#ifndef _GRIB_MGR
#define _GRIB_MGR

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>
#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include <grib/GDS.hh>
#include <grib/BMS.hh>
#include <grib/BDS.hh>
using namespace std;

//
// Forward class declarations
//

class GribMgr {

public:

  GribMgr();
  ~GribMgr();

  int inventoryRecord( ui08 *gribPtr );
  int unpackRecord( ui08 *gribPtr, int nx, int ny );
  inline void reset(void){ _forecastTime = -1; }

  inline int getForecastTime(){ return( _forecastTime ); }
  inline time_t getGenerateTime(){ return( _generateTime ); }

  inline int getParameterId(){ return( _pds->getParameterId() ); }
  inline int getLevelId(){ return( _pds->getLevelId() ); }
  inline int getLevel(){ return( _pds->getLevelVal() ); }
  inline string getLongName(){ return( _pds->getLongName() ); }
  inline string getName(){ return( _pds->getName() ); }
  inline string getUnits(){ return( _pds->getUnits() ); }

  inline bool isOneLevelVal(){ return( _pds->isOneLevelVal() ); }
  inline int getLevelVal(){ return( _pds->getLevelVal() ); }
  inline int getLevelValTop(){ return( _pds->getLevelValTop() ); }
  inline int getLevelValBottom(){ return( _pds->getLevelValBottom() ); }

  inline fl32 *getData(){ return( _bds->getData() ); }
  inline int getNumValues(){ return( _bds->getNumValues() ); }
  inline const Pjg *getProjection(){ return &(_gds->getProjection()); }

  int getMdvVerticalLevelType();
  inline fl32 getMissingVal(){ return( BDS::MISSING_DATA ); }

  inline double getFirstLat() {return (_gds->getFirstLat()); }
  inline double getFirstLon() {return (_gds->getFirstLon()); }
  inline double getOriginLat() {return (_gds->getOriginLat()); }
  inline double getOriginLon() {return (_gds->getOriginLon()); }
//  inline double getLov() {return (_gds->getLov()); }
  inline double getLatin1() {return (_gds->getLatin1()); }
  inline double getLatin2() {return (_gds->getLatin2()); }

private:

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
  
   
};

#endif
