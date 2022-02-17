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
//////////////////////////////////////////////////////////////////
// GFSrecord - Reads a Grib record and loads the various Grib
//              objects that make up the Grib file.
//
//
//////////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <values.h>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>
#include <rapmath/math_macros.h>
#include <toolsa/umisc.h>
#include <cstdio>

#include <grib/IdSec.hh>
#include <grib/PDS.hh>
#include <grib/BDS.hh>
#include <grib/BMS.hh>
#include <grib/gds_.hh>
#include <grib/gds_equidistant.hh>
#include <grib/GribVertType.hh>

#include <Mdv/Mdvx.hh>

#include <toolsa/MemBuf.hh>

#include "GFSrecord.hh"
#include "GfsIngest.hh"
using namespace std;

GFSrecord::GFSrecord ()
{
   _eofFound         = false;
   _gribRec          = new MemBuf();

  _indicatorSec = new IdSec();
  _prodDef = new PDS();
  _gridDes = new gds_equidistant();
  _bitMap = new BMS();
  _dataSec = new BDS();

}

GFSrecord::~GFSrecord()
{
  delete _gribRec;

  delete _indicatorSec;
  delete _prodDef;
  delete _gridDes;
  delete _bitMap;
  delete _dataSec;

}

// reorders data from North to South to South to North
// East/West increments remain the same
// For GFS data this means  that the data starts in the 
// lower left hand corner instead of the upper left hand 
// corner. 

void
GFSrecord::reOrderNS_2_SN (fl32 *data, int numX, int numY)
{
  int j = 0;
  MemBuf *gribData = new MemBuf();

  fl32 *bufPtr = (fl32 *) gribData->load 
                     ((void *) data, numX * numY * sizeof (fl32));

  for (int y = numY - 1; y >= 0; y--) {
    for (int x = 0;  x < numX; x++) {
      data[j] = bufPtr[(y * numX) + x];
      j++;
    }
  }

  // switch the beginning and end latitude to reflect reordering
  double oldStartLat = getStartLat();

  setStartLat(getEndLat());
  setEndLat(oldStartLat);
  delete gribData;

}

int
GFSrecord::loadGribRec(FILE *fp, int recPosition)
{
  ui08 *gribPtr = NULL;
  ui08 *bitMapPtr = NULL;
  int nPts = 0;
  int gribLen = 0;

  _eofFound = false;
  fseek(fp, recPosition, SEEK_SET);

  _gribRec->reset();

  // determine the record size

  gribPtr = (ui08 *) _gribRec->reserve (8);

  if ((gribLen = (int) fread((void *)gribPtr, sizeof(char),
                                      8, fp)) != 8 ) {
    if( ferror(fp) != 0 ) {
      cout << "ERROR: reading from file " << endl;
      return ((ui08) 0);
    }
  }

  if (feof(fp)) {
    _eofFound = true;
cout << "EOF FOUND " << endl;
    return RI_SUCCESS;
  }

  if (_indicatorSec->unpack(gribPtr) != RI_SUCCESS)
    cout << "Error unpacking the Indicator Section" << endl;

  gribLen = _indicatorSec->getTotalSize();

  gribPtr = (ui08 *)_gribRec->reserve(gribLen);
  //_indicatorSec->print(stdout);

  if (((int) fread((void *)gribPtr, sizeof(char),
				             gribLen - 8, fp)) != gribLen - 8 ) {
	  if( ferror(fp) != 0 ) {
		cout << "ERROR: reading from file " << endl;
		return ((ui08) 0);
	  }
  }

  if (_prodDef->unpack(gribPtr) != RI_SUCCESS)
	  cout << "Error unpacking the Product Description Section" << endl;

  //_prodDef->print(stdout);
	
  gribPtr += _prodDef->getSize();

  if (_prodDef->gdsUsed ()) {
	  if (_gridDes->unpack(gribPtr) != RI_SUCCESS)
		cout << "Error unpacking the Grid Description Section" << endl;
      //_gridDes->print(stdout);
	  
	  gribPtr += _gridDes->getSize();
	  nPts = _gridDes->getGridDim();

  }

  if (_prodDef->bmsUsed()) {
	if (_bitMap->unpack(gribPtr) != RI_SUCCESS)
		cout << "Error unpacking the Bit Map Section" << endl;
	    // int nBitMapPts = _bitMap->getBitMapSize();
		  
        bitMapPtr = _bitMap->getBitMap();
	    gribPtr += _bitMap->getSize();

  }

  if (_dataSec->unpack(gribPtr, nPts,
				        _prodDef->getDecimalScale(), 0, bitMapPtr,
						(float) _prodDef->getLevelValBottom(),
						(float) _prodDef->getLevelValTop()) != RI_SUCCESS)
		  cout << "Error unpacking the Binary Data Section" << endl;

  //_dataSec->print(stdout);
	
  //return(RI_SUCCESS);
  return(gribLen);
}


	  


int
GFSrecord::getMdvVerticalLevelType()
{
  GribVertType::vert_type_t grib_type = _prodDef->getVerticalLevelType();
  
  switch (grib_type)
  {
  case GribVertType::SURFACE :
  case GribVertType::POTENTIAL_VORTICITY :
  case GribVertType::UNKNOWN_211 :
  case GribVertType::UNKNOWN_212 :
  case GribVertType::UNKNOWN_213 :
  case GribVertType::UNKNOWN_214 :
  case GribVertType::UNKNOWN_222 :
  case GribVertType::UNKNOWN_223 :
  case GribVertType::UNKNOWN_224 :
  case GribVertType::UNKNOWN_232 :
  case GribVertType::UNKNOWN_233 :
  case GribVertType::UNKNOWN_234 :
  case GribVertType::UNKNOWN_242 :
  case GribVertType::UNKNOWN_244 :
    return Mdvx::VERT_TYPE_SURFACE;
    
  case GribVertType::ISOBARIC :
  case GribVertType::BETWEEN_PRS_DIFF :
    return Mdvx::VERT_TYPE_PRESSURE;
    
  case GribVertType::MEAN_SEA_LEVEL :
  case GribVertType::ALTITUDE_ABOVE_MSL :
  case GribVertType::HEIGHT_ABOVE_GROUND :
  case GribVertType::SIGMA :
  case GribVertType::BETWEEN_SIGMA :
  case GribVertType::DEPTH_BELOW_LAND_SURFACE :
  case GribVertType::BETWEEN_DEPTH :
    return Mdvx::VERT_TYPE_Z;
    
  case GribVertType::HYBRID :
    return Mdvx::VERT_TYPE_SIGMA_Z;
    
  case GribVertType::CLOUD_BASE :
  case GribVertType::CLOUD_TOPS :
  case GribVertType::ZERO_ISOTHERM :
  case GribVertType::MAX_WIND :
  case GribVertType::TROPOPAUSE :
  case GribVertType::NOMINAL_ATM_TOP :
  case GribVertType::ENTIRE_ATMOSPHERE :
  case GribVertType::UNKNOWN_10 :
  case GribVertType::FREEZING_LEVEL :
  case GribVertType::CONV_CLD_TOP :
  case GribVertType::MAX_EQ_THETA_PRESSURE :
  case GribVertType::EQUILIBRIUM_LEVEL_HEIGHT :
    return Mdvx::VERT_FIELDS_VAR_ELEV;
    
  default:
    return -1;
    
  } /* endswitch - grib_type */
}
