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
//////////////////////////////////////////////////
// BMS - Bit Map Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
/////////////////////////////////////////////////

#include <cstring>
#include <grib/constants.h>
#include <grib/BMS.hh>
#include <iostream>

using namespace std;


const int BMS::NUM_HEADER_BYTES = 6;

BMS::BMS() :
  GribSection(),
  _numUnusedBits(0),
  _bitMapId(0),
  _nBitMapBytes(0),
  _bitMap(0)
{
  _nBytes = NUM_HEADER_BYTES;
}

BMS::~BMS() 
{
   if( _bitMap )
      delete[] _bitMap;
}

void
BMS::setBitmap(const ui08 *new_bitmap, const int bitmap_size)
{
  // Calculate the number of bytes needed for the bitmap.  Note
  // that this must be an even number of bytes.

  _nBitMapBytes = (bitmap_size + 7) / 8;
  if (_nBitMapBytes % 2)
    ++_nBitMapBytes;
  
  // Allocate space for the new bitmap and initialize it to 0

  delete [] _bitMap;
  _bitMap = new ui08[_nBitMapBytes];
  memset(_bitMap, 0, _nBitMapBytes);

  // Copy the given bitmap.  The given bitmap has a byte for each
  // bit, so set the appropriate bit for every non-zero byte in the
  // given bitmap.

  for (int i = 0; i < bitmap_size; ++i)
  {
    if (new_bitmap[i] == 0)
      continue;
    
    int current_byte = i / 8;
    int current_bit = 7 - (i % 8);
	
    _bitMap[current_byte] |= (1 << current_bit);
  }

  _bitMapId = 0;
  
  _nBytes = NUM_HEADER_BYTES + _nBitMapBytes;

  _numUnusedBits = _nBitMapBytes*8-bitmap_size;

}

  
int
BMS::unpack( ui08 *bmsPtr ) 
{
   // initialize private variables
   _numUnusedBits = 0;
   _bitMapId      = 0;
   _nBitMapBytes  = 0;
   if (_bitMap)
      delete[] _bitMap;
   // 
   // Length in bytes of section 
   //
   _nBytes = _upkUnsigned3( bmsPtr[0], bmsPtr[1], bmsPtr[2] );

   //
   // Number of unused bytes at the end of the section -
   // the section will be padded with zeros to get an even
   // number of bytes - see Grib documentation
   //
   _numUnusedBits = (int) bmsPtr[3];

   //
   // Some centers use a predefined bit map - if the id is 
   // something other than zero, it is one of these predefined
   // bit maps
   //
   _bitMapId = _upkUnsigned2( bmsPtr[4], bmsPtr[5] );
   
   //
   // If bit map id is zero, a bit map follows
   //
   if (_bitMapId == 0) {
   
      //
      // Just copy bit map
      //
      _nBitMapBytes = _nBytes - 6;

      if (_nBitMapBytes > 0 ) {
	 _bitMap = new ui08[_nBitMapBytes];
	 memcpy( (void *) _bitMap,  bmsPtr + 6, _nBitMapBytes );
      }

   } 
   else {
    
      //
      // No predefined bit maps right now
      //
      cerr << "ERROR:: no predefined bit maps present\n";

      return( GRIB_FAILURE );
   }
    
   return( GRIB_SUCCESS );
}

