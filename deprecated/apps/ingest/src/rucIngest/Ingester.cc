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
// Ingester
//
// $Id: Ingester.cc,v 1.33 2016/03/07 01:23:11 dixon Exp $
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
#include <euclid/Pjg.hh>
#include <grib/PDS.hh>
#include <toolsa/port.h>

#include "GribField.hh"
#include "Ingester.hh"
#include "RucIngest.hh"

using namespace std;

Ingester::Ingester( Params  &params)
{
  _eofFound = false;
  _missingVal = _gribMgr.getMissingVal();
  _paramsPtr = &params;
  _outputFields.insert( _outputFields.begin(), _paramsPtr->_output_fields, 
			(_paramsPtr->_output_fields)+(_paramsPtr->output_fields_n) );

}

Ingester::~Ingester()
{

  _cleanup();
}

  
int
Ingester::processFile( string& filePath )
{
  FILE *fp;

  //
  // make a copy of the output field id list, so that entries can be removed when 
  // found in grib file
  //
  list<Params::out_field_t> outFieldsFound = _outputFields;

  int recordCount = 1;
  int unitConv;
  float upperLimit;
  float lowerLimit;

  // 
  // get uncompressed file size
  //
  stat_struct_t fileStat;
  if(ta_stat_uncompress((char *) filePath.c_str(), &fileStat) == -1) {
    cerr << "WARNING: Couldn't stat file " << filePath << endl << flush;
    return(-1);
  }

  //
  // get file size
  //
  size_t fileSize = static_cast<long>(fileStat.st_size);
  if (_paramsPtr->debug)
    cerr << "   file size is " << fileSize << " bytes."<< endl;

  //
  // allocate space for file
  //
  ui08 *gribBuf = new ui08[fileSize];
  ui08 *bufStart = gribBuf;

  //
  // Open the file
  //
  if((fp = ta_fopen_uncompress((char *) filePath.c_str(), "r")) == NULL) {
    cerr << "WARNING: Couldn't open file " << filePath << endl << flush;
    return(-1);
  }
   
  //
  // Read in entire file
  //
  size_t readLen = 0;

  if( (readLen = fread((void *)bufStart, 
		       sizeof(ui08), 
		       fileSize, 
		       fp)) !=  fileSize) {
    if( ferror(fp) != 0 ) {
      return( -1 );
    }
  }

  if (_paramsPtr->debug)
    cerr << "   read " << readLen << " bytes."<< endl;

  ui08 *gribPtr = gribBuf;

  //
  // subtract 4 bytes from filesize to account for the termination flagg '7777' at the
  // end of the file
  //
  while(static_cast<size_t>(gribPtr-bufStart) < (fileSize-4)) {


    size_t revOffset = fileSize - static_cast<size_t>(gribPtr-bufStart);
    //
    // locate next GRIB record
    //
    _findGrib(&gribPtr, revOffset);

    int ret = _gribMgr.inventoryRecord(gribPtr);
    if(ret == -2) {
      // -2 means that a 'garbage' record was found. This is a recoverable
      // error, so continue trying to inventory the files
      //
      // increment the record count. or the starting point for reading a field
      // will be off by one record
      //
      ++recordCount;
      ++gribPtr;
      cerr << "WARNING: Could not inventory GRIB record." << endl << flush;
      continue;
    }
    else if(ret == -1) {
      cerr << "ERROR: Could not inventory GRIB record." << endl << flush;
      return(-1);
      delete [] gribBuf;
    }

    const Pjg *gribPjg = _gribMgr.getProjection();

    // check if this is a requested record
    if(_foundField(_gribMgr.getParameterId(), _gribMgr.getLevelId(), 
		    outFieldsFound, unitConv, upperLimit, lowerLimit)) { 

      PMU_force_register( "Processing grib record" );
      
      GribField* gfp;

      if(_isFieldNew(_gribMgr.getParameterId(), _gribMgr.getLevelId())) { 
	
	_gribFields.push_back(new GribField(_paramsPtr->debug));
	gfp = _gribFields.back();
	
	//
	// need a hack to get around the water vapor mixing ratio problem.
	// 
	if((_gribMgr.getParameterId() == 185) && 
	   (_paramsPtr->runOnOldRuc40File)) {
	  gfp->init(_gribMgr.getParameterId(), _gribMgr.getLevelId(),
		    _gribMgr.getGenerateTime(), _gribMgr.getForecastTime(),
		    _gribMgr.getMdvVerticalLevelType(), unitConv, upperLimit, 
		    lowerLimit, "WVMR", "Water vapor mixing ratio", "kg/kg", gribPjg);
	}
	else if(_gribMgr.getParameterId() == 58) {
	  gfp->init(_gribMgr.getParameterId(), _gribMgr.getLevelId(),
		    _gribMgr.getGenerateTime(), _gribMgr.getForecastTime(),
		    _gribMgr.getMdvVerticalLevelType(), unitConv, upperLimit, 
		    lowerLimit, "ICMR", "Ice mixing ratio", "kg/kg", gribPjg);
	}
	else {
	  gfp->init(_gribMgr.getParameterId(), _gribMgr.getLevelId(),
		    _gribMgr.getGenerateTime(), _gribMgr.getForecastTime(),
		    _gribMgr.getMdvVerticalLevelType(), unitConv, upperLimit, 
		    lowerLimit, _gribMgr.getName(), _gribMgr.getLongName(), 
		    _gribMgr.getUnits(), gribPjg);
	}
      }
      else {
	gfp = _getField(_gribMgr.getParameterId(), _gribMgr.getLevelId());
      }

      //
      // Process the record
      //
      int ret = _gribMgr.unpackRecord( gribPtr, gribPjg->getNx(), gribPjg->getNy() );
      if(ret == -1) {
	cerr << "WARNING: Could not unpack GRIB record." << endl << flush;
	continue;
      }

      gfp->addPlane(_gribMgr.getLevel(), _gribMgr.getData() );

    } 
    ++gribPtr;
    ++recordCount;
  }

  // 
  // completed pass through file. now assemble the data volumes for each of the fields
  //
  list<GribField*>::iterator gfi;
  for(gfi = _gribFields.begin(); gfi != _gribFields.end(); gfi++) {
    (*gfi)->assemble();

    if(_paramsPtr->debug) {
      (*gfi)->print();
    }

  }

  delete [] gribBuf;
  fclose(fp);
  return(0);
   
}


