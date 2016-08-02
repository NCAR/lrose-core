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
// acPosVector.cc
//
// C++ wrapper for two points of aircraft position.
//
// Units for altitude are flight level.
//
// Niles Oien May 2004 (borrowing a lot from Mike Dixon's GentPt class)
//

#include <rapformats/acPosVector.hh>
#include <dataport/bigend.h>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>
using namespace std;

const double acPosVector::missingVal = -9999.0;

//
// Constructor - no arguments.
//
acPosVector::acPosVector(){
  //
  // Load up header.
  //
  _hdr.timePrevious = 0L;
  _hdr.latPrevious = acPosVector::missingVal;
  _hdr.lonPrevious = acPosVector::missingVal;
  _hdr.altPrevious = acPosVector::missingVal;
  //
  _hdr.timeCurrent = 0L;
  _hdr.latCurrent = acPosVector::missingVal;
  _hdr.lonCurrent = acPosVector::missingVal;
  _hdr.altCurrent = acPosVector::missingVal;
  //
  // Fill in the callsign.
  //
  _callsign = "";
  //
  _hdr.callsign_len = _callsign.size() + 1;
  //
  // Set error string to nothing.
  //
  _errStr = "";
  //
  // Set the spares to the bad value.
  //
  for (int i=0; i < acPosVector::acPosVector_numSpares; i++){
    _hdr.spare[i] =  acPosVector::missingVal;
  }
}
//
// Constructor - previous point only.
//
acPosVector::acPosVector(ti32 timePrevious, fl32 latPrevious, fl32 lonPrevious,
			 fl32 altPrevious, string callsign){


  //
  // Load up header.
  //
  _hdr.timePrevious = timePrevious;
  _hdr.latPrevious = latPrevious;
  _hdr.lonPrevious = lonPrevious;
  _hdr.altPrevious = altPrevious;
  //
  _hdr.timeCurrent = 0L;
  _hdr.latCurrent = acPosVector::missingVal;
  _hdr.lonCurrent = acPosVector::missingVal;
  _hdr.altCurrent = acPosVector::missingVal;
  //
  // Fill in the callsign.
  //
  _callsign = callsign;
  //
  _hdr.callsign_len = _callsign.size() + 1;
  //
  // Set error string to nothing.
  //
  _errStr = "";
  //
  // Set the spares to the bad value.
  //
  for (int i=0; i < acPosVector::acPosVector_numSpares; i++){
    _hdr.spare[i] =  acPosVector::missingVal;
  }
}
//
// Constructor - all data members.
//
acPosVector::acPosVector(ti32 timePrevious, fl32 latPrevious, 
			 fl32 lonPrevious, fl32 altPrevious, 
			 ti32 timeCurrent, fl32 latCurrent, 
			 fl32 lonCurrent, fl32 altCurrent, 
			 string callsign){
  //
  // Load up header.
  //
  _hdr.timePrevious = timePrevious;
  _hdr.latPrevious = latPrevious;
  _hdr.lonPrevious = lonPrevious;
  _hdr.altPrevious = altPrevious;
  //
  _hdr.timeCurrent = timeCurrent;
  _hdr.latCurrent = latCurrent;
  _hdr.lonCurrent = lonCurrent;
  _hdr.altCurrent = altCurrent;
  //
  // Fill in the callsign.
  //
  _callsign = callsign;
  //
  _hdr.callsign_len = _callsign.size() + 1;
  //
  // Set error string to nothing.
  //
  _errStr = "";
  //
  // Set the spares to the bad value.
  //
  for (int i=0; i < acPosVector::acPosVector_numSpares; i++){
    _hdr.spare[i] =  acPosVector::missingVal;
  }
  _swapTimesIfRequired();
}
//
// Destructor. Does nothing.
//
acPosVector::~acPosVector(){
}

/////////////////////////////////////////////////////
// Set the spare values.
//
void acPosVector::setSpare(int index, double val){
  if (index < acPosVector::acPosVector_numSpares){
    _hdr.spare[index] = val;
  }
}

