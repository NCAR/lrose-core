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
//////////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <climits>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <rapmath/math_macros.h>
#include <toolsa/umisc.h>
#include <euclid/Pjg.hh>
#include <grib/PDS.hh>
#include <cassert>
#include <grib/constants.h>

#include "GribField.hh"
#include "Ingester.hh"
#include "Grib2Mdv.hh"
#include "RucGribMgr.hh"
#include "AvnGribMgr.hh"
#include "WafsGribMgr.hh"
#include "AfwaGribMgr.hh"
#include "DtraGribMgr.hh"


using namespace std;


Ingester::Ingester( Params  &params)
{
  _missingVal = GribMgr::getMissingVal();
  _eofFound = false;
  _gribBuf = MEMbufCreate();

  _paramsPtr = &params;
  _gribMgr = NULL;

  _outputFields.insert( _outputFields.begin(), _paramsPtr->_output_fields, 
			(_paramsPtr->_output_fields)+(_paramsPtr->output_fields_n) );
}


Ingester::~Ingester()
{
  MEMbufDelete( _gribBuf );
}


/// Allocate the correct type of GribMgr.
/// The GribMgr will get deleted in tearDown();
/// \param inputGribType is the kind of GRIB file to process
void Ingester::setup(Params::grib_type_t inputGribType)
{
  if (inputGribType == Params::GRIB_TYPE_RUC) {
    _gribMgr = new RucGribMgr();

  } else if (inputGribType == Params::GRIB_TYPE_AVN
    // Surprise!  ETA files can be treated like AVN.
    || inputGribType == Params::GRIB_TYPE_ETA) {
    _gribMgr = new AvnGribMgr();

  } else if (inputGribType == Params::GRIB_TYPE_WAFS) {
    _gribMgr = new WafsGribMgr();

  } else if (inputGribType == Params::GRIB_TYPE_AFWA) {
    _gribMgr = new AfwaGribMgr();

  } else if (inputGribType == Params::GRIB_TYPE_DTRA) {
    _gribMgr = new DtraGribMgr();

  } else {
    cout << "Error:  Grib type " << inputGribType << " is not yet supported." << endl;
    assert(false);
  }

  _gribMgr->setParams(_paramsPtr);
}


int
Ingester::processFile( string& filePath )
{
  FILE *fp;
  ui08 *gribPtr;

  //
  // make a copy of the output field id list, so that entries can be removed when 
  // found in grib file
  //
  list<Params::out_field_t> outFieldsFound = _outputFields;

  int recordCount = 1;
  int unitConv;

  //
  // New file - reset eof flag and forecast time
  //
  _eofFound = false;
  _gribMgr->reset();

  //
  // Open the file
  //
  if((fp = ta_fopen_uncompress((char *) filePath.c_str(), "rb")) == NULL) {
    cerr << "WARNING: Couldn't open file " << filePath << endl << flush;
    return(-1);
  }

  while( !_eofFound ) {
    // move file pointer to first character in the GRIB record
    _gribMgr->findFirstRecord(fp);

    // load the next GRIB record
    bool loadResult = loadNextGrib(fp);
    if (_eofFound) {
      continue;
    }
    if (!loadResult) {
      cerr << "WARNING: Loading of " << filePath <<
        " terminated on record " << recordCount << endl;
      break;
    }
    gribPtr = (ui08*) MEMbufPtr(_gribBuf);

    assert(_gribMgr != NULL);
    int ret = _gribMgr->inventoryRecord(gribPtr);
    if(ret == -2) {
      // -2 means that a 'garbage' record was found. This is a recoverable
      // error, so continue trying to inventory the files
      //
      // increment the record count. or the starting point for reading a field
      // will be off by one record
      //
      ++recordCount;
      cerr << "WARNING: Could not inventory GRIB record." << endl << flush;
      continue;
    }
    else if(ret == -1) {
      cerr << "ERROR: Could not inventory GRIB record." << endl << flush;
      return(-1);
    }

    // check if this is a requested record
    if(_foundField(_gribMgr->getParameterId(), _gribMgr->getLevelId(), 
		    outFieldsFound, unitConv)) { 

      // extract record contents
      PMU_force_register( "Processing grib record" );
      int ret = _gribMgr->unpackRecord( gribPtr, _gribMgr->getProjection()->getNx(),
        _gribMgr->getProjection()->getNy() );
      if(ret == -1) {
	cerr << "WARNING: Could not unpack GRIB record." << endl << flush;
	continue;
      }

      // Note some NCEP Grib files have Corrupt headers. - This is used to override these
      // settings and use a consistant convention.
      if(_paramsPtr->always_swap_ns) {
          _gribMgr->swapGridOrientationNS_2_SN();
      } else  {
        // swap orientation so that latitude has positive increments (South to North)
        if (_gribMgr->getGridOrientation() == GDS::GO_NS_WE) {
          // Grib AVN data is ordered North to South (W to E), MDV S to N (W to E)
          _gribMgr->swapGridOrientationNS_2_SN();
        }
      }

      // if the grid is an irregular quasi-grid, map it to a rectangle
      _gribMgr->mapQuasiToRegular();

      // copy record contents into a grib field
      GribField* gfp;

      if(_isFieldNew(_gribMgr->getParameterId(), _gribMgr->getLevelId())) { 

	_gribFields.push_back(new GribField(_paramsPtr->debug));
	gfp = _gribFields.back();
	
	//
	// need a hack to get around the water vapor mixing ratio problem.
	// 
	if((_gribMgr->getParameterId() == 185) && 
	   (_paramsPtr->runOnOldRuc40File)) {
	  gfp->init(_gribMgr->getParameterId(), _gribMgr->getLevelId(),
		    _gribMgr->getGenerateTime(), _gribMgr->getForecastTime(),
		    _gribMgr->getMdvVerticalLevelType(), unitConv, "WVMR", 
		    "Water vapor mixing ratio", "kg/kg", _gribMgr->getProjection());
	}
	else {
	  gfp->init(_gribMgr->getParameterId(), _gribMgr->getLevelId(),
		    _gribMgr->getGenerateTime(), _gribMgr->getForecastTime(),
		    _gribMgr->getMdvVerticalLevelType(), unitConv, 
		    _gribMgr->getName(), _gribMgr->getLongName(),
                    _gribMgr->getUnits(), _gribMgr->getProjection());
	}
      }
      else {
	gfp = _getField(_gribMgr->getParameterId(), _gribMgr->getLevelId());
      }

      gfp->addPlane(_gribMgr->getLevel(), _gribMgr->getData() );
    }
    ++recordCount;
  }

  if(_paramsPtr->debug) {
    cout << "Loaded " << recordCount - 1 << " records." << endl;
  }

  // completed pass through file. now assemble the data volumes for each of the fields
  list<GribField*>::iterator gfi;
  for(gfi = _gribFields.begin(); gfi != _gribFields.end(); gfi++) {
    (*gfi)->assemble();

    if(_paramsPtr->debug) {
      (*gfi)->print();
    }

  }

  fclose(fp);
  return(0);
   
}


