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
//////////////////////////////////////////////////////////
// Mdvx_timelist.cc
//
// Timelist routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1999
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/udatetime.h>
#include <toolsa/TaStr.hh>
#include <didss/RapDataDir.hh>
#include <didss/LdataInfo.hh>
#include <toolsa/ReadDir.hh>
#include <iomanip>
#include <iterator>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
using namespace std;

/////////////////////////////////////////////////////////////////
// setTimeListModeValid
//
// Set the time list so that it finds all of the valid data
// times between the start and end times.
// For forecast data where multiple forecasts exist for the same
// valid time, a single valid time will be returned.

void Mdvx::setTimeListModeValid(const string &dir,
				time_t start_time,
				time_t end_time)

{
  _timeList.setModeValid(dir, start_time, end_time);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeGen
//
// set the time list so that it finds all of the
// generate times between the start and end times

void Mdvx::setTimeListModeGen(const string &dir,
			      time_t start_gen_time,
			      time_t end_gen_time)
  
{
  _timeList.setModeGen(dir, start_gen_time, end_gen_time);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeForecast
//
// set the time list so that it finds all of the forecast
// times for the given generate time

void Mdvx::setTimeListModeForecast(const string &dir,
				   time_t gen_time)
  
{
  _timeList.setModeForecast(dir, gen_time);
}

// setTimeListModeLead() is deprecated

void Mdvx::setTimeListModeLead(const string &dir,
			       time_t gen_time)
{
  setTimeListModeForecast(dir, gen_time);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeGenPlusForecasts
//
// Set the time list so that it finds all of the
// generate times between the start and end gen times.
// Then, for each generate time, all of the forecast times are
// found. These are made available in the
// _forecastTimesArray, which is represented by vector<vector<time_t> >
  
void Mdvx::setTimeListModeGenPlusForecasts(const string &dir,
					   time_t start_gen_time,
					   time_t end_gen_time)
  
{
  _timeList.setModeGenPlusForecasts
    (dir, start_gen_time, end_gen_time);
}

/////////////////////////////////////////////////////////////////
// setModeValidMultGen
//
// Set the time list so that it finds all of the forecasts
// within the time interval specified. For each forecast found
// the associated generate time is also determined.
// The forecast times will be available in the _timeList array.
// The generate times will be available in the _genTimes array.

void Mdvx::setTimeListModeValidMultGen(const string &dir,
				       time_t start_time,
				       time_t end_time)

{
  _timeList.setModeValidMultGen(dir, start_time, end_time);
}
  
/////////////////////////////////////////////////////////////////
// setTimeListModeFirst
//
// set the time list mode so that it finds the first data time

void Mdvx::setTimeListModeFirst(const string &dir)

{
  _timeList.setModeFirst(dir);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeLast
//
// set the time list mode so that it finds the last data time

void Mdvx::setTimeListModeLast(const string &dir)

{
  _timeList.setModeLast(dir);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeClosest
//
// set the time list mode so that it finds the closest available data time
// to the search time within the search margin

void Mdvx::setTimeListModeClosest(const string &dir,
				  time_t search_time,
				  int time_margin)

{
  _timeList.setModeClosest(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstBefore
//
// set the time list mode so that it finds the first available data time
// before the search time within the search margin

void Mdvx::setTimeListModeFirstBefore(const string &dir,
				      time_t search_time,
				      int time_margin)

{
  _timeList.setModeFirstBefore(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeFirstAfter
//
// set the time list mode so that it finds the first available data time
// after the search time within the search margin

void Mdvx::setTimeListModeFirstAfter(const string &dir,
				     time_t search_time,
				     int time_margin)

{
  _timeList.setModeFirstAfter(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeBestForecast
//
// Set the time list so that it returns the best forecast
// for the search time, within the time margin

void Mdvx::setTimeListModeBestForecast(const string &dir,
				       time_t search_time,
				       int time_margin)

{
  _timeList.setModeBestForecast(dir, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// setTimeListModeSpecifiedForecast
//
// Set the time list so that it returns the forecast for the given
// generate time, closest to the search time, within the time margin

void Mdvx::setTimeListModeSpecifiedForecast(const string &dir,
					    time_t gen_time,
					    time_t search_time,
					    int time_margin)

{
  _timeList.setModeSpecifiedForecast(dir, gen_time, search_time, time_margin);
}

/////////////////////////////////////////////////////////////////
// clearTimeListMode
//
// clear out time list mode info

void Mdvx::clearTimeListMode()

{
  _timeList.clearMode();
}

//////////////////////////
// print time list request

void Mdvx::printTimeListRequest(ostream &out)

{

  out << "================== Time-list request ====================" << endl;
  _timeList.printRequest(out);
  if (_readHorizLimitsSet) {
    out << "  Min lat: " << _readMinLat << endl;
    out << "  Min lon: " << _readMinLon << endl;
    out << "  Max lat: " << _readMaxLat << endl;
    out << "  Max lon: " << _readMaxLon << endl;
  }
  out << "===========================================================" << endl;

}

//////////////////////////
// print time height request

void Mdvx::printTimeHeightRequest(ostream &out)

{
  out << "================== Time-height request ====================" << endl;
  _timeList.printRequest(out);
  printReadRequest(out);
  out << "===========================================================" << endl;
}

//////////////////////////////////////////////////////////
// compile time list
//
// Compile a list of available data times in the specified
// directory, according to the mode set.
//
// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.
//
// After a successful call to compileTimeList(), use the get methods
// to access time list information.

int Mdvx::compileTimeList()
  
{ 
  clearErrStr();
  if (_timeList.compile()) {
    _errStr = "ERROR - Mdvx::compileTimeList\n";
    _errStr + _timeList.getErrStr();
    return -1;
  }
  if (_timeList.getValidTimes().size() == 0) {
    _noFilesFoundOnRead = true;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
// Compile time-height (time-series) profile, according to the
// read settings and time list specifications.
//
// Before using this call:
//
// (a) Set up read parameters, such as fields, vlevels, encoding
//     type etc.
// (b) Set up the lat/lon of the point to be sampled, using:
//     addReadWayPt(lat, lon)
// (c) Set up the time list, using setTimeListMode????()
//
// Data fields returned will be time-height profiles, with time
// in the x dimension, ny = 1, height in the z dimension.
//
// Actual times for the data will be returned in the time lists,
// namely validTimes and genTimes.

// Returns 0 on success, -1 on failure
// getErrStr() retrieves the error string.

int Mdvx::compileTimeHeight()
  
{

  // clear fields, chunks

  clearFields();
  clearChunks();
  clearErrStr();
  _errStr += "ERROR - Mdvx::compileTimeHeight\n";

  // check a few things
  
  if (_timeList.getMode() == MdvxTimeList::MODE_UNDEFINED) {
    _errStr += "  You must specify a time list mode.\n";
    return -1;
  }

  if (_vsectWayPts.size() != 1) {
    _errStr += "  You must specify a single way-pt.\n";
    return -1;
  }

  // compile the time list
  
  if (compileTimeList()) {
    _errStr += "  Cannot compile time list.\n";
    return -1;
  }
  if (_timeList.getValidTimes().size() == 0) {
    _errStr += "  No suitable times found.\n";
    return -1;
  }

  // compile the time-height profile
  
  if (_compileTimeHeight()) {
    return -1;
  }
  
  clearErrStr();
  return 0;

}

////////////////////////////////////////////////////
// compile the time-height profile

int Mdvx::_compileTimeHeight() 

{

  // read in the vertical point for each time
  
  const vector<string> &paths = _timeList.getPathList();
  if (paths.size() == 0) {
    _errStr += "  No suitable times found.\n";
    _noFilesFoundOnRead = true;
    return -1;
  }
  
  // make a copy of this Mdvx object
  
  Mdvx vsect0(*this);
  vsect0.clearFields();
  vsect0.clearChunks();

  // read in first height profile
  
  vsect0.setReadPath(paths[0]);
  vsect0.setReadEncodingType(ENCODING_FLOAT32);
  vsect0.setReadCompressionType(COMPRESSION_NONE);
  vsect0._timeList.clearMode();
  vsect0._timeList.clearData();
  vsect0.clearReadTimeListAlso();
  if (vsect0.readVsection()) {
    TaStr::AddStr(_errStr,
                  "  Cannot read in vsection for path: ", paths[0]);
    _errStr += vsect0.getErrStr();
    return -1;
  }

  // load up master header
  
  const vector<time_t> &validTimes = _timeList.getValidTimes();
  _mhdr = vsect0._mhdr;
  _mhdr.time_begin = validTimes[0];
  _mhdr.time_end = validTimes[validTimes.size() - 1];
  _mhdr.time_centroid = _mhdr.time_end;
  _mhdr.num_data_times = (si32) validTimes.size();
  _mhdr.data_dimension = 2;
  _mhdr.n_chunks = 0;
  setDataSetInfo("Time height profile");

  // set up fields
  
  fl32 missing = -9999.0;
  vector<MdvxField *> fields;
  
  for (size_t ifield = 0; ifield < vsect0.getNFields(); ifield++) {
    
    const MdvxField *fileField = vsect0.getField(ifield);
    if (fileField != NULL) {

      field_header_t fhdr = fileField->getFieldHeader();
      vlevel_header_t vhdr = fileField->getVlevelHeader();
      
      // customize the headers
      
      int nx = (int) paths.size();
      double minx = (double) validTimes[0];
      double maxx = (double) validTimes[validTimes.size() - 1];
      double dx = (maxx - minx) / (nx - 1.0);
      fhdr.nx = nx;
      fhdr.grid_dx = dx;
      fhdr.grid_minx = minx;
      
      fhdr.ny = 1;
      fhdr.grid_miny = 0;
      fhdr.grid_dy = 1.0;
      
      fhdr.proj_type = PROJ_TIME_HEIGHT;
      fhdr.proj_rotation = 0;
      fhdr.bad_data_value = missing;
      fhdr.missing_data_value = missing;
      
      fhdr.data_element_nbytes = sizeof(fl32);
      fhdr.volume_size =
        (fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes);
      
      // create a new field object
      // this constructor allocates space for the data and fills the
      // array with missing values
      
      MdvxField *fld = new MdvxField(fhdr, vhdr, NULL, true, false);
      
      // add field to vector
      
      fields.push_back(fld);

    } // if (fileField != NULL) 

  } // ifield

  // free up data in vsect0

  vsect0.clearFields();
  vsect0.clearChunks();

  // loop through all entries in time list

  for (size_t ix = 0; ix < paths.size(); ix++) {
    
    // make a copy of this Mdvx object
  
    Mdvx vsect(*this);
    vsect.clearFields();
    vsect.clearChunks();
    vsect.clearErrStr();

    // read in height profile
  
    vsect.setReadPath(paths[ix]);
    vsect.setReadEncodingType(ENCODING_FLOAT32);
    vsect.setReadCompressionType(COMPRESSION_NONE);
    vsect._timeList.clearMode();
    vsect._timeList.clearData();
    vsect.clearReadTimeListAlso();
    if (vsect.readVsection()) {
      TaStr::AddStr(_errStr,
                    "  Cannot read in vsection for path: ", paths[ix]);
      _errStr += vsect.getErrStr();
      // free up fields
      for (size_t jj = 0; jj < fields.size(); jj++) {
        delete fields[jj];
      }
      return -1;
    }

    // loop through fields

    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      
      MdvxField *fld = fields[ifield];
      field_header_t &fhdr = fld->_fhdr;
      vlevel_header_t &vhdr = fld->_vhdr;
      fl32 *data = (fl32 *) fld->_volBuf.getPtr();
      
      const MdvxField *fileField = vsect.getField(fld->getFieldName());
      if (fileField == NULL) {
        continue;
      }
      
      const field_header_t &fileFhdr = fileField->getFieldHeader();
      const vlevel_header_t &fileVhdr = fileField->getVlevelHeader();
      const fl32 *fileData = (const fl32*) fileField->getVol();
      fl32 fileMissing = fileFhdr.missing_data_value;
      
      // loop through the vlevels
        
      for (int iz = 0; iz < fhdr.nz; iz++) {
        
        // find the relevant vlevel index in the file data
        
        int fileIz = -1;
        for (int jz = 0; jz < fileFhdr.nz; jz++) {
          if (fabs(fileVhdr.level[jz] - vhdr.level[iz]) < 0.00001) {
            fileIz = jz;
          }
        }
        
        // if corresponding vlevel found, and the data is not missing,
        // copy it in

        if (fileIz >= 0) {
          fl32 fileVal = fileData[fileIz];
          if (fileVal != fileMissing) {
            int dataIndex = iz * fhdr.nx + ix;
            data[dataIndex] = fileVal;
          }
        }
        
      } // iz
      
    } // ifield
    
  } // ix
  
  // add the fields

  for (int ifield = 0; ifield < (int) fields.size(); ifield++) {
    MdvxField *fld = fields[ifield];
    fld->computeMinAndMax(true);
    fld->convertType(_readEncodingType, _readCompressionType,
                     _readScalingType, _readScale, _readBias);
    addField(fld);
  }

  // update the master header to match the field headers
  
  updateMasterHeader();
  
  return 0;

}

////////////////////////////////////////////////
// get a requested field from the index
  
const MdvxField *Mdvx::_getRequestedField(int request_index)
  
{

  const MdvxField *fld = NULL;
  if (_readFieldNames.size() > 0) {
    fld = getFieldByName(_readFieldNames[request_index]);
    if (fld == NULL) {
      TaStr::AddStr(_errStr, "  Cannot find requested field name: ",
		    _readFieldNames[request_index]);
      TaStr::AddStr(_errStr, "    File path: ", _pathInUse);
    }
  } else {
    fld = getFieldByNum(request_index);
    if (fld == NULL) {
      TaStr::AddInt(_errStr, "  Cannot find requested field number: ",
		    request_index);
      TaStr::AddStr(_errStr, "    File path: ", _pathInUse);
    }
  }

  return fld;

}