/////////////////////////////////////////////////////
// Update the point with new values.
//
void acPosVector::updatePoint(ti32 time, fl32 lat, 
			      fl32 lon, fl32 alt){
  //
  // If our current values are misssing, accept these ones.
  //
  if (
      (_hdr.timeCurrent == 0L) &&
      (_hdr.latCurrent == acPosVector::missingVal) &&
      (_hdr.lonCurrent == acPosVector::missingVal) &&
      (_hdr.altCurrent == acPosVector::missingVal)
      ){
    _hdr.timeCurrent = time;
    _hdr.latCurrent = lat;
    _hdr.lonCurrent = lon;
    _hdr.altCurrent = alt;
    _swapTimesIfRequired();
    return;
  }
  //
  // Otherwise, age off what we have to accept these.
  //
  _hdr.timePrevious = _hdr.timeCurrent;
  _hdr.latPrevious = _hdr.latCurrent;
  _hdr.lonPrevious = _hdr.lonCurrent;
  _hdr.altPrevious = _hdr.altCurrent;
  //
  _hdr.timeCurrent = time;
  _hdr.latCurrent = lat;
  _hdr.lonCurrent = lon;
  _hdr.altCurrent = alt;
  _swapTimesIfRequired();
  return;

}

////////////////////////////////////////////////////////
// age data out of the point.
//
void acPosVector::agePoint(){
  //
  _hdr.timePrevious = _hdr.timeCurrent;
  _hdr.latPrevious = _hdr.latCurrent;
  _hdr.lonPrevious = _hdr.lonCurrent;
  _hdr.altPrevious = _hdr.altCurrent;
  //
  _hdr.timeCurrent = 0L;
  _hdr.latCurrent = acPosVector::missingVal;
  _hdr.lonCurrent = acPosVector::missingVal;
  _hdr.altCurrent = acPosVector::missingVal;

  return;
}

///////////////////////////////////////////////////////////
//
// isComplete() returns true if all data are non-missing.
//
bool acPosVector::isComplete(){

  if (
      (_hdr.timeCurrent == 0L) ||
      (_hdr.latCurrent == acPosVector::missingVal) ||
      (_hdr.lonCurrent == acPosVector::missingVal) ||
      (_hdr.altCurrent == acPosVector::missingVal) ||
      (_hdr.timePrevious == 0L) ||
      (_hdr.latPrevious == acPosVector::missingVal) ||
      (_hdr.lonPrevious == acPosVector::missingVal) ||
      (_hdr.altPrevious == acPosVector::missingVal)
      ){
    return false;
  }

  return true;

}


///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the object values.
// Handles byte swapping.
// Returns 0 on success, -1 on failure

int acPosVector::disassemble(const void *buf, int len){

  _errStr = "ERROR - acPosVector::disassemble()\n";
  
  // check minimum len for header
  
  if (len < (int) sizeof(header_t)) {
    TaStr::AddInt(_errStr, "  Buffer too short for header, len: ", len);
    TaStr::AddInt(_errStr, "  Minimum valid len: ",
		  sizeof(header_t));
    return -1;
  }

 
  // local copy of buffer

  _memBuf.free();
  _memBuf.add(buf, len);
  
  // get header
  
  _BE_to_header(_memBuf.getPtr());

  // The header is now in _hdr
  // Get the callsign string.

  _callsign = "";
  char *callsignChars = (char *) _memBuf.getPtr() + sizeof(header_t);
  if (_hdr.callsign_len > 0){
    callsignChars[_hdr.callsign_len-1] = char(0);
    _callsign = callsignChars;
  }

  return 0;

}


///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.
//
// returns 0 on success, -1 on failure
// Use getErrStr() on failure.

int acPosVector::assemble(){
  //
  // Make sure the entry for the callsign length in the header is correct.
  //
  _hdr.callsign_len = _callsign.size() + 1;
  //
  _errStr += "ERROR - GenPt::assemble()\n";
  //
  // Declare a buffer to byte swap the header into.
  //
 header_t hdr;
 _BE_from_header(&hdr);
 //
 // Add it to the buffer.
 //
 _memBuf.free();
 _memBuf.add(&hdr, sizeof(header_t));
 //
 // Add the callsign.
 //
 _memBuf.add(_callsign.c_str(), _callsign.size() + 1);
 //
 return 0;
 //
}