bool Ingester::loadNextGrib(FILE *fp)
{
  // Read just the GRIB Indicator Section.  This section will tell us the size of the grib record,
  // so we can allocate memory for it and read in the correct number of bytes first time.
  IdSec tempIndicator;
  size_t indicatorSize = tempIndicator.getSize();
  char *indicatorBuffer = new char[indicatorSize];
  if (fread(indicatorBuffer, sizeof(char), indicatorSize, fp) != indicatorSize) {
    _eofFound = true;
    delete [] indicatorBuffer;
    return false;
  }

  // examine the indicator section
  if (tempIndicator.unpack((ui08 *)indicatorBuffer) != GRIB_SUCCESS) {
    cerr << "ERROR: Cannot unpack ID section" << endl;
    return false;
  }
  int recordSize = tempIndicator.getTotalSize();

  // resize buffer to handle the entire grib record
  MEMbufPrepare(_gribBuf, recordSize);

  // copy indicator into the grib buffer
  ui08 *gribPtr = (ui08 *)MEMbufPtr(_gribBuf);
  memcpy(gribPtr, indicatorBuffer, indicatorSize);

  // read the bulk of the grib record directly into the buffer
  size_t bytesRead = fread(gribPtr + indicatorSize, sizeof(char), recordSize - indicatorSize, fp);
  if (bytesRead != recordSize - indicatorSize) {
    cout << "Ingester::loadNextGrib read " << bytesRead << " bytes but expected "
      << recordSize - indicatorSize << endl;

    // free memory and return
    delete [] indicatorBuffer;
    return false;
  }

  // free memory and return
  delete [] indicatorBuffer;
  return true;
}


bool
Ingester::_foundField( const int& param_id, const int& level_id, 
			  list<Params::out_field_t>& out, int& uc )
// param_id is what's in the file, out is what the user requested
{
  // check if we are outputting all fields
  if (_paramsPtr->output_all_fields) {
    return true;
  }

  list<Params::out_field_t>::iterator ofi;

  // check parameter against the requested output fields
  for( ofi = out.begin(); ofi != out.end(); ofi++ ) {
    if ((ofi->param_id == param_id) && (ofi->level_id == level_id)) {
      uc = (int)ofi->out_units;
      return true;
    }
  }

  // check to see if parameter needed for derived fields
  if (param_id == Params::UGRD || param_id == Params::VGRD) {
    for( ofi = out.begin(); ofi != out.end(); ofi++ ) {
      if ((ofi->param_id == Params::WIND || ofi->param_id == Params::WDIR)
        && (ofi->level_id == level_id)) {
        uc = (int)ofi->out_units;
        return true;
      }
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
}


/// Delete the GribMgr that was allocated in setup().
void Ingester::tearDown()
{
  // delete the grib manager
  assert(_gribMgr != NULL);
  _gribMgr->reset();
  delete _gribMgr;
  _gribMgr = NULL;
}

