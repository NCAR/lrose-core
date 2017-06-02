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
// ByteOrder.cc
//
// Dealing with data byte order
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2010
//
///////////////////////////////////////////////////////////////

#include <Ncxx/ByteOrder.hh>
#include <cstring>
#include <sys/types.h>
using namespace std;

bool ByteOrder::_set = false;
bool ByteOrder::_isBigEndian = false;

//////////////////////////////////////////////////////////////////////////
/// Function to determine whether the underlying machine
/// is big endian or not.
///
/// Returns true if big-endian, false if not.

bool ByteOrder::hostIsBigEndian() {
  if (_set) {
    return _isBigEndian;
  }
  union {
    u_int16_t d;
    unsigned char bytes[2];
  } short_int;
  short_int.d = 1;
  if (short_int.bytes[1] != 0) {
    _isBigEndian = true;
  } else {
    _isBigEndian = false;
  }
  _set = true;
  return _isBigEndian;
}
  
//////////////////////////////////////////////////////////////////////////
//  Perform in-place 64-bit word byte swap, if necessary, to produce
//  BE representation from machine representation, or vice-versa.
// 
//  If force is true, swapping is forced irrespective of host byte order.
// 
//  Array must be aligned.
// 
//  Returns the number of bytes converted.  Note that this number is 0
//  if no swapping is performed.

int ByteOrder::swap64(void *array, size_t nbytes,
                      bool force /* = false*/)
  
{


  // do we need to swap?

  if(hostIsBigEndian() && !force) {
    return 0;
  }

  int ndoubles = nbytes / 8;
  char *ptr = (char*) array;
  for (int i = 0; i < ndoubles; i++) {

    // Copy the 8 bytes to 2 ui32's - Reversing 1st & 2nd
    // PTR                 L1      L2
    // 1 2 3 4 5 6 7 8 ->  5 6 7 8 1 2 3 4

    u_int32_t l1,l2;
    memcpy((void*)&l2,(void*)ptr,4);
    memcpy((void*)&l1,(void*)(ptr+4),4);

    // Reverse the 4 bytes of each ui32
    // 5 6 7 8  -> 8 7 6 5
    l1 = (((l1 & 0xff000000) >> 24) |
	  ((l1 & 0x00ff0000) >> 8) |
	  ((l1 & 0x0000ff00) << 8) |
	  ((l1 & 0x000000ff) << 24));

    // 1 2 3 4 -> 4 3 2 1
    l2 = (((l2 & 0xff000000) >> 24) |
	  ((l2 & 0x00ff0000) >> 8) |
	  ((l2 & 0x0000ff00) << 8) |
	  ((l2 & 0x000000ff) << 24));


    // Copy the reversed value back into place
    memcpy(ptr, &l1, 4);
    memcpy(ptr + 4, &l2, 4);

    ptr+=8;  // Move to the next 8 byte value

  } // i

  return nbytes;
}

//////////////////////////////////////////////////////////////////////////
//  Performs an in-place 32-bit word byte swap, if necessary, to produce
//  BE representation from machine representation, or vice-versa.
// 
//  If force is true, swapping is forced irrespective of host byte order.
// 
//  Array must be aligned.
// 
//  Returns the number of bytes converted.  Note that this number is 0
//  if no swapping is performed.
 
int ByteOrder::swap32(void *array, size_t nbytes,
                      bool force /* = false*/)
  
{

  // do we need to swap?

  if(hostIsBigEndian() && !force) {
    return 0;
  }
  
  int nlongs = nbytes / sizeof(u_int32_t);
  u_int32_t *this_long = (u_int32_t *) array;
  
  for (int i = 0; i < nlongs; i++) {

    u_int32_t l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));
    
    this_long++;

  }

  return nbytes;

}

//////////////////////////////////////////////////////////////////////////
//  Performs an in-place 16-bit word byte swap, if necessary, to produce
//  BE representation from machine representation, or vice-versa.
// 
//  If force is true, swapping is forced irrespective of host byte order.
// 
//  Array must be aligned.
// 
//  Returns the number of bytes converted.  Note that this number is 0
//  if no swapping is performed.

int ByteOrder::swap16(void *array, size_t nbytes,
                      bool force /* = false */)

{


  // do we need to swap?

  if(hostIsBigEndian() && !force) {
    return 0;
  }

  int nlongs = nbytes / sizeof(u_int32_t);
  u_int32_t *this_long = (u_int32_t *) array;
  
  for (int i = 0; i < nlongs; i++) {
    
    u_int32_t l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 8) |
                  ((l & 0x00ff0000) << 8) |
                  ((l & 0x0000ff00) >> 8) |
                  ((l & 0x000000ff) << 8));

    this_long++;

  }
  
  if (nlongs * sizeof(u_int32_t) != nbytes) {
    int nshorts = nbytes / sizeof(u_int16_t);
    u_int16_t *array16 = (u_int16_t *) array;
    u_int16_t s = array16[nshorts-1];
    array16[nshorts-1]= (((s & 0xff00) >> 8) | ((s & 0x00ff) << 8));
  }

  return nbytes;
  
}