/////////////////////////
// print - file
//
void acPosVector::print(FILE *out){

  fprintf(out,"Callsign : %s\n", _callsign.c_str() );

  fprintf(out,"Previous time : ");
  if (_hdr.timePrevious == 0L){
    fprintf(out,"Not set ");
  } else {
    fprintf(out,"%s ", utimstr(_hdr.timePrevious));
  }

  fprintf(out,"Current time : ");
  if (_hdr.timeCurrent == 0L){
    fprintf(out,"Not set\n");
  } else {
    fprintf(out,"%s\n", utimstr(_hdr.timeCurrent));
  }

  char rep1[64], rep2[64];

  fprintf(out, " Latitude : ");
  _checkMissing(_hdr.latPrevious, rep1);  _checkMissing(_hdr.latCurrent, rep2); 
  fprintf(out,"from %s to %s\n", rep1, rep2);

  fprintf(out, " Longitude : ");
  _checkMissing(_hdr.lonPrevious, rep1);  _checkMissing(_hdr.lonCurrent, rep2); 
  fprintf(out,"from %s to %s\n", rep1, rep2);

  fprintf(out, " Altitude : ");
  _checkMissing(_hdr.altPrevious, rep1);  _checkMissing(_hdr.altCurrent, rep2); 
  fprintf(out,"from %s to %s\n", rep1, rep2);

  for (int i=0; i < acPosVector::acPosVector_numSpares; i++){
    _checkMissing(_hdr.spare[i], rep1);
    fprintf(out," Spare[%d] : %s\n", i, rep1);
  }

  return;

}

/////////////////////////
// print - stream
//
void acPosVector::print(ostream &out){

  out << "Callsign : " << _callsign << endl;

  out << "Previous time : ";
  if (_hdr.timePrevious == 0L){
    out << "Not set ";
  } else {
    out << utimstr(_hdr.timePrevious) << " ";
  }
  
  out << "Current time : ";
  if (_hdr.timeCurrent == 0L){
    out << "Not set " << endl;
  } else {
    out << utimstr(_hdr.timeCurrent) << endl;
  }

  char rep1[64], rep2[64];
  
  out << " Latitude : ";
  _checkMissing(_hdr.latPrevious, rep1);  _checkMissing(_hdr.latCurrent, rep2); 
  out << "from " << rep1 << " to " << rep2 << endl;
  
  out << " Longitude : ";
  _checkMissing(_hdr.lonPrevious, rep1);  _checkMissing(_hdr.lonCurrent, rep2); 
  out << "from " << rep1 << " to " << rep2 << endl;


  out << " Altitude : ";
  _checkMissing(_hdr.altPrevious, rep1);  _checkMissing(_hdr.altCurrent, rep2); 
  out << "from " << rep1 << " to " << rep2 << endl;

  for (int i=0; i < acPosVector::acPosVector_numSpares; i++){
    _checkMissing(_hdr.spare[i], rep1);
    out <<" Spare[" << i << "] : " << rep1 << endl;
  }

  return;

}

void acPosVector::_BE_from_header(void *buf){
  //
  // Copy the header to the buffer.
  //
  memcpy(buf, &_hdr, sizeof(header_t));
  //
  // Byte swap the buffer.
  //
  BE_from_array_32(buf, sizeof(header_t));
  //
  return;
  //
}

void acPosVector::_BE_to_header(void *buf){
  //
  // Copy the buffer to the header.
  //
  memcpy(&_hdr, buf, sizeof(header_t));
  //
  // Byte swap.
  //
  BE_to_array_32(&_hdr, sizeof(header_t));
  //
  return;
  //
}

void acPosVector::_checkMissing(double val, char *representation){

  if (val == acPosVector::missingVal){
    sprintf(representation,"%s", "MISSING");
  } else {
    sprintf(representation,"%g", val);
  }
}
//
// Swap the entries of the times came in out of order.
//
void acPosVector::_swapTimesIfRequired(){

  if (_hdr.timePrevious > _hdr.timeCurrent){
    //
    time_t tempT;
    tempT = _hdr.timePrevious;    _hdr.timePrevious = _hdr.timeCurrent;
    _hdr.timeCurrent = tempT;
    //
    double temp;
    temp = _hdr.latPrevious;    _hdr.latPrevious = _hdr.latCurrent;
    _hdr.latCurrent = temp;
    //
    temp = _hdr.lonPrevious;    _hdr.lonPrevious = _hdr.lonCurrent;
    _hdr.lonCurrent = temp;
    //
    temp = _hdr.altPrevious;    _hdr.altPrevious = _hdr.altCurrent;
    _hdr.altCurrent = temp;
    //
  }
  return;
}

