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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: SeviriData.cc,v 1.11 2016/03/07 01:23:05 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: SeviriData.cc,v 1.11 2016/03/07 01:23:05 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	SeviriData
//
// Author:	G. M. Cunning
//
// Date:	Wed Jul 25 22:52:57 2007
//
// Description: This class handles reading the netcdf file and provides an 
//		interface to the data to objects that use the data.
//
//


// C++ include files
#include <cassert>
#include <netcdf.hh>
#include <cstring>

// System/RAP include files
#include <toolsa/pmu.h>
#include <euclid/PjgCalc.hh>
#include <euclid/PjgLatlonCalc.hh>

// Local include files
#include "SeviriData.hh"
#include "SeviriConverter.hh"

using namespace std;

// define any constants
const string SeviriData::_className    = "SeviriData";

//
// netcdf dimension names
//
const char* SeviriData::LINES_DIM = "lines";
const char* SeviriData::ELEMS_DIM = "elems";
const char* SeviriData::BANDS_DIM = "bands";
const char* SeviriData::AUDIT_COUNT_DIM = "auditCount";
const char* SeviriData::AUDIT_SIZE_DIM = "auditSize";

//
// netcdf variable names
//
const char* SeviriData::IMAGE_DATE_VAR = "imageDate";
const char* SeviriData::IMAGE_TIME_VAR = "imageTime";
const char* SeviriData::START_LINE_VAR = "startLine";
const char* SeviriData::START_ELEM_VAR = "startElem";
const char* SeviriData::NUM_LINES_VAR = "numLines";
const char* SeviriData::NUM_ELEMS_VAR = "numElems";
const char* SeviriData::DATA_WIDTH_VAR = "dataWidth";
const char* SeviriData::LINE_RES_VAR = "lineRes";
const char* SeviriData::ELEM_RES_VAR = "elemRes";
const char* SeviriData::PREFIX_SIZE_VAR = "prefixSize";
const char* SeviriData::CR_DATE_VAR = "crDate";
const char* SeviriData::CR_TIME_VAR = "crTime";
const char* SeviriData::BANDS_VAR = "bands";
const char* SeviriData:: AUDIT_TRAIL_VAR = "auditTrail";
const char* SeviriData::DATA_VAR = "data";
const char* SeviriData::LATITUDE_VAR = "latitude";
const char* SeviriData::LONGITUDE_VAR = "longitude";

//
// netcdf variable units
//
const char* RES_UNITS = "km";
const char* GRID_UNITS = "degrees";

template <class T> string toString( T value );

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

SeviriData::SeviriData() :
  _params(0),
  _ncFile(0),
  _pjg(0),
  _processData(false),
  _pathName(""),
  _missing(-9999.0),
  _numX(-1),
  _numY(-1),
  _numPts(-1),
  _numBands(-1),
  _dx(-1.0),
  _dy(-1.0),
  _dataWidth(-1),
  _prefixSize(-1),
  _startLine(-1),
  _startElem(-1),
  _elemRes(-1),
  _lineRes(-1)
{
  _pjg = new Pjg();
}

