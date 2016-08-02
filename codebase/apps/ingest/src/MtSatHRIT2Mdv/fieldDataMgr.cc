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
// fieldDataMgr.cc
//
// Implements fieldDataMgr class
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include "fieldDataMgr.hh"

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>

#include <toolsa/pmu.h>
#include <toolsa/file_io.h>

#include "Params.hh"

// Constructor, from filename.
fieldDataMgr::fieldDataMgr ( char *fileName, bool isVis, Params *TDRP_params){

  // See if this field has been requested.
   _thisFieldRequested = true;
   if (isVis && (TDRP_params->outputFields == Params::OUTPUT_IR)){
     _thisFieldRequested = false; _ok = true;
     _haveNorth = true; _haveSouth = true;
     return;
   }

   if (!(isVis) && (TDRP_params->outputFields == Params::OUTPUT_VIS)){
     _thisFieldRequested = false; _ok = true;
     _haveNorth = true; _haveSouth = true;
     return;
   }


  // Init values.

  _params = TDRP_params;
  _haveNorth = false; _haveSouth = false;

  _fieldNx = _fieldNy = 0;
  _fieldTime = 0L;
  _subLon = _coff = _loff = _cfac = _lfac = 0.0;
  _fieldData = NULL; _byteData = NULL; _pixelData = NULL;


  _nx = _ny = 0;
  _convertPoints.clear();

  _ok = false; // Start off pessimistic.
  _isVis = isVis;

  if ((_isVis) && (_params->fillMissingVis) && (0 == strncmp("NONE", fileName, 4))) {
    _haveNorth = true; _haveSouth = true;  _ok = true; // Can't miss, we're filling data
  }

  if (0 == strncmp("NONE", fileName, 4)) return;

  if (_params->mode == Params::REALTIME){
    //
    // Check the quiescent time.
    //
    struct stat buf;
    if (stat(fileName, &buf)){
      cerr << "Stat failed on " << fileName << endl;
      return;
    }
    
    unsigned long lastSize = buf.st_size;
    unsigned long currentSize = lastSize;
    
    do {
   
      if (_params->debug){
	cerr << "File size is " << currentSize;
	cerr << " checking quiesence over ";
	cerr << _params->fileFindTimeout.qTime << " seconds." << endl;
      }
   
      lastSize = currentSize;
      
      for (int q=0; q < _params->fileFindTimeout.qTime; q++){
	sleep(1);
	PMU_auto_register("Wait for Qtime check");
      }
      
      if (stat(fileName, &buf)){
	cerr << "stat failed on " << fileName << endl;
	return;
      }
      
      currentSize = buf.st_size;
      
    } while(currentSize != lastSize);

  }



  //
  // Open the input file.
  //
  FILE *fp = ta_fopen_uncompress(fileName, "r");
  if (fp == NULL){
    if (_params->debug){
      cerr << "open uncompress failed on " << fileName << endl;
    }
    return;
  }

  PMU_auto_register("Reading data file");

  fseek(fp, 0, SEEK_END);
  long fileSize = ftell(fp);
  rewind(fp);

  _byteData = (ui08 *)malloc(fileSize * sizeof(ui08));
  if (_byteData == NULL){
    cerr << "Malloc failed on file " << fileName << endl;
    exit(-1);
  }
  _pixelData = (ui16 *) _byteData;

  long numRead = fread(_byteData, sizeof(ui08), fileSize, fp);
  if (numRead != fileSize){
    cerr << "Read failed on file " << fileName << endl;
    return;
  }

  fclose(fp);

  if (_params->debug) cerr << fileSize << " bytes read from " << fileName << endl;

  if (_isVis){
    _nx = 11000; _ny = 11000;
  } else {
    _nx = 2750; _ny = 2750;
  }

  if (fileSize < _nx*_ny*2){
    //
    // Must be a half disk image.
    //
    _ny = _ny / 2;
    if (_params->debug) cerr << "Half disk image, " << _nx << " by " << _ny << endl;
  } else {
    if (_params->debug) cerr << "Full disk image, " << _nx << " by " << _ny << endl;
  }

  //
  // Byte swap the two byte pixel data.
  //
  for (int i=0; i < 2 * _nx * _ny; i=i+2){
    ui08 t = _byteData[i];
    _byteData[i] = _byteData[i+1];
    _byteData[i+1] = t;
  }


  //
  // 
  //
  int min=0, max=0;
  double total = 0.0;
  int num = 0;

  int first = 1;
  for (int i=0; i <  _nx * _ny; i++){
    if (_pixelData[i] != 65535){
      total += _pixelData[i];
      num++;
      if (first){
	first = 0;
	min = max = _pixelData[i];
      } else {
	if (_pixelData[i] < min) min = _pixelData[i];
	if (_pixelData[i] > max) max = _pixelData[i];
      }
    }
  }

  double mean = total / double(num);

  if (_params->debug){
    cerr << "Two byte values in " << fileName << " range from ";
    cerr << min << " to " << max << " range " << max - min << " mean " << mean << endl;
  }
  //
  // Read the trailing header data (oxymoron).
  //
  int ip = 2 * _nx * _ny + 28;

  char geoString[33];
  geoString[32] = char(0);
  memcpy(geoString, _byteData + ip, 32);

  if (1 != sscanf(geoString,"GEOS(%lf)", &_subLon)){
    cerr << "Error parsing center lon from " << geoString << endl;
    return;
  }

  if (_params->debug) 
    cerr << "Satellite above longitude " << _subLon << endl;


  ip = _nx * _ny * 2 + 60;
  //
  // Byte swap to get line, col co-eff's and offsets.
  //
  for (int k=0; k < 4; k++){
    unsigned char b0 = _byteData[4*k+ip];
    unsigned char b1 = _byteData[4*k+ip+1];
    unsigned char b2 = _byteData[4*k+ip+2];
    unsigned char b3 = _byteData[4*k+ip+3]; 

    _byteData[4*k+ip+3] = b0;
    _byteData[4*k+ip+2] = b1;
    _byteData[4*k+ip+1] = b2;
    _byteData[4*k+ip] = b3;
  }

  //
  // Then get the offsets and co-effs.
  //
  for (int k=0; k < 4; k++){
    int *p = (int *) (_byteData + 4 * k + ip);

    switch( k ){

    case 0 :
      _cfac = *p;
      break;

    case 1 :
      _lfac = *p;
      break;

    case 2 :
      _coff = *p;
      break;

    case 3 :
      _loff = *p;
      break;
    }
  }

  if (_params->debug){
    cerr << "cfac, lfac, coff, loff : " << _cfac << ", ";
    cerr << _lfac << ", " << _coff << ", " << _loff << endl;
  }

  ip = _nx * _ny * 2 + 79;

  char bufString[1024];
  int sp = 0; // String pointer.

  while (ip < fileSize) {

    if (!(iscntrl((int)_byteData[ip]))){
      bufString[sp] = char(_byteData[ip]);
      sp++;
      bufString[sp] = char(0);
    } else {
      sp = 0;
      _convertPoint_t p;
      if (2==sscanf(bufString,"%d:=%lf", &p.countVal, &p.physicalVal)){
	if (!(_isVis)) p.physicalVal -=  273.16; // Go from K to C
	if (p.countVal < 1024) _convertPoints.push_back( p );
      }
    }
    ip++;
    if (_byteData[ip] ==4) break;
  }

  if (_convertPoints.size() == 0){
    cerr << "No conversion points could be parsed in " << fileName << endl;
    return;
  }

  if (_params->debug){
    int icount = 0;
    cerr << _convertPoints.size() << " physical value conversion tag points : " << endl;
    for (unsigned i=0; i < _convertPoints.size(); i++){
      cerr << " {" <<_convertPoints[i].countVal << ", " << _convertPoints[i].physicalVal << "}";
      icount++;
      if (icount == 5){
	cerr << endl;
	icount = 0;
      }
    }
    cerr << endl;
  }

  //
  // Go looking for the time, which is actually the annotation string.
  //

  ip=ip+3;
  sp = 0;

  while (ip < fileSize) {

    if (!(iscntrl((int)_byteData[ip]))){
      bufString[sp] = char(_byteData[ip]);
      sp++;
      bufString[sp] = char(0);
    } else {
      break;
    }
    ip++;
    if ((sp > 1000) || (_byteData[ip] ==5)) break;
  }

  if (_params->debug){
    cerr << "Annotation string used for time : " << bufString << endl;
  }

  //
  // Set the _haveNorth, _haveSouth booleans. In this annotation
  // string, DK01 == full earth scan, DK02 == Northern hemisphere,
  // DK03 == Southern. These booleans were both initialzed to FALSE.
  //

  if (NULL != strstr(bufString, "DK01")) {
    _haveNorth = true;
    _haveSouth = true;
  }

  if (NULL != strstr(bufString, "DK02"))  _haveNorth = true;
  if (NULL != strstr(bufString, "DK03"))  _haveSouth = true;

  if (_params->debug){

    if (_haveNorth)
      cerr << "Northern hemisphere : Present" << endl;
    else
      cerr << "Northern hemisphere : Absent" << endl;

    if (_haveSouth)
      cerr << "Southern hemisphere : Present" << endl;
    else
      cerr << "Southern hemisphere : Absent" << endl;

  }

  date_time_t dTime;
  dTime.sec = 0;
  if (5 != sscanf(bufString + strlen("IMG_DK01IR1_"), "%4d%2d%2d%2d%2d",
				     &dTime.year, &dTime.month, &dTime.day,
				     &dTime.hour, &dTime.min)){
    cerr << "Failed to parse time from : " << bufString << endl;
    return;
  }

  uconvert_to_utime( &dTime );
  _fieldTime = dTime.unix_time;

  if (_params->debug) cerr << "Data time is " << utimstr( _fieldTime ) << endl;

  //
  // Fill out the _physicalArray with linear interpolation.
  //
  for (int i=0; i < 1024; i++){
    _physicalArray[i] = badVal;
  }
  
  for (unsigned i=0; i < _convertPoints.size(); i++){
    int index = _convertPoints[i].countVal;
    _physicalArray[index] = _convertPoints[i].physicalVal;
  }

  //
  // Find the first non-bad point in the array.
  //
  int j=0;
  do {
    if (_physicalArray[j] != badVal) break;
    j++;
  } while(1);

  //
  // Take that first value and fill back to the start.
  //
  for (int l=0; l < j; l++){
    _physicalArray[l] = _physicalArray[j];
  }

 //
  // Find the last non-bad point in the array.
  //
  j=1023;
  do {
    if (_physicalArray[j] != badVal) break;
    j--;
  } while(1);

  //
  // Take that last value and fill up to the end.
  //
  for (int k=1023; k > j; k--){
    _physicalArray[k] = _physicalArray[j];
  }

  //
  // Fill in in between.
  //
  for (j=1; j < 1023; j++){

    if (badVal == _physicalArray[j]){
      int e = j + 1;
      do {
	if (badVal != _physicalArray[e]) break;
	e++;
      } while(1);
      int startIndex = j - 1;
      int endIndex = e;

      for (int l= startIndex + 1; l < endIndex; l++){
	double t = double(l  - startIndex) / double(endIndex - startIndex);
	_physicalArray[l] = t * _physicalArray[endIndex] + (1.0 - t) * _physicalArray[startIndex];
      }
      j = endIndex;
    }

  }

  /* -- too verbose
  if (_params->debug){
    cerr << "Interpolated mapping array : " << endl;
    for (int i=0; i < 1024; i++){
      cerr << "Count value of " << i << " maps to " << _physicalArray[i] << endl;
    }
  }
  */

  _ok = true;

  return;
}


