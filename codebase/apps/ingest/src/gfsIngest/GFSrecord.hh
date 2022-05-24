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
// GFSrecord
//
//
////////////////////////////////////////////////////////
#ifndef _GFS_RECORD_
#define _GFS_RECORD_

#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>

#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include <grib/gds_.hh>
#include <grib/gds_equidistant.hh>
#include <grib/BDS.hh>
#include <grib/BMS.hh>
#include "Params.hh"
using namespace std;


class GFSrecord {
		
public:

  GFSrecord ();
  ~GFSrecord();
   
  int loadGribRec (FILE *, int recPos);
  void reOrderNS_2_SN (fl32 *, int numX, int numY);

  // Print methods
  inline void printIdSec (FILE *stream) { _indicatorSec->print(stream); };
  inline void printPDS (FILE *stream) { _prodDef->print(stream); };
  inline void printGDS (FILE *stream) { _gridDes->print(stream); };
  inline void printBDS (FILE *stream) { _dataSec->print(stream); };

  // Product Definition access methods
  inline int getForecastTime() { return _prodDef->getForecastTime(); };
  inline time_t getGenerateTime() { return _prodDef->getGenerateTime(); };
  inline int getParameterId() { return _prodDef->getParameterId(); };
  inline string getName(){ return( _prodDef->getName() ); }
  inline string getLongName(){ return( _prodDef->getLongName() ); }
  inline int getVerticalLevelType(){ return( _prodDef->getVerticalLevelType() ); }
  inline string getUnits(){ return( _prodDef->getUnits() ); }
  int getMdvVerticalLevelType();
  inline int getLevelVal() { return _prodDef->getLevelVal(); };

  // Get record data
  inline fl32  *getData() { return _dataSec->getData(); };

  // Grid Description access methods
  inline void  setStartLat (double newVal) { _gridDes->setFirstLat(newVal); };
  inline void  setEndLat (double newVal) { _gridDes->setEndLat(newVal); };
  inline double  getStartLon() { return _gridDes->getFirstLon(); };
  inline double  getStartLat() { return _gridDes->getFirstLat(); };
  inline double  getEndLat() { return _gridDes->getEndLat(); };
  inline int  getNx() { return _gridDes->getNx(); };
  inline int  getNy() { return _gridDes->getNy(); };
  inline double  getDx() { return _gridDes->getDx(); };
  inline double  getDy() { return _gridDes->getDy(); };
  inline gds::grid_orientation_t _getGridOrientation() { return _gridDes->getGridOrientation(); };

  inline bool  eof() { return _eofFound; };

private:

  bool _eofFound;

  MemBuf *_gribRec;

  // Indicator section
  IdSec *_indicatorSec;

  // Product Definition Section
  PDS *_prodDef;

  // Optional Grid Description Section, abstract base type Gds
  // for GFS Lat/Lon / Equidistant Cylindrical / Plate Carree projection is used
  gds *_gridDes;

  // Bit Map Section
  BMS *_bitMap;

  // Binary Data Section
  BDS *_dataSec;


};

#endif
