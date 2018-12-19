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
// $Id: BMS.cc,v 1.13 2018/10/13 22:29:11 dixon Exp $
//
/////////////////////////////////////////////////

#include <string.h>
#include <iostream>
#include <cmath>

#include <grib2/BMS.hh>
#include <grib2/DS.hh>

using namespace std;

namespace Grib2 {

BMS::BMS (si32 bitMapType, si32 grid_size, si32 *bit_map) :
  GribSection()
{
  _bitMapIndicator = bitMapType;
  _sectionNum = 6;
  _sectionLen = 6;
  _gridSz = grid_size;

  if(bitMapType == 254 && bit_map == NULL)
  {
    _bitMap = NULL;
    _bitMapIndicator = 255;
    return;
  }
  if(bitMapType == 0 && bit_map == NULL)
  {
    cerr << "ERROR: BMS()" << endl;
    cerr << "Defined bit map code but bit map pointer is null." << endl;
    _bitMap = NULL;
    _bitMapIndicator = 255;
    return;
  }

   switch (_bitMapIndicator) {
     case 0:
       if (_gridSz > 0 ) {
          _bitMap = new si32[_gridSz];
	  for(int a = 0; a < _gridSz; a++)
	    _bitMap[a] = bit_map[a];
	  _sectionLen = 6 + (int)ceil(_gridSz / 8.0);
       }
       break;
     case 254:
       _bitMap = bit_map;
       break;
     // A bit map does not apply to this product
     case 255:
       _bitMap = NULL;
       break;

     default:
       cerr << "ERROR: BMS()" << endl;
       cerr << "Pre-determined bit maps are not currently supported." << endl;
       return;
   }
}

BMS::~BMS() 
{
  // Dont delete the bitmap unless indicator == 0
  // Which would mean we created the memory
  if( _bitMapIndicator == 0 && _bitMap )
    delete[] _bitMap;
}

  
int BMS::unpack( ui08 *bmsPtr ) 
{
   // 
   // Length in bytes of section 
   //
   _sectionLen = _upkUnsigned4( bmsPtr[0], bmsPtr[1], bmsPtr[2], bmsPtr[3] );

   _sectionNum = (si32 ) bmsPtr[4];

   if (_sectionNum != 6) {
     cerr << "ERROR: BMS::unpack()" << endl;
     cerr << "Incorrect section found, should be 6, found " << _sectionNum << endl;
     return( GRIB_FAILURE );
   }

   si32 nBitMapBytes = _sectionLen - 6;

   int bitMapIndicator = (int) bmsPtr[5];

   _gridSz = nBitMapBytes * 8;

   switch (bitMapIndicator) {

     // Bit-map present and specified in this section
     case 0:

       if (nBitMapBytes > 0 ) {
	 if( _bitMapIndicator == 0 && _bitMap )
	   delete[] _bitMap;

	 _bitMap = new si32[_gridSz];
	 DS::gbits (bmsPtr + 6, _bitMap, 0, 1, 0, _gridSz);

       }
       break;

     // A bit map used is defined previously in the same GRIB message
     case 254:
       // already set
       break;

     // A bit map does not apply to this product
     case 255:
       _bitMap = NULL;
       break;

     default:
       cerr << "ERROR: BMS::unpack()" << endl;
       cerr << "Pre-determined bit maps are not currently supported." << endl;
       return( GRIB_FAILURE );
   }

   _bitMapIndicator = bitMapIndicator;

   return( GRIB_SUCCESS );
}


int BMS::pack(ui08 *bmsPtr)
{
  _pkUnsigned4(_sectionLen, &(bmsPtr[0]));

  bmsPtr[4] = (ui08) _sectionNum;

  bmsPtr[5] = (ui08) _bitMapIndicator;


  // Bit-map present and specified in this section
  if(_bitMapIndicator == 0) 
   {
       int num = _gridSz / 8;
       int index = 6;
       int ind = 0;
       for(int i = 0; i < num; i++) {
	 bmsPtr[index++] = (_bitMap[ind + 0] * 128) + (_bitMap[ind + 1] * 64) + (_bitMap[ind + 2] * 32) + 
	   (_bitMap[ind + 3] * 16) + (_bitMap[ind + 4] * 8) + (_bitMap[ind + 5] * 4) + 
	   (_bitMap[ind + 6] * 2) + _bitMap[ind + 7];
	 ind += 8;
       }
       num = _gridSz % 8;
       if(num > 0) 
       {
	 int rem[8];
	 for(int i = 0; i < num; i++)
	   rem[i] = _bitMap[ind + i];
	 for(int i = num; i < 8; i++)
	   rem[i] = 0;
	 bmsPtr[index++] = (rem[0] * 128) + (rem[1] * 64) + (rem[2] * 32) + 
	   (rem[3] * 16) + (rem[4] * 8) + (rem[5] * 4) + 
	   (rem[6] * 2) + rem[7];
       }
       if(index != _sectionLen) {
	 cerr << "WARNING: BMS::pack()" << endl;
	 cerr << "Estimated Bitmap size != actual packed bitmap size." << endl;
       }

       //DS::sbits (bmsPtr + 6, _bitMap, 0, 1, 0, _gridSz);
       // Pad with zeros to fill Octet
       //if(_gridSz % 8 != 0) {
       //int zero = 0;
       //DS::sbit (bmsPtr, &zero,  48 + _gridSz, 8 - (_gridSz % 8));
       //}
   } else if(_bitMapIndicator != 254 && _bitMapIndicator != 255) {
     cerr << "ERROR: BMS::pack()" << endl;
     cerr << "Pre-determined bit maps are not currently supported." << endl;
     return( GRIB_FAILURE );
   }

   return GRIB_SUCCESS;
}

void BMS::print(FILE *stream, const bool print_bitmap) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Bit Map Section:\n");
  fprintf(stream, "--------------------------------------------------\n");
  fprintf(stream, "BMS length %d\n", _sectionLen);
  fprintf(stream, "   Bit-map indicator %d\n", _bitMapIndicator );
  switch (_bitMapIndicator) {
    case 0:
      fprintf(stream, "     Bit-map present and specified in this section\n");
      break;
    case 254:
      fprintf(stream, "     A bit map used is defined previously in the same GRIB message\n");
      break;
    case 255:
      fprintf(stream, "     A bit map does not apply to this product\n");
      break;
    default:
      fprintf(stream, "     The bit map pre-determined by the originating/generating Centre\n");
      fprintf(stream, "     it is not specified in this Section.\n");

  }
  
  if (print_bitmap && _bitMapIndicator == 0)
  {
    int nBytes = _sectionLen - 6;
    for (int i = 0; i < nBytes; ++i)
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
    fprintf(stream, "\n");
} 

} // namespace Grib2

