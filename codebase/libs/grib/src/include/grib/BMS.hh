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
///////////////////////////////////////////////////
// BMS - Bit Map Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
//////////////////////////////////////////////////
#ifndef _BMS_
#define _BMS_

#include "GribSection.hh"
using namespace std;

class BMS:public GribSection {

public:

   BMS();
   ~BMS();
   
   int   unpack( ui08 *bmsPtr, int nPtsExpected );
   int   unpack( ui08 *bmsPtr );
   int   pack( ui08 *bmsPtr );

   inline ui08 *getBitMap(){ return( _bitMap ); }
   inline int   getBitMapSize(){ return( _nBitMapBytes ); }
   inline int getSize() { return( _nBytes); } 

   void setBitmap(const ui08 *new_bitmap, const int bitmap_size);
  
   void print(FILE *, const bool print_bitmap = false) const;
   void print(ostream &stream, const bool print_bitmap = false) const;

private:

  static const int NUM_HEADER_BYTES;
  
   int _numUnusedBits;
   int _bitMapId;
   
   int   _nBitMapBytes;
   ui08 *_bitMap;
   
};

#endif
