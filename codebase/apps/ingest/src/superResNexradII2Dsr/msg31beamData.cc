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
// msg31beamData.cc
//
// Deals with message 31 beam data. You pass in params,
// compression type flag, uncompressed data length
// and the buffer that has the original beam data in it.
// The class then puts together the array of data values.
// A method allows resampling onto another spacing.
// 
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
/////////////////////////////////////////////////////////////

#include <cmath>
#include <bzlib.h>

#include "msg31beamData.hh"

using namespace std;

const float msg31beamData::badVal = -9999.0;

// Constructor, from hostname and port, socket IO assumed.
msg31beamData::msg31beamData ( Params *TDRP_params, int compressionFlags, 
			       int compressedLen, unsigned char *buffer ){

  _params = TDRP_params;
  //
  // Start off pessimistic
  //
  _ok = false;
  _data = NULL;
  _fieldName = "UNKNOWN";
  _fieldNum = -1;
  _wanted = false;

  // Init these to the "I have not decoded them yet" value.
  _lat = _lon = _alt = _nyquistVel = -999.0;

  if (buffer == NULL) return; // No data for this field in this case.

  //
  // Work out the field name, number from the first few bytes in the header.
  // Somewhat laborious process, but worth it. The first three are fields we are not
  // interested in so leave the field number at -1 for them. After that the field
  // number has to be set depending on the field we have and what fields we want.
  //
  bool haveFieldName = false;
  if (0 == strncmp((const char *) buffer, "RVOL", 4)){
    haveFieldName = true; _fieldName = "VOL";

    // Lat, lon, alt

    fl32 *lat = (fl32 *) &buffer[8];
    _bSwapLong((ui32 *) lat);

    fl32 *lon = (fl32 *) &buffer[12];
    _bSwapLong((ui32 *) lon);

    si16 *siteHeight = (si16 *) &buffer[16];
    _bSwap(siteHeight);

    ui16 *towerHeight = (ui16 *) &buffer[18];
    _bSwap(towerHeight);

    _vcp = 256*buffer[40]+buffer[41];

    // cerr << "Lat " << *lat << " lon " << *lon << " site " << *siteHeight << " tower " << *towerHeight << " VCP : " << _vcp << endl;

    _lat = *lat; _lon = *lon; _alt = *siteHeight + *towerHeight;

  }

  if (0 == strncmp((const char *) buffer, "RELV", 4)){
    haveFieldName = true; _fieldName = "ELV";
  }

  if (0 == strncmp((const char *) buffer, "RRAD", 4)){
    haveFieldName = true; _fieldName = "RAD";

    //
    // decode nyquist velocity, JamesD needs this.
    //
    ui16 *nyVel = (ui16 *) &buffer[16];
    _bSwap(nyVel);

    //    cerr << "NYQUIST : " << *nyVel << endl;

    _nyquistVel = *nyVel / 100.0;

  }

  int potentialFieldNum = 0;

  char fname[256];
  memcpy(fname, buffer, 4);
  fname[4] = '\0';

  if (0 == strncmp(fname, "DREF", 4)){
    haveFieldName = true; _fieldName = "DBZ";  if (_params->fieldList.wantDBZ) _fieldNum = potentialFieldNum;
  }
  if (_params->fieldList.wantDBZ) potentialFieldNum++;

  if (0 == strncmp(fname, "DVEL", 4)){
    haveFieldName = true; _fieldName = "VEL"; if (_params->fieldList.wantVEL)  _fieldNum = potentialFieldNum;
  }
  if (_params->fieldList.wantVEL) potentialFieldNum++;

  if (0 == strncmp(fname, "DSW", 3)){
    haveFieldName = true; _fieldName = "SW";  if (_params->fieldList.wantSW) _fieldNum = potentialFieldNum;
  }
  if (_params->fieldList.wantSW) potentialFieldNum++;

  if (0 == strncmp(fname, "DZDR", 4)){
    haveFieldName = true; _fieldName = "ZDR";  if (_params->fieldList.wantZDR) _fieldNum = potentialFieldNum;
  }
  if (_params->fieldList.wantZDR) potentialFieldNum++;

  if (0 == strncmp(fname, "DPHI", 4)){
    haveFieldName = true; _fieldName = "PHI"; if (_params->fieldList.wantPHI) _fieldNum = potentialFieldNum;
  }
  if (_params->fieldList.wantPHI) potentialFieldNum++;

  if (0 == strncmp(fname, "DRHO", 4)){
    haveFieldName = true; _fieldName = "RHO"; if (_params->fieldList.wantRHO) _fieldNum = potentialFieldNum;
  }

  //
  // If we were unable to get a valid field name, this is a serious error.
  //
  if (!(haveFieldName)){
    if (_params->debug >= Params::DEBUG_NORM){
      cerr << "WARNING : unexpected field name: " << fname << endl;
    }
    _wanted = false;
    return;
  }

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << "Field name is " << _fieldName << endl;
  }

  if (_fieldNum != -1) _wanted = true;
  if (!(_wanted)) return; // Pointless doing rest of decoding.


  //
  // Get the numeric metadata
  //
  si16 *nGates = (si16 *) &buffer[8];
  _bSwap( nGates );

  si16 *rangeFirstGate = (si16 *)&buffer[10];
  _bSwap( rangeFirstGate );

  si16 *gateSpacing = (si16 *) &buffer[12];
  _bSwap (gateSpacing );

  int bitSize = (int)buffer[19];

  fl32 *scale = (fl32 *) &buffer[20];
  _bSwapLong((ui32 *) scale);

  fl32 *bias = (fl32 *) &buffer[24];
  _bSwapLong((ui32 *) bias);

  _gateSpacing = double(*gateSpacing)/1000.0;
  _rangeFirstGate = double(*rangeFirstGate)/1000.0;
  _nGates =  *nGates;

  //
  // Uncompress the data, if necessary. 0=> no compression,
  // 1 => bzip2 compression, 2 => zlib compression
  // Length includes 28 byte header - it is not entirely clear
  // to me if the header is compressed or not. It seems like
  // it is not.
  //
  ui08 *uncompressedData=NULL;

  if (compressionFlags == 0){
    uncompressedData= &buffer[28];
  } else {
    int nBytes = _nGates;
    if (bitSize == 16) nBytes = 2*nBytes;
    uncompressedData = (ui08 *) malloc(nBytes);
    if (uncompressedData == NULL){
      cerr << "Malloc failed!" << endl;
      exit(-1);
    }

    int bzError;
     unsigned int olength;

    switch (compressionFlags) {

    case 1 :
      // bzip2 compression. I'm going to put some code in here, because
      // I've had experience with the bunzip stuff. But as of Feb 2008 this is untested.
      // We have not had any compressed data yet. Niles Oien.
#ifdef BZ_CONFIG_ERROR
      bzError = BZ2_bzBuffToBuffDecompress((char *) uncompressedData, &olength,
					   (char *) &buffer[28], compressedLen, 0, 0);
#else
      bzError = bzBuffToBuffDecompress((char *) uncompressedData, &olength,
				       (char *) &buffer[28], compressedLen, 0, 0);
#endif

      if (bzError){
	cerr << "bunzip attempt returned " << bzError << endl;
	return;
      }

      break;

    case 2 :
      // zlib compression. I'm going to wait re this, mostly
      // because I'd like an example of it before I perpetrate the code.
      // It's also not clear to me that it will ever be implemented. Niles Oien.
      cerr << "zlib decompression not yet implemented" << endl;
      return;
      break;

    default :
      cerr << "Unrecognized compression flag " << compressionFlags << endl;
      return;
      break;

    }
  }

  //
  // Should be ready to get data.
  //
  _data = (fl32 *)malloc(sizeof(fl32) * _nGates);
  if (_data == NULL) return;

  if ((bitSize != 8) && (bitSize != 16)){
    cerr << "Unsupported bit width " << bitSize << endl;
    return;
  }

  bool first = true;
  double min=0.0, max=0.0;
  double total = 0.0;
  int numGood = 0;

  if (bitSize == 8){
    for (int igate=0; igate < _nGates; igate++){
      unsigned char dataByte = uncompressedData[igate];

      _data[igate] = badVal;
      if (dataByte > 1){ 
	numGood++;
	_data[igate] = (double(dataByte) - *bias) / *scale;
	total += _data[igate];
	if (first){
	  first = false;
	  min = max = _data[igate];
	} else {
	  if (_data[igate] < min) min = _data[igate];
	  if (_data[igate] > max) max = _data[igate];
	}
      }
    }
  }

  if (bitSize == 16){
    for (int igate=0; igate < _nGates; igate++){
      ui16 *dataWord = (ui16 *) &uncompressedData[2*igate];
      _bSwap(dataWord);
      
      _data[igate] = badVal;
      if (*dataWord > 1){ 
	numGood++;
	_data[igate] = (double(*dataWord) - *bias) / *scale;
	total += _data[igate];
	if (first){
	  first = false;
	  min = max = _data[igate];
	} else {
	  if (_data[igate] < min) min = _data[igate];
	  if (_data[igate] > max) max = _data[igate];
	}
      }
    }
  }

  double mean = -999.0;
  if (numGood > 0) mean = total/double(numGood);

  if (compressionFlags) free(uncompressedData);

  if (_params->debug >= Params::DEBUG_NORM){
    if (first){
      cerr << "Field " << _fieldName << " number " << _fieldNum << " is all missing ";
    } else {
      cerr << "Field " << _fieldName << " number " << _fieldNum << " goes from " << min << " to " << max << " mean " << mean;
    }
    cerr << " " << _nGates << " gates, spacing " << _gateSpacing << " Km, first gate at " << _rangeFirstGate << endl;
  }
  
  if (_params->debug >= Params::DEBUG_DATA){
    for (int igate=0; igate < _nGates; igate++){
      double range = _rangeFirstGate + igate * _gateSpacing;
      cerr << "{" << range << " km, " << _data[igate] << "} ";
      if ((igate > 0) && (igate % 5 == 0)) cerr << endl;
    }
    cerr << endl << endl;
  }

  _ok = true;
  
  return;
}

