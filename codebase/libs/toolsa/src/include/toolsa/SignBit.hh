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
////////////////////////////////////////////////////////////////////
// SignBit.hh
//
// Mike Dixon, RAL, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// May 2008
//
////////////////////////////////////////////////////////////////////
//
// Testing sign bit of a number
//
////////////////////////////////////////////////////////////////////

#ifndef SIGN_BIT_HH
#define SIGN_BIT_HH

#include <cstring>

#ifdef DEBUG_PRINT
#include <iostream>
#endif

class SignBit
{

public:

  ///////////////////
  // isSet()
  //
  // Test for sign bit of any variable type.
  // Sign bit is assumed to be most significant bit.
  //
  // Returns true if sogn bit is set, false otherwise
  
  template <class T> static bool isSet(T xx)
  {
    
    // determine if we are on a big or small endian machine
    
    bool isBigEndian = true;
    union {
      unsigned short d;
      unsigned char bytes[2];
    } short_int;
    short_int.d = 1;
    if (short_int.bytes[1] != 0) {
      isBigEndian = true;
    } else {
      isBigEndian = false;
    }
    
    // copy the most significant byte to int08
    
    unsigned char int08;
    if (isBigEndian) {
      memcpy(&int08, (unsigned char *) &xx, 1);
    } else {
      memcpy(&int08, (unsigned char *) &xx + sizeof(xx) - 1, 1);
    }

#ifdef DEBUG_PRINT
    cerr << "==>> int08: ";
    cerr << (int) int08 << " ";
    int jj = 1;
    for (int ii = 7; ii >= 0; ii--) {
      int kk = jj << ii;
      if (kk & int08) {
        cerr << "1";
      } else {
        cerr << "0";
      }
    }
    cerr << endl;
#endif
    
    // test for most significant bit of this byte

    unsigned char sign08 = 128;
    if (int08 & sign08) {
      return true;
    } else {
      return false;
    }
    
  }

};

#endif
