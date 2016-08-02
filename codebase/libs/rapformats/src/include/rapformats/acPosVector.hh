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
// acPosVector.hh
//
// C++ wrapper for two points of aircraft position.
//
// Units for altitude are flight level.
//
// Niles Oien May 2004 (borrowing a lot from Mike Dixon's GentPt class)
//

#ifndef _acPosVector_hh
#define _acPosVector_hh


#include <string>
#include <iostream>
#include <cstdio>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>

using namespace std;

class acPosVector {

public:

  //
  // Some useful constants.
  //
  static const int acPosVector_numSpares = 5;
  static const double missingVal;
  //
  //
  // struct for header data. This contains most everything,
  // except the callsign string.
  //

  typedef struct {
    //
    // First point - previous.
    //
    ti32 timePrevious;
    fl32 latPrevious;
    fl32 lonPrevious;
    fl32 altPrevious;
    //
    // Second point - current.
    //
    ti32 timeCurrent;
    fl32 latCurrent;
    fl32 lonCurrent;
    fl32 altCurrent;
    //
    // Length of the callsign string. Includes trailing NULL.
    //
    si32 callsign_len;
    //
    // Spares.
    //
    fl32 spare[acPosVector_numSpares];
    //
    //
  } header_t;



  //
  // Public methods.
  //

  // Constructor - no arguments.
  //
  acPosVector();
  //
  // Constructor - previous point only.
  //
  acPosVector(ti32 timePrevious, fl32 latPrevious, fl32 lonPrevious,
	      fl32 altPrevious, string callsign);
  //
  // Constructor - all data members.
  //
  acPosVector(ti32 timePrevious, fl32 latPrevious, 
	      fl32 lonPrevious, fl32 altPrevious, 
	      ti32 timeCurrent, fl32 latCurrent, 
	      fl32 lonCurrent, fl32 altCurrent, 
	      string callsign);
  //
  // Destructor.
  //
  ~acPosVector();
  //
  // Method to add the next point. If timeCurrent,
  // latCurrent, lonCurrent, and altCurrent are all
  // missing then they are set to the values that are passed
  // in.
  //
  // If they are non-missing then the Previous values are
  // set to the Current values and the Current values are set to
  // what is passed in.
  //
  void updatePoint(ti32 time, fl32 lat, 
		   fl32 lon, fl32 alt);
  //
  // Age data out of the point. Previous data are set to
  // the current data, current data are set to missing.
  //
  void agePoint();
  //
  // isComplete() returns true if all data are non-missing.
  //
  bool isComplete();
  //
  //
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the object values.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure

  int disassemble(const void *buf, int len);


  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  //
  // returns 0 on success, -1 on failure
  // Use getErrStr() on failure.

  int assemble();

  // get the assembled buffer pointer

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ////////////////
  // error string
  
  const string &getErrStr() const { return (_errStr); }

  /////////////////////////
  // print

  void print(FILE *out);
  void print(ostream &out);

  //////////////////////////////////////////////////////////
  // set methods.

  void clear();

  void setTimePrevious( time_t timePrev ) { _hdr.timePrevious = timePrev; }
  void setLatPrevious( double latPrev ) { _hdr.latPrevious = latPrev; }
  void setLonPrevious( double lonPrev ) { _hdr.lonPrevious = lonPrev; }
  void setAltPrevious( double altPrev ) { _hdr.altPrevious = altPrev; }
  //
  void setTimeCurrent( time_t timeCurrent ) { _hdr.timeCurrent = timeCurrent; }
  void setLatCurrent( double latCurrent ) { _hdr.latCurrent = latCurrent; }
  void setLonCurrent( double lonCurrent ) { _hdr.lonCurrent = lonCurrent; }
  void setAltCurrent( double altCurrent ) { _hdr.altCurrent = altCurrent; }
  //
  void setCallsign(const string &callsign) { 
    _callsign = callsign; 
    _hdr.callsign_len = _callsign.size() + 1; 
  }
  //
  void setSpare(int index, double val); 

  //////////////////////////////////////////////////////////
  // get methods.

  time_t getTimePrevious() const { return _hdr.timePrevious; }
  double getLatPrevious() const { return _hdr.latPrevious; }
  double getLonPrevious() const { return _hdr.lonPrevious; }
  double getAltPrevious() const { return _hdr.altPrevious; }
  //
  time_t getTimeCurrent() const { return _hdr.timeCurrent; }
  double getLatCurrent() const { return _hdr.latCurrent; }
  double getLonCurrent() const { return _hdr.lonCurrent; }
  double getAltCurrent() const { return _hdr.altCurrent; }
  const string &getCallsign() const { return (_callsign); }
  //
  double getSpare(int index) const{
    if (index < acPosVector::acPosVector_numSpares){
      return  _hdr.spare[index];
    } else {
      return acPosVector::missingVal;
    }
  }

protected:

  MemBuf _memBuf;
  mutable string _errStr;


private:

  //
  // The data member.
  //
  header_t _hdr;
  //
  // The actual callsign.
  //
  string _callsign;

  void _BE_from_header(void *buf);
  void _BE_to_header(void *buf);
  void _checkMissing(double val, char *representation);
  void _swapTimesIfRequired();

};


#endif