// Get status.
bool msg31beamData::isOk(){
  return _ok;
}

// Get number of gates.
int msg31beamData::getNgates(){
  return _nGates;
}

// Get data at ith gate (first gate is 0)
fl32 msg31beamData::getGate(int iGate ){

  if ((!(_ok)) || (iGate < 0) || (iGate > _nGates-1)) return badVal;
  return _data[iGate];

}

// Get range to first gate
double msg31beamData::getRangeFirstGate(){
  return _rangeFirstGate;
}

// Get gate spacing
double msg31beamData::getGateSpacing(){
  return _gateSpacing;
}

// Get field name
string msg31beamData::getFieldName(){
  return _fieldName;
}

// Get field number
int msg31beamData::getFieldNum(){
  return _fieldNum;
}

// See if this field is wanted
bool msg31beamData::isWanted(){
  return _wanted;
}


// Resample data onto new spacing.
void msg31beamData::resampleData( double firstGateRangeKm,
				  double lastGateRangeKm,
				  double spacingKm){

  //
  // If something is wrong, return.
  //
  if (!(_ok)) return;

  //
  // Get the new number of gates, check if we have to do anything,
  // if we do, allocate space for the new data.
  //
  int newNumGates = (int)rint((lastGateRangeKm - firstGateRangeKm)/spacingKm);

  if ((newNumGates == _nGates) && 
      _equal(_gateSpacing, spacingKm) &&
      _equal(_rangeFirstGate, firstGateRangeKm)){
    if (_params->debug >= Params::DEBUG_NORM){
      cerr << "Resampling is a no-op for these data." << endl;
    }
    return;
  }

  if (_params->debug >= Params::DEBUG_NORM){
    cerr << "Resampling " << _fieldName << " data..." << endl;
  }

  fl32 *newData = (fl32 *)malloc(newNumGates * sizeof(fl32));
  if (newData == NULL){
    cerr << "Resampling malloc failed!" << endl;
    exit(-1);
  }

  //
  // Loop through, getting the range distance and using it to
  // fill the new data from the old data.
  //
  for (int ing=0; ing < newNumGates; ing++){
    double dist = firstGateRangeKm + ing * spacingKm;
    int oldIndex = (int)rint((dist - _rangeFirstGate) / _gateSpacing);
    if ((oldIndex < 0) || (oldIndex > _nGates -1))
      newData[ing] = badVal;
    else
      newData[ing] = _data[oldIndex];
  }

  //
  // Set the internal variables that need resetting.
  //
  _gateSpacing = spacingKm;
  _rangeFirstGate = firstGateRangeKm;
  _nGates = newNumGates;
  
  //
  // Let go of the old data, copy the new data into place, get
  // rid of the temporary workspace.
  //
  free( _data );

  _data = (fl32 *)malloc(newNumGates * sizeof(fl32));
  if (_data == NULL){
    cerr << "resampling malloc failed!" << endl;
    exit(-1);
  }

  for (int ig=0; ig < newNumGates; ig++)
    _data[ig] = newData[ig];

  free (newData);

  return;
}

