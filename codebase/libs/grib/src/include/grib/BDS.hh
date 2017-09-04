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
///////////////////////////////////////////////
// BDS - Binary Data Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
//////////////////////////////////////////////
#ifndef _BDS_
#define _BDS_

#include <math.h>
#include <string>
#include <vector>

#include "GribSection.hh"

using namespace std;

class BDS: public GribSection {

public:

   BDS();
   ~BDS();

   /// Populate this structure from the raw BDS data.
   /// Sometimes the row length is fixed, and sometimes we have an irregular grid.
   /// \param rowLen the number of data points in every row
   int    unpack( ui08 *bdsPtr, int nPts, int decimalScale, 
                  int rowLen = 0, ui08 *bitmap = NULL,
                  float bottomVal = 0., float toVal = 0. );

   /// \param rowLen the number of data points in each row (there are _numFirstOrder rows)
   int    unpack( ui08 *bdsPtr, int nPts, int decimalScale, 
                  vector<int> &rowLen, ui08 *bitmap = NULL,
                  float bottomVal = 0., float toVal = 0. );

   int    pack(ui08 *bdsPtr, ui08 *bitmap = NULL);

   int    unpackBitmap (ui08 *bitMapPtr, int nPts, float bottomVal,
                                           float topVal);

   inline fl32  *getData(){ return( _data ); }
   inline const fl32  *getConstData() const { return( _data ); }
   inline int    getNumValues() const { return( _numValues ); }

  fl32 getMinDataValue() const;
  fl32 getMaxDataValue() const;
  const fl32 *getData() const { return _data; };
  
  void setData(fl32 *new_data, const int num_values,
	       const double min_data_value, const double max_data_value,
	       const int decimal_scale, const int max_bit_len = 0);
  
  void setOutputBitLen(const int bit_len);
  
   //
   // Constants
   //
   static const double REFERENCE_SCALE_FACTOR;
   static const fl32   MISSING_DATA;
   static const int    MISSING_INT_DATA;

   void print(FILE *stream) const;
   void printData(FILE *stream) const;
   void print(ostream &stream) const;
   void printData(ostream &stream) const;
   

private:

  static const int NUM_BYTES_HEADER;
  
   bool   _gridPointData;
   bool   _simplePacking;
   bool   _originalDataFloat;
   bool   _additionalFlags;
   
   int    _numUnusedBits;
   int    _binaryScale;
   
  double _refValue;     // _reference (min value) * scale
    
   int    _nBitsPerDatum;
   int    _numValues;
   
   fl32  *_data;
   
   bool   _singleDatum;
   bool   _noSecondaryBitmap;
   bool   _secondOrderConstWidth;

   //
   // For second order packing
   //
   int    _firstOrderOffset;
   int    _secondOrderOffset;
   int    _numFirstOrder;
   int    _numSecondOrder;
   ui08  *_secondOrderWidths;
   ui08  *_secondaryBitmap;
   int    _secondaryBitmapSize;

   //
   // Variables used for unpacking data
   //
  fl32   _reference;    // min data value (like bias in MDV)
  fl32   _scale;        // power of 10 used to convert data to int for packing

   int   *_firstOrder;
   int   *_secondOrder;

   // 
   // private functions
   //
  void _calcNumBytes()
  { 
    _nBytes = NUM_BYTES_HEADER + (_numValues * _nBitsPerDatum + 7) / 8; 
    if(_nBytes % 2) _nBytes++;
  }
  
  void _calcUnusedBits()
  { 
    _numUnusedBits = _nBytes*8 - NUM_BYTES_HEADER*8 - _nBitsPerDatum*(long int)_numValues;
  }
  
   void   _clearData();
   double _upkReference( ui08 a, ui08 b, ui08 c, ui08 d );
   int    _pkReference(ui08 *bds_ptr);
   int    _upkFirstOrder( ui08 *bdsPtr, ui08 *bitmap );
   int    _pkFirstOrder( ui08 *bdsPtr, ui08 *bitmap, int &num_data_bytes );
   int    _upkSecondOrder( ui08 *bdsPtr, vector<int> &rowLen, ui08 *bitmap );
   int    _upkData( ui08 *dataPtr, ui08 *bitmap, int *dataInt,
		    int dataWidth, int nVals, int startBit = 0, 
		    int bitmapStartBit = 0 );
   int    _pkData(ui08 *packedDataPtr, ui08 *bitmap, unsigned int *dataInt,
		  int dataWidth, int nVals, int &num_data_bytes,
		  int startBit = 0, int bitmapStartBit = 0);
   void   _calcDataWidthAndScale(const double min_data_value,
				 const double max_data_value,
				 const int decimal_scale,
				 const int max_bit_len);
  
   
};

#endif

   