// See if constructor went OK.
bool fieldDataMgr::isOk(){
  return _ok;
}

// Check coverage.
bool fieldDataMgr::northCovered(){
  return _haveNorth;
}

bool fieldDataMgr::southCovered(){
  return _haveSouth;
}


// Get the data time.
time_t fieldDataMgr::getTime(){
  if (!_thisFieldRequested) return 0L;
  return _fieldTime;
}

// Load data.
void fieldDataMgr::loadData(double minLat, double minLon, 
			    double maxLat, double maxLon, 
			    double delLat, double delLon){

  if (!_thisFieldRequested) return;

  //
  // Let go of anything we may have had before from a previous load.
  //
  if (_fieldData != NULL) free (_fieldData);

  //
  // If we are not OK, forget about it.
  //
  if (!(_ok)) return;

  //
  // Make sure the longitudes are well behaved.
  // Putting the break point between 0 and 360 is OK for MTSAT, which scans the pacific.
  //
  if (minLon < 0.0) minLon += 360.0;
  if (maxLon < 0.0) maxLon += 360.0;

  if (_params->debug){
    cerr << "Loading data from " << minLat << ", " << minLon << " to ";
    cerr << maxLat << ", " << maxLon << " by " << delLat << ", " << delLon << endl;
  }

  //
  // Calculate the extent.
  //
  _fieldNx = (int)rint((maxLon - minLon) / delLon);
  _fieldNy = (int)rint((maxLat - minLat) / delLat);

  if (_params->debug){
    cerr << "Array will be " << _fieldNx << " by " <<  _fieldNy << endl;
  }

  _fieldData = (fl32 *)malloc(_fieldNx * _fieldNy * sizeof(fl32));
  if (_fieldData == NULL){
    cerr << "Malloc failure!" << endl;
    exit(-1);
  }

  //
  // If this is a visible field that we have to fill in, do so.
  //
  if ((_isVis) && (_params->fillMissingVis) && (_pixelData == NULL)){
    if (_params->debug){
      cerr << "Manufacturing all-missing visible field." << endl;
    }
    for (int i=0; i < _fieldNx * _fieldNy; i++){
      _fieldData[i] = badVal;
    }
    return;
  }


  //
  // Set some things up.
  //
  double twoToMinus16 = pow(2.0, -16.0);
  double xfac = twoToMinus16 * _cfac;
  double yfac = twoToMinus16 * _lfac;
 
  double pi = acos(-1.0);
  double toRad = pi / 180.0;
  double toDeg = 180.0 / pi;
  //
  // Loop through in lat/lon space.
  //
  for (int iy=0; iy < _fieldNy; iy++){

    double lat = minLat + iy*delLat;
    PMU_auto_register("Loading data buffer");

    for (int ix=0; ix < _fieldNx; ix++){

      double lon = minLon + ix * delLon;

      //
      // Go from lat/lon to x,y and then from x,y to
      // line number and count in our image data.
      //

      //
      // Go from lat, lon to x,y. This is from HRIT documentation.
      //
      double c_lat = atan(0.993243 * tan(lat * toRad));
      double cosClat = cos(c_lat);
  
      double Rl = 6356.5838 / sqrt( 1.0 - 0.00675701 * cosClat*cosClat );

      double delLonAngle = (lon - _subLon) * toRad;

      double R1 = 42164.0 - Rl * cosClat * cos( delLonAngle );
      double R2 = -Rl * cosClat * sin( delLonAngle );
      double R3 = Rl * sin(c_lat);

      double Rn = sqrt(R1*R1 + R2*R2 + R3*R3);

      //
      // Get angles x and y - these are in degrees
      //
      double x = toDeg * atan(-R2/R1);
      double y = toDeg * asin(-R3/Rn);

      //
      // Then go from x,y to line,col
      //
      int colNum = (int)rint(_coff + x * xfac);
      int lineNum= (int)rint(_loff + y * yfac);

      if ((colNum < 0) || (lineNum < 0) ||
	  (colNum > _nx-1) || (lineNum > _ny-1)
	  ){
	//
	// The navigation has gone wrong - off the face of the earth?
	//
	_fieldData[ix + iy * _fieldNx] = badVal;
	continue;
      }

      if (_pixelData[colNum + lineNum * _nx] == 65535){
	//
	// This is the missing value.
	//
	_fieldData[ix + iy * _fieldNx] = badVal;
	continue;
      }

      if ((_params->calOption == Params::CALIBRATE_LUT) && (_pixelData[colNum + lineNum * _nx] > 1023)){
	//
	// We were told to use a LUT, but our LUT only goes 0..1023. Mark as bad.
	//
	_fieldData[ix + iy * _fieldNx] = badVal;
	continue;
      }


      //
      // If we got here, then we have non-missing navigated data.
      //
      switch (_params->calOption){

      case Params::CALIBRATE_NONE :
	_fieldData[ix + iy * _fieldNx] = _pixelData[colNum + lineNum * _nx];
	break;

      case Params::CALIBRATE_LUT :
	_fieldData[ix + iy * _fieldNx] = _physicalArray[_pixelData[colNum + lineNum * _nx]];
	break;

      case Params::CALIBRATE_SCALE_OFFSET :
	if (_isVis){
	  _fieldData[ix + iy * _fieldNx] = _params->scaleBias.visScale * double(_pixelData[colNum + lineNum * _nx]) +
	    _params->scaleBias.visBias;
	} else {
	  _fieldData[ix + iy * _fieldNx] = _params->scaleBias.irScale * double(_pixelData[colNum + lineNum * _nx]) +
	    _params->scaleBias.irBias;
	}
	break;

      default :
	cerr << "Cannot recognise calibration option " << _params->calOption <<  " - edit param file" << endl;
	exit(-1); // Unlikely.
	break;

      }
    }
  }

  return;
}

// Get data loaded with call to loadData()
fl32 *fieldDataMgr::getData(){
  return _fieldData;
}

// Get some specifics about the data.
int fieldDataMgr::getNx(){
  return _fieldNx;
}


int fieldDataMgr::getNy(){
  return _fieldNy;
}


// Destructor.
fieldDataMgr::~fieldDataMgr (){

  if (!_thisFieldRequested) return;

  if (_byteData != NULL) free(_byteData);
  if (_fieldData != NULL) free (_fieldData);
  return;
}