SeviriData::SeviriData(const SeviriData &from) :
  _params(0),
  _ncFile(0),
  _pjg(0),
  _processData(false),
  _pathName(""),
  _missing(-9999.0),
  _numX(-1),
  _numY(-1),
  _numPts(-1),
  _numBands(-1),
  _dx(-1.0),
  _dy(-1.0),
  _dataWidth(-1),
  _prefixSize(-1),
  _startLine(-1),
  _startElem(-1),
  _elemRes(-1),
  _lineRes(-1)
{
   _copy(from);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
SeviriData::~SeviriData()
{

  delete _pjg;
  reset();
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::readFile
//
// Description:	takes care of reading the  netcdf file.
//
// Returns:	if successful, return true. otherwise, return false
//
// Notes:	method will uncompress file, if .gz is the final extention 
//		of file_name.
//
//

bool 
SeviriData::readFile(const string& file_path)
{
  const string methodName = _className + "::readFile";

  // silence the NcFile object
  NcError err(NcError::silent_nonfatal);

  _pathName = file_path;

  _ncFile = new NcFile(file_path.c_str(), NcFile::ReadOnly);
  
  if(!_ncFile->is_valid()) {
    cerr << file_path << " is not a valid netcdf file." << endl;
    return false;
  }
  
  string msg = methodName + " -- pulling out variables from netcdf file";
  PMU_auto_register(msg.c_str());

  //
  // get acces to the dimensions
  //
  bool getDimSuccess = true;
  NcDim *linesDim;
  if (!_getDimension(&linesDim, LINES_DIM)) {
    getDimSuccess = false;
  }

  NcDim *elemsDim;
  if (!_getDimension(&elemsDim, ELEMS_DIM)) {
    getDimSuccess = false;
  }

  NcDim *bandsDim;
  if (!_getDimension(&bandsDim, BANDS_DIM)) {
    getDimSuccess = false;
  }

  NcDim *auditCountDim;
  if (!_getDimension(&auditCountDim, AUDIT_COUNT_DIM)) {
    getDimSuccess = false;
  }

  NcDim *auditSizeDim;
  if (!_getDimension(&auditSizeDim, AUDIT_SIZE_DIM)) {
    getDimSuccess = false;
  }


  //
  // get access to the variables
  //
  bool getVarSuccess = true;
  NcVar* imageDateNc;
  if (!_getVariable(&imageDateNc, IMAGE_DATE_VAR)) {
    getVarSuccess = false;
  }

  NcVar* imageTimeNc;
  if (!_getVariable(&imageTimeNc, IMAGE_TIME_VAR)) {
    getVarSuccess = false;
  }

  NcVar* startLineNc;
  if (!_getVariable(&startLineNc, START_LINE_VAR)) {
    getVarSuccess = false;
  }

  NcVar* startElemNc;
  if (!_getVariable(&startElemNc, START_ELEM_VAR)) {
    getVarSuccess = false;
  }

  NcVar* numLinesNc;
  if (!_getVariable(&numLinesNc, NUM_LINES_VAR)) {
    getVarSuccess = false;
  }

  NcVar* numElemsNc;
  if (!_getVariable(&numElemsNc, NUM_ELEMS_VAR)) {
    getVarSuccess = false;
  }

  NcVar* dataWidthNc;
  if (!_getVariable(&dataWidthNc, DATA_WIDTH_VAR)) {
    getVarSuccess = false;
  }

  NcVar* lineResNc;
  if (!_getVariable(&lineResNc, LINE_RES_VAR)) {
    getVarSuccess = false;
  }

  NcVar* elemResNc;
  if (!_getVariable(&elemResNc, ELEM_RES_VAR)) {
    getVarSuccess = false;
  }

  NcVar* prefixSizeNc;
  if (!_getVariable(&prefixSizeNc, PREFIX_SIZE_VAR)) {
    getVarSuccess = false;
  }

  NcVar* crDateNc;
  if (!_getVariable(&crDateNc ,CR_DATE_VAR)) {
    getVarSuccess = false;
  }

  NcVar* crTimeNc;
  if (!_getVariable(&crTimeNc, CR_TIME_VAR)) {
    getVarSuccess = false;
  }

  NcVar* auditTrailNc;
  if (!_getVariable(&auditTrailNc, AUDIT_TRAIL_VAR)) {
    getVarSuccess = false;
  }

  NcVar* bandsNc;
  if (!_getVariable(&bandsNc, BANDS_VAR)) {
    getVarSuccess = false;
  }

  NcVar* dataNc;
  if (!_getVariable(&dataNc, DATA_VAR)) {
    getVarSuccess = false;
  }

  NcVar* latitudeNc;
  if (!_getVariable(&latitudeNc, LATITUDE_VAR)) {
    getVarSuccess = false;
  }

  NcVar* longitudeNc;
  if (!_getVariable(& longitudeNc, LONGITUDE_VAR)) {
    getVarSuccess = false;
  }

  if (!(getVarSuccess && getVarSuccess)) {
    _ncFile->close();
    delete _ncFile;
    return false;
  }

  _numX = elemsDim->size();
  _numY = linesDim->size();
  _numPts = _numX*_numY;
  _numBands = bandsDim->size();

  int imgDate = _ncFile->get_var(IMAGE_DATE_VAR)->as_int(0);
  int imgTime = _ncFile->get_var(IMAGE_TIME_VAR)->as_int(0);
  _startLine = _ncFile->get_var(START_LINE_VAR)->as_int(0);
  _startElem = _ncFile->get_var(START_ELEM_VAR)->as_int(0);
  int numLine = _ncFile->get_var(NUM_LINES_VAR)->as_int(0);
  int numElem = _ncFile->get_var(NUM_ELEMS_VAR)->as_int(0);
  _dataWidth = _ncFile->get_var(DATA_WIDTH_VAR)->as_int(0);
  int elemRes = _ncFile->get_var(ELEM_RES_VAR)->as_int(0);
  int lineRes = _ncFile->get_var(LINE_RES_VAR)->as_int(0);
  _prefixSize = _ncFile->get_var(PREFIX_SIZE_VAR)->as_int(0);
  int crDate = _ncFile->get_var(CR_DATE_VAR)->as_int(0);
  int crTime = _ncFile->get_var(CR_TIME_VAR)->as_int(0);

  assert(_numX == numElem);
  assert(_numY == numLine);
  
  //
  // set the image and creation DateTime objects
  // need a method to handle the fucking julian day bullshit
  //
  _setTime(imgDate, imgTime, _imageTime);
  _setTime(crDate, crTime, _creationTime);
  
  if (_params->debug_mode == Params::DEBUG_VERBOSE) {
    cout << "Dimension info:" << endl;
    cout << "\t" << LINES_DIM << " = " << _numX << endl;
    cout << "\t" << ELEMS_DIM << " = " << _numY << endl;
    cout << "\t" << BANDS_DIM << " = " << _numBands << endl;
    cout << "\t" << AUDIT_COUNT_DIM << " = " << auditCountDim->size() << endl;
    cout << "\t" << AUDIT_SIZE_DIM << " = " << auditSizeDim->size() << endl << endl;
    cout << "Variable info:" << endl;
    cout << "\t" << IMAGE_DATE_VAR  << " = " << imgDate << endl;
    cout << "\t" << IMAGE_TIME_VAR  << " = " << imgTime << endl;
    cout << "\t" << START_LINE_VAR  << " = " << _startLine << endl;
    cout << "\t" << START_ELEM_VAR  << " = " << _startElem << endl;
    cout << "\t" << NUM_LINES_VAR  << " = " << numLine << endl;
    cout << "\t" << NUM_ELEMS_VAR  << " = " << numElem << endl;
    cout << "\t" << DATA_WIDTH_VAR  << " = " << _dataWidth << endl;
    cout << "\t" << ELEM_RES_VAR  << " = " << elemRes << endl;
    cout << "\t" << LINE_RES_VAR  << " = " << lineRes << endl;
    cout << "\t" << PREFIX_SIZE_VAR  << " = " << _prefixSize << endl;
    cout << "\t" << CR_DATE_VAR  << " = " << crDate << endl;
    cout << "\t" << CR_TIME_VAR  << " = " << crTime << endl << endl;
    cout << "The image time is " << _imageTime.getStr() << endl;
    cout << "The creation time is " << _creationTime.getStr() << endl << endl;
  }


  //
  // setup the Pjg object;
  //

  _setProjection(latitudeNc, longitudeNc);

  //
  // copy the band numbers and data
  //
  float* ncVals = new float[_numPts];
  for (int i = 0; i < _numBands; i++) {

    int band_num = bandsNc->as_int(i);
    if (!_processBand(band_num)) {
      continue;
    }

    _bandNums.push_back(band_num);

    int bandInfoIndex = _getBandInfoIndex(band_num);

    float* data = new float[_numPts];

    dataNc->get(ncVals, (i+1), _numY, _numX);

    for (int k = 0; k < _numY; k++) {
      for (int j = 0; j < _numX; j++) {
	int idxMdv = j + k*_numX;
	int idxNc = j + (_numY-1-k)*_numX;

	data[idxMdv] = _missing;

	if(ncVals[idxNc] < _params->_band_info[bandInfoIndex].lower_count_limit) {
	  if(_params->limit_fill_mode == Params::LIMIT_FILL_CLIP) {
	    data[idxMdv] = _params->_band_info[bandInfoIndex].lower_count_limit;
	  }
	}
	else if(ncVals[idxNc] < _params->_band_info[bandInfoIndex].upper_count_limit) {
	  if(_params->limit_fill_mode == Params::LIMIT_FILL_CLIP) {
	    data[idxMdv] = _params->_band_info[bandInfoIndex].upper_count_limit;
	  }
	}
	else {
	  data[idxMdv] = ncVals[idxNc];
	}
      } 
    }
    _bandData.push_back(data);
    
  }
  delete [] ncVals;

  _ncFile->close();
  delete _ncFile;

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::convertUnits
//
// Description:	takes care of converting the units
//
// Returns:	none
//
// Notes:
//
//

void
SeviriData::convertUnits(SeviriConverter* converter)
{
  const string methodName = _className + "::convertUnits";

  for (size_t i = 0; i < _bandData.size(); i++) {
    converter->calculateRadiances(_bandNums[i], _numPts, _missing, &(_bandData[i]));
    converter->calculateBrightnessTemps(_bandNums[i], _numPts, _missing, &(_bandData[i]));
  }

}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::reset
//
// Description:	takes  care of resetting object
//
// Returns:	none
//
// Notes:	
//

void 
SeviriData::reset()
{
  const string methodName = _className + "::reset";

  _numX = -1;
  _numY = -1;
  _numPts = -1;
  _numBands = -1;
  _startLine = -1;
  _startElem = -1;
  _dataWidth = -1;
  _prefixSize = -1;
  _imageTime = -1;
  _creationTime = -1;

  for (size_t i = 0; i < _bandData.size(); i++) {
    delete [] _bandData[i];    
  }
  _bandData.erase(_bandData.begin(), _bandData.end());
  _bandNums.erase(_bandNums.begin(), _bandNums.end());

}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::operator=
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

SeviriData& 
SeviriData::operator=(const SeviriData& from)
{
  _copy(from);

  return *this;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::_copy
//
// Description:	take care of making a copy
//
// Returns:	
//
// Notes:
//
//

void 
SeviriData::_copy(const SeviriData& from)
{
  _params = from._params;
  _ncFile = from._ncFile;
  _numX = from._numX; 
  _numY = from._numY;
  _numPts = from._numPts;
  _numBands = from._numBands;
  _dx = from._dx;
  _dy = from._dy;
  _dataWidth = from._dataWidth;
  _prefixSize = from._prefixSize;
  _startLine = from. _startLine; 
  _startElem = from._startElem; 
  _elemRes = from._elemRes;
  _lineRes = from._lineRes;
  _imageTime = from._imageTime; 
  _creationTime = from._creationTime;

  _bandNums = from. _bandNums;

  _pjg = from._pjg;

  for (size_t i = 0; i < from._bandData.size(); i++) {
    float *data = new float[_numPts];
    for (int j = 0; j < _numPts; j++) {
      data[j] = from._bandData[i][j];
    }
    _bandData.push_back(data);
  }



}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::_getDimension
//
// Description:	wrapper around nNcFile::get_dim
//
// Returns:	
//
// Notes:
//
//

bool 
SeviriData::_getDimension(NcDim **dim, const char* id)
{
  const string methodName = _className + "::_getDimension";

  *dim = 0;
  *dim = _ncFile->get_dim(id);
  if (!(*dim)) {
    cerr << methodName << " -- failed to get " << id << " dimension." << endl;
    return false;
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::_getDimension
//
// Description:	wrapper around nNcFile::get_var
//
// Returns:	
//
// Notes:
//
//

bool 
SeviriData::_getVariable(NcVar **var, const char* id)
{
  const string methodName = _className + "::_getVariable";

  *var = 0;
  *var = _ncFile->get_var(id);
  if (!(*var)) {
    cerr << methodName << " -- failed to get " << id << " variable." << endl;
    return false;
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::_setTime
//
// Description:	this method takes care of hiding the mess of parsing 
//		out day-of-the-year from the_date and sets datetime.
//
// Returns:	
//
// Notes:
//
//

void 
SeviriData::_setTime(int the_date, int the_time, 
		       DateTime& datetime)
{
  int year = the_date/1000;
  int doy = the_date - year*1000;
  int hour = the_time/10000;
  int minute  = (the_time - hour*10000)/100;
  int second   = the_time - hour*10000 - minute*100;

  datetime.setByDayOfYear(year, doy, hour, minute, second);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SeviriData::_setProjection
//
// Description:	sets the Pjg object.
//
// Returns:	
//
// Notes:	Based on the audit trail in files used in development, 
//		it is assumed that all netcdf files use a rectilinear
//		projection. At RAl, this is referred to as a latlon
//		projection.
//
//		Mcidas grid reference is the lower left corner, so shift
//		minX and MinY accordingly.
//
//

void 
SeviriData::_setProjection(const NcVar* latitude, const NcVar* longitude)
{

  float startLon = longitude->as_float(0); 
  float endLon = longitude->as_float(_numX-1); 
  float startLat = latitude->as_float(0);
  float endLat = latitude->as_float(_numPts-1);
  float deltaLon = fabs(startLon - endLon);
  float deltaLat= fabs(startLat - endLat);
  _dx = deltaLon/static_cast<float>(_numX);
  _dy = deltaLat/static_cast<float>(_numY);

  float minX = startLon + _dx/2.0; 
  float minY = startLat + _dy/2.0; 

  _pjg->initLatlon(_numX, _numY, 1, _dx, _dy, 0.0, minX, minY);
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name: SeviriData::_processBand
//
// Description:	checks if current band should be processed
//
// Returns:	true or false
//
// Notes:
//
//

bool
SeviriData::_processBand(int num)
{
  const string methodName = _className + string( "::_processBand" );

  _processData = false;

  if (_params->write_all_bands) {
    _processData = true;
  }

  for (int i = 0; i < _params->band_list_n; i++) {
    
    if (_params->_band_list[i] == num) {
      _processData = true;
    }

  }

  return _processData;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name: SeviriData::_getBandInfoIndex
//
// Description:	checks if current band should be processed
//
// Returns:	index to entry in _parmas->_band_info
//
// Notes:
//
//

int
SeviriData::_getBandInfoIndex(int band_num)
{
  const string methodName = _className + string( "::_getBandInfoIndex" );

  int index = 0;
  for(int i = 0; i < _params->band_info_n; i++) {
    if(band_num == _params->_band_info[i].number) {
      index = i;
      break;
    }
  }

  return index;
}