int
BMS::unpack( ui08 *bmsPtr, int nPtsExpected)
{
   // 
   // Length in bytes of section 
   //
   _nBytes = _upkUnsigned3( bmsPtr[0], bmsPtr[1], bmsPtr[2] );

   //
   // Number of unused bytes at the end of the section -
   // the section will be padded with zeros to get an even
   // number of bytes - see Grib documentation
   //
   _numUnusedBits = (int) bmsPtr[3];

   //
   // Some centers use a predefined bit map - if the id is 
   // something other than zero, it is one of these predefined
   // bit maps
   //
   _bitMapId = _upkUnsigned2( bmsPtr[4], bmsPtr[5] );
   
   //
   // If bit map id is zero, a bit map follows
   //
   if( _bitMapId == 0 ) {
   
      //
      // Just copy bit map
      //
      _nBitMapBytes = _nBytes - 6;
      if( _nBitMapBytes > 0 ) {
	 _bitMap = new ui08[_nBitMapBytes];
	 memcpy( (void *) _bitMap, (void *) &bmsPtr[6], _nBitMapBytes );
      }
   } else {
    
      //
      // No predefined bit maps right now
      //
      return( GRIB_FAILURE );
   }
    
   //
   // Check number of points in bitmap
   //
   if( _nBitMapBytes * 8 - _numUnusedBits != nPtsExpected ) {
      return( GRIB_FAILURE );
   }
    
   return( GRIB_SUCCESS );
}

int BMS::pack(ui08 *bmsPtr)
{
  // Length in bytes of section 

  _pkUnsigned3(_nBytes, &(bmsPtr[0]));

  // Number of unused bytes at the end of the section -
  // the section will be padded with zeros to get an even
  // number of bytes - see Grib documentation

  bmsPtr[3] = (ui08)_numUnusedBits;

  // Some centers use a predefined bit map - if the id is 
  // something other than zero, it is one of these predefined
  // bit maps

  _pkUnsigned2(_bitMapId, &(bmsPtr[4]));
   
  // If bit map id is zero, a bit map follows

  if (_bitMapId == 0)
  {
    // Just copy bit map

    if (_nBitMapBytes > 0)
      memcpy((void *)(bmsPtr + 6), (void *)_bitMap, _nBitMapBytes);
  } 
  else
  {
    // No predefined bit maps right now
    
    cerr << "ERROR:: no predefined bit maps present\n";

    return GRIB_FAILURE;
  }
    
   return GRIB_SUCCESS;
}

void
BMS::print(FILE *stream, const bool print_bitmap) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Bit Map Section:\n");
  fprintf(stream, "-------------------------\n");
  fprintf(stream, "BMS length %d\n", _nBytes);
  fprintf(stream, "   Number of bytes in bitmap %d\n", _nBitMapBytes);
  fprintf(stream, "   Number Unused bits %d\n", _numUnusedBits);
  fprintf(stream, "   Predefined bit map 0 = no -> %d\n" , _bitMapId);
  
  if (print_bitmap)
  {
    for (int i = 0; i < _nBitMapBytes; ++i)
    {
      if (i != 0 && (i % 5 == 0))
	fprintf(stream, "\n");

      if (_bitMap[i] & 128) fprintf(stream, "     1");
      else fprintf(stream, "     0");
    
      if (_bitMap[i] & 64) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 32) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 16) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 8) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 4) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 2) fprintf(stream, "1");
      else fprintf(stream, "0");
    
      if (_bitMap[i] & 1) fprintf(stream, "1");
      else fprintf(stream, "0");
    }
  }
  
} 

void
BMS::print(ostream &stream, const bool print_bitmap) const
{
  stream << endl << endl;
  stream << "Grib Bit Map Section:" << endl;
  stream << "-------------------------" << endl;
  stream << "BMS length " << _nBytes << endl;
  stream << "   Number of bytes in bitmap " << _nBitMapBytes << endl;
  stream << "   Number Unused bits " << _numUnusedBits << endl;
  stream << "   Predefined bit map 0 = no -> " << _bitMapId << endl;
  
  if (print_bitmap)
  {
    for (int i = 0; i < _nBitMapBytes; ++i)
    {
      if (i != 0 && (i % 5 == 0))
	stream << endl;

      if (_bitMap[i] & 128) stream << "     1";
      else stream << "     0";
    
      if (_bitMap[i] & 64) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 32) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 16) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 8) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 4) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 2) stream << "1";
      else stream << "0";
    
      if (_bitMap[i] & 1) stream << "1";
      else stream << "0";
    }
  }
  
} 