size_t
Ingester::_findGrib( ui08 **gribPtr, size_t bufLen )
{

  //
  // scan buffer to end minus four bytes
  //
  size_t bufCount = 0;
  while(bufCount <  bufLen-4 ) {
    if( strncmp( (char*)*gribPtr, "GRIB", 4 ) == 0) {
      break;
    }
    (*gribPtr)++;
    bufCount++;
  }
 
  return(bufCount);
}

bool
Ingester::_foundField( const int& param_id, const int& level_id, 
		       list<Params::out_field_t>& out, int& uc, 
		       float& u_rng, float& l_rng)
{

  list<Params::out_field_t>::iterator ofi;

  for( ofi = out.begin(); ofi != out.end(); ofi++ ) {
    if ((ofi->param_id == param_id) && (ofi->level_id == level_id)) {
      uc = (int)ofi->out_units;
      u_rng = (float)ofi->upper_range_limit;
      l_rng = (float)ofi->lower_range_limit;
      return true;
    }
  }

  return false;
}


bool
Ingester::_isFieldNew(const int& param_id, const int& level_id)
{

  list<GribField*>::iterator gfi;
  
  for( gfi = _gribFields.begin(); gfi != _gribFields.end(); gfi++ ) {
    if (((*gfi)->getParameterId() == param_id) && ((*gfi)->getLevelId() == level_id)) {
      return false;
    }
  }

  return true;
}


GribField*
Ingester::_getField(const int& param_id, const int& level_id)
{

  list<GribField*>::iterator gfi;
  
  for( gfi = _gribFields.begin(); gfi != _gribFields.end(); gfi++ ) {
    if (((*gfi)->getParameterId() == param_id) && ((*gfi)->getLevelId() == level_id)) {
      return *gfi;
    }
  }

  return 0;
}


void Ingester::_cleanup()
{
  if(!_gribFields.empty()) {
    for(list<GribField*>::iterator i = _gribFields.begin(); i != _gribFields.end(); i++) {
      delete *i;
      *i = 0;
    }

    _gribFields.erase(_gribFields.begin(), _gribFields.end() );
  }

  _gribMgr.reset(); 
}