// Destructor.
msg31beamData::~msg31beamData (){
  if (_data != NULL) free(_data);
  return;
}


//
// Small routines to do byte swapping.
//
void msg31beamData::_bSwap(void *i){

  if (!(_params->byteSwap)) return;

  unsigned char *bPtr = (unsigned char *) i;
  unsigned char tmp;
  tmp = *bPtr;
  *bPtr = *(bPtr+1);
  *(bPtr+1) = tmp;
  return;
}


void msg31beamData::_bSwapLong(void *i){

  if (!(_params->byteSwap)) return;

  unsigned char *bPtr = (unsigned char *) i;
  unsigned char b0 = *bPtr;
  unsigned char b1 = *(bPtr+1);
  unsigned char b2 = *(bPtr+2);
  unsigned char b3 = *(bPtr+3);

  *bPtr = b3;
  *(bPtr+1) = b2;
  *(bPtr+2) = b1;
  *(bPtr+3) = b0;
 
  return;
}

bool msg31beamData::_equal(double a, double b){
  if (fabs(a-b) < 0.01) return true;
  return false;
}

// Access to internal members. Set to 0 initially and then
// overwritten.
double msg31beamData::getLat(){ return _lat; }
double msg31beamData::getLon(){ return _lon; }
double msg31beamData::getAlt(){ return _alt; }
double msg31beamData::getNyquistVel(){ return _nyquistVel; }
int msg31beamData::getVCP(){ return _vcp; }
