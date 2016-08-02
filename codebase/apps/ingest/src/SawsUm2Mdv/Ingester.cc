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
/////////////////////////////////////////////////////////////
// Ingester.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
// Copied from Grib2Mdv
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/TaArray.hh>
#include <grib/constants.h>
#include "Ingester.hh"
#include "SawsUm2Mdv.hh"
using namespace std;

// constructor

Ingester::Ingester(SawsUm2Mdv &parent,
		   const string &prog_name,
		   const Params  &params) :
  _parent(parent),
  _progName(prog_name),
  _params(params)
{
  _missingVal = GribMgr::getMissingVal();
  _eofFound = false;
}

// destructor

Ingester::~Ingester()
{
  _clearInputFields();
}

///////////////////////////////////////////////
// read a file
//
// Returns 0 on success, -1 on failure

int Ingester::readFile(const string& filePath)
{

  // make sure _fields are empty

  _clearInputFields();
  
  FILE *fp;
  ui08 *gribPtr;

  // make a copy of the output field id vector, so that entries can be removed
  // when found in grib file

  int recordCount = 1;
  
  // New file - reset eof flag and forecast time
  
  _eofFound = false;
  _gribMgr.reset();

  // Open the file
  
  if((fp = ta_fopen_uncompress((char *) filePath.c_str(), "rb")) == NULL) {
    cerr << "WARNING: Couldn't open file " << filePath << endl << flush;
    return(-1);
  }

  while(!_eofFound) {

    // move file pointer to first character in the GRIB record
    _gribMgr.findFirstRecord(fp);

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
    gribPtr = (ui08 *) _gribBuf.getPtr();
    
    int ret = _gribMgr.inventoryRecord(gribPtr);

    if(ret == -2) {

      // -2 means that a 'garbage' record was found. This is a recoverable
      // error, so continue trying to inventory the files

      // increment the record count. or the starting point for reading a field
      // will be off by one record

      ++recordCount;
      if (_params.debug) {
	cerr << "WARNING: Could not inventory GRIB record." << endl << flush;
      }
      continue;
      
    } else if(ret == -1) {

      if (_params.debug) {
	cerr << "ERROR: Could not inventory GRIB record." << endl << flush;
      }
      fclose(fp);
      return(-1);

    }

    // check if this is a requested record
    
    OutputField *outField = _parent.getOutputField(_gribMgr.getParameterId());
    if (outField != NULL) {
      
      // extract record contents
      PMU_auto_register("Processing grib record");
      int ret = _gribMgr.unpackRecord(gribPtr,
				      _gribMgr.getProjection()->getNx(),
				      _gribMgr.getProjection()->getNy());
      
      if(ret == -1 && _params.debug) {
	cerr << "WARNING: Could not unpack GRIB record." << endl << flush;
	continue;
      }
      
      // maybe swap orientation so that latitude has
      // positive increments (South to North)
      // Grib AVN data is ordered North to South (W to E),
      // MDV S to N (W to E)
      
      if (_gribMgr.getGridOrientation() == GDS::GO_NS_WE) {
	_gribMgr.swapGridOrientationNS_2_SN();
      }
      
      // if the grid is an irregular quasi-grid, map it to a rectangle

      _gribMgr.mapQuasiToRegular();
      
      // create a new field object if needed
      
      if (_gribMgr.getLevelId() == GRIB_ISOBARIC) {
	
	if(_getField(_gribMgr.getParameterId()) == NULL) {
	  // need a new field object
	  InputField *gfp =
	    new InputField(_progName, _params,
			  _gribMgr.getParameterId(),
			  _gribMgr.getLevelId(),
			  _gribMgr.getGenerateTime(),
			  _gribMgr.getForecastTime(),
			  outField->getUnitsConversion(), 
			  _gribMgr.getName(),
			  _gribMgr.getLongName(),
			  _gribMgr.getUnits(),
			  _gribMgr.getProjection());
	  _fields.push_back(gfp);
	  outField->addInputField(gfp);
	} // if(_getField ...
        
	// add plane to grib field
	
	InputField* gfp = _getField(_gribMgr.getParameterId());
	if (gfp != NULL) {
	  gfp->addPlane(_gribMgr.getLevel(), _gribMgr.getData());
	}
	
      } else {
	
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "WARNING: " << _progName
	       << "::Ingester::readFile" << endl;
	  cerr << "Field: " << _gribMgr.getName()
	       << " is not ISOBARIC, it will be ignored" << endl;
	  cerr << "  levelId: " << _gribMgr.getLevelId() << endl;
	}
	
      } // if (_gribMgr.getLevelId() ...

    } // if(_outputFieldExists ...

    ++recordCount;

  } // while
  
  // close file
  fclose(fp);

  if(_params.debug >= Params::DEBUG_VERBOSE) {
    cout << "Loaded " << recordCount - 1 << " records." << endl;
  }

  // Completed pass through file.
  // Now assemble the data in each field

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->assemble();
  }
  
  return 0;
   
}

/////////////////////////////////////////
// load next grib record

bool Ingester::loadNextGrib(FILE *fp)
{

  // Read just the GRIB Indicator Section.  This section will tell us
  // the size of the grib record, so we can allocate memory for it and
  // read in the correct number of bytes first time.

  IdSec tempIndicator;
  size_t indicatorSize = tempIndicator.getSize();
  TaArray<char> indicatorBuffer_;
  char *indicatorBuffer = indicatorBuffer_.alloc(indicatorSize);
  if (fread(indicatorBuffer, sizeof(char), indicatorSize, fp)
      != indicatorSize) {
    _eofFound = true;
    return false;
  }

  // examine the indicator section
  if (tempIndicator.unpack((ui08 *)indicatorBuffer) != GRIB_SUCCESS) {
    return false;
  }
  int recordSize = tempIndicator.getTotalSize();

  // resize buffer to handle the entire grib record
  _gribBuf.prepare(recordSize);

  // copy indicator into the grib buffer
  ui08 *gribPtr = (ui08 *) _gribBuf.getPtr();
  memcpy(gribPtr, indicatorBuffer, indicatorSize);

  // read the bulk of the grib record directly into the buffer
  size_t bytesRead = fread(gribPtr + indicatorSize,
			   sizeof(char), recordSize - indicatorSize, fp);
  if (bytesRead != recordSize - indicatorSize) {
    cerr << "ERROR - Ingester::loadNextGrib" << endl;
    cerr << "  Read nbytes: " << bytesRead << endl;
    cerr << "  Expected nbytes: " << recordSize - indicatorSize << endl;
    return false;
  }

  // free memory and return
  return true;

}

//////////////////////////////////////////////////
// find field
//
// Side effect: sets units conversion in uc

bool Ingester::_outputFieldExists(const int field_id, int& uc)
{

  for (int ii = 0; ii < _params.output_fields_n; ii++) {
    if (field_id == _params._output_fields[ii].grib_field_id) {
      uc = (int) _params._output_fields[ii].units_conversion;
      return true;
    }
  }
  return false;
}
  

///////////////////////////////////////////////////////////////
// Get field
// Returns NULL on failure

InputField*
Ingester::_getField(const int field_id)
{
  vector<InputField*>::iterator gfi;
  for(gfi = _fields.begin(); gfi != _fields.end(); gfi++) {
    if (((*gfi)->getFieldId() == field_id)) {
      return *gfi;
    }
  }
  return NULL;
}

//////////////////////////
// clear allocated memory

void Ingester::_clearInputFields()
{
  for(size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}
