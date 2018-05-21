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
//////////////////////////////////////////////
// BDS - Binary Data Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
//////////////////////////////////////////////
#include <cstring>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <iostream>

#include <grib/constants.h>
#include <grib/BDS.hh>
using namespace std;

//
// Constants 
//   - REFERENCE_SCALE_FACTOR is the scale factor used to
//     unpack the reference value.  It is equal to 2^(24)
//   - MISSING_DATA is the value used to indicate a
//     missing value in the scaled data
//   - MISSING_INT_DATA is the value used to indicate a
//     missing value in the data before it is scaled
//
const double BDS::REFERENCE_SCALE_FACTOR = 16777216; 
const fl32   BDS::MISSING_DATA           = FLT_MAX;
const int    BDS::MISSING_INT_DATA       = INT_MAX;

const int    BDS::NUM_BYTES_HEADER       = 11;


BDS::BDS() :
  GribSection(),
  _gridPointData(true),
  _simplePacking(true),
  _originalDataFloat(true),
  _additionalFlags(false),
  _numUnusedBits(0),
  _binaryScale(0),
  _refValue(0.0),
  _nBitsPerDatum(0),
  _numValues(0),
  _data(0),
  _singleDatum(true),
  _noSecondaryBitmap(true),
  _secondOrderConstWidth(true),
  _firstOrderOffset(0),
  _secondOrderOffset(0),
  _numFirstOrder(0),
  _numSecondOrder(0),
  _secondOrderWidths(0),
  _secondaryBitmap(0),
  _secondaryBitmapSize(0),
  _reference(0.0),
  _scale(1.0),
  _firstOrder(0),
  _secondOrder(0)
{
  _calcNumBytes();
}

BDS::~BDS() 
{
   _clearData();
}

fl32 BDS::getMinDataValue() const
{
  fl32 min_value = MISSING_DATA;
  bool value_found = false;
  
  for (int i = 0; i < _numValues; ++i)
  {
    if (_data[i] == MISSING_DATA)
      continue;
    
    if (!value_found ||
	_data[i] < min_value)
    {
      min_value = _data[i];
      value_found = true;
    }
  }
  
  return min_value;
}

fl32 BDS::getMaxDataValue() const
{
  fl32 max_value = MISSING_DATA;
  bool value_found = false;
  
  for (int i = 0; i < _numValues; ++i)
  {
    if (_data[i] == MISSING_DATA)
      continue;
    
    if (!value_found ||
	_data[i] > max_value)
    {
      max_value = _data[i];
      value_found = true;
    }
  }
  
  return max_value;
}

void
BDS::_clearData() 
{
  delete[] _data;
  delete[] _secondOrderWidths;
  delete[] _secondaryBitmap;
  delete[] _firstOrder;
  delete[] _secondOrder;

  _data = 0;
  _secondOrderWidths = 0;
  _secondaryBitmap = 0;
  _firstOrder = 0;
  _secondOrder = 0;
}

void BDS::setData(fl32 *new_data, const int num_values,
		  const double min_data_value,
		  const double max_data_value,
		  const int decimal_scale,
		  const int max_bit_len)
{

  

  delete [] _data;
  _data = new_data;
  _numValues = num_values;
    
  _reference = min_data_value;
  _refValue = _reference * pow(10.0, decimal_scale);
    
  _calcDataWidthAndScale(min_data_value, max_data_value, 
			 decimal_scale, max_bit_len);

  _calcNumBytes();
  _calcUnusedBits();
}

void BDS::setOutputBitLen(const int bit_len)
{ 
  _nBitsPerDatum = bit_len; 
  _calcNumBytes(); 
  _calcUnusedBits(); 
}
  

int BDS::unpack( ui08 *bdsPtr, int nPts, int decimalScale, 
             int rowLen, ui08 *bitmap, float bottomVal, float topVal ) 
{
  _simplePacking = true;
  int numberOfRows = 1;

  if( bdsPtr[3] & 64 )
    _simplePacking = false;

  if (!_simplePacking) {
    // retrieve the number of rows (_numFirstOrder)
    numberOfRows = _upkUnsigned2( bdsPtr[16], bdsPtr[17] );
  }

  // create a vector of row lengths here
  vector<int> localRowLengths;
  for (int row = 0; row < numberOfRows; row++) {
    localRowLengths.push_back(rowLen);
  }

  // call the vector version
  int retValue = unpack(bdsPtr, nPts, decimalScale, localRowLengths, bitmap, bottomVal, topVal);
  return retValue;
}


int BDS::unpack( ui08 *bdsPtr, int nPts, int decimalScale, 
             vector<int> &rowLen, ui08 *bitmap, float bottomVal, float topVal ) 
{
   int secondaryBitmapOffset = 0;

   //
   // Clear out all of the arrays first
   //
   _clearData();
   
   _gridPointData         = true;
   _simplePacking         = true;
   _originalDataFloat     = true;
   _additionalFlags       = false;
   _singleDatum           = true;
   _noSecondaryBitmap     = true;
   _secondOrderConstWidth = true;
   _numUnusedBits         = 0;
   _binaryScale           = 1;
   _nBitsPerDatum         = 0;
   _numValues             = 0;
   _refValue              = 0.0;
   _reference             = 0.0;
   _scale                 = 1.0;

//   if ((bdsPtr[0] == '7') && (bdsPtr[1] == '7')
//                    && (bdsPtr[2] == '7') && (bdsPtr[3] == '7')) {
      // The bitmap represents the total image and is not used to mark
      // areas of no data 

//      cout << "Found a bitmap image, unpacking!!!!!" << endl;
//      cout << "Top value is " << topVal << endl;
//      cout << "Bottom value is " << bottomVal << endl;
//      ui08 *bitmapPtr = bitmap;
//      if (unpackBitmap(bitmapPtr, nPts, bottomVal, topVal) != GRIB_SUCCESS) {
//        cout << "Error unpacking Bit Map image" << endl;
//        return(GRIB_FAILURE);
//      }
//      return (GRIB_SUCCESS);
//   }

   //
   // Length of section
   //
   _nBytes = _upkUnsigned3( bdsPtr[0], bdsPtr[1], bdsPtr[2] );
   
   //
   // Flags
   //
   if( bdsPtr[3] & 128 ) 
      _gridPointData = false;
   if( bdsPtr[3] & 64 )
      _simplePacking = false;
   if( bdsPtr[3] & 32 )
      _originalDataFloat = false;
   if( bdsPtr[3] & 16 )
      _additionalFlags = true;
   
   //
   // Number of unused bits at the end of the section
   //
   _numUnusedBits = (int) (bdsPtr[3] & 15);

   //
   // Note that data values are scaled via the following equation:
   //
   //      Y * 10^D = R + (X * 2^E)
   //
   //   Y = original value
   //   D = decimal scale factor (see PDS section)
   //   R = reference value
   //   X = internal value
   //   E = binary scale
   //
   // "The WMO Format for the Storage of Weather Product Information
   // and the Exchange of Weather Product Messages in Gridded Binary
   // Form as Used by NCEP Central Operations", Clifford H. Dey,
   // March 10, 1998, U.S. Department of Commerce, National Oceanic
   // and Atmospheric Administration, National Weather Service,
   // National Centers for Environmental Prediction, Sec. 0, page 4
   //
   
   //
   // Binary scale factor (E)
   //
   _binaryScale = _upkSigned2( bdsPtr[4], bdsPtr[5] );
   
   //
   // Reference value (R)
   //
   _refValue  = _upkReference( bdsPtr[6], bdsPtr[7], bdsPtr[8], bdsPtr[9] );
   _reference = _refValue / pow(10.0, decimalScale);
   _scale     = pow(2.0, _binaryScale) / pow(10.0, decimalScale);
   
   //
   // Width in bits per data value - 
   //   If second order packing is used this will now
   //   refer to the width of the collection of first
   //   order packed data values (See Grib documentation)
   //
   _nBitsPerDatum = (int) bdsPtr[10];
   
   //
   // If there are more flags, load them up
   //
   if( _additionalFlags ) {
      if( bdsPtr[13] & 64 )
         _singleDatum = false;
      if( bdsPtr[13] & 32 )
         _noSecondaryBitmap = false;
      if( bdsPtr[13] & 16 )
         _secondOrderConstWidth =  false;
   }

   //
   // Matrix values not supported at this time
   //
   if( !_singleDatum ) {
     cerr << "   Error: not single datum" << endl;
      return( GRIB_FAILURE );
   }

   //
   // Only decode grid point data - spherical harmonic coefficients
   // not supported at this time
   //
   if( !_gridPointData ) {
     cerr << "    Error: not grid point data" << endl;
      return( GRIB_FAILURE );
   }

   //
   // Create data array
   //
   if( nPts <= 0 ) {
     cerr << "    Error: nPts = " << nPts << endl;
      return( GRIB_FAILURE );
   } 
   _data      = new fl32[nPts];
   _numValues = nPts;

   //
   // Unpack the data
   //
   if( _simplePacking ) {
      if( _upkFirstOrder( bdsPtr, bitmap ) != GRIB_SUCCESS ) {
	cerr << "    Error: _upkFirstOrder returned error" << endl;
	 return( GRIB_FAILURE );
      }
   } else {

      // 
      // Second order packing employed
      //
      _firstOrderOffset  = _upkUnsigned2( bdsPtr[11], bdsPtr[12] ) - 1;
      _secondOrderOffset = _upkUnsigned2( bdsPtr[14], bdsPtr[15] ) - 1;
      _numFirstOrder     = _upkUnsigned2( bdsPtr[16], bdsPtr[17] );
      _numSecondOrder    = _upkUnsigned2( bdsPtr[18], bdsPtr[19] );

      if( _secondOrderConstWidth ) {
	 _secondOrderWidths = new ui08[1];
	 *_secondOrderWidths = bdsPtr[21];
         secondaryBitmapOffset = 22;
      } else {
	 _secondOrderWidths = new ui08[_numFirstOrder];
         memcpy( (void *) _secondOrderWidths, (void *)(&bdsPtr[21]),
                 _numFirstOrder );
         secondaryBitmapOffset = 21 + _numFirstOrder;
      }
      
      if( _noSecondaryBitmap == false ) {
	 
        _secondaryBitmapSize = _numSecondOrder / 8;
        _secondaryBitmap     = new ui08[_secondaryBitmapSize];
        memcpy( (void *) _secondaryBitmap, 
                (void *)(&bdsPtr[secondaryBitmapOffset]),
                _secondaryBitmapSize );
      }

      if( _upkSecondOrder( bdsPtr, rowLen, bitmap ) != GRIB_SUCCESS ) {
	 return( GRIB_FAILURE );
      }
      
   } 

   return( GRIB_SUCCESS );
   
}

  
int BDS::pack(ui08 *bds_ptr, ui08 *bitmap)
{
  static const string method_name = "BDS::pack()";
  
  // Flags

  bds_ptr[3] = 0;

  if (!_gridPointData)
    bds_ptr[3] &= 128;
  if (!_simplePacking)
    bds_ptr[3] &= 64;
  if (!_originalDataFloat)
    bds_ptr[3] &= 32;
  if (_additionalFlags)
    bds_ptr[3] &= 16;
   
  
  //  bds_ptr[3] = bds_ptr[3] | (_numUnusedBits & 15);

  // Binary scale factor (E)
  
  _pkSigned2(_binaryScale, &(bds_ptr[4]));
   
  // Reference value (R)

  if (_pkReference(bds_ptr) != GRIB_SUCCESS)
    return GRIB_FAILURE;
   
  // Width in bits per data value

  bds_ptr[10] = (ui08)_nBitsPerDatum;
   
  // If there are more flags, load them up

  
  if (_additionalFlags)
  {
    bds_ptr[13] = 0;

    if (!_singleDatum)
      bds_ptr[13] &= 64;
    if (!_noSecondaryBitmap)
      bds_ptr[13] &= 32;
    if (!_secondOrderConstWidth)
      bds_ptr[13] &= 16;
  }

  // Pack the data

  int num_data_bytes;
  
  if (_simplePacking)
  {
    if(_pkFirstOrder(bds_ptr, bitmap, num_data_bytes) != GRIB_SUCCESS)
      return GRIB_FAILURE;
  }
  else
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Second order packing not yet implemented" << endl;
    
    return GRIB_FAILURE;
    
  } 

  // Length of section
  _nBytes = NUM_BYTES_HEADER + num_data_bytes;
  if (_nBytes % 2) {
    _nBytes++;
  }


  _pkUnsigned3(_nBytes, &(bds_ptr[0]));
  _calcUnusedBits();
  bds_ptr[3] = bds_ptr[3] | (_numUnusedBits & 15);

  return GRIB_SUCCESS;
}

  
double
BDS::_upkReference( ui08 a, ui08 b, ui08 c, ui08 d )
{
   double mantissa;
   double exponent;
   double value;
   double sign;
   double factor;
   
   if( a & 128 )
      sign = -1.0;
   else
      sign = 1.0;
   
   mantissa = (double) _upkUnsigned3( b, c, d );
   exponent = (double) (a & 127);
   factor   = (double) pow( 16, exponent - 64 );
   
   //
   // Unpack the reference value - see Grib documentation
   //
   value = sign * mantissa * factor / REFERENCE_SCALE_FACTOR;
   
   return( value );
}

int BDS::_pkReference(ui08 *bds_ptr)
{
  static const string method_name = "BDS::_pkReference()";
  
  int exponent;
  int mantissa;
  
  // Test for zero

  if (_refValue == 0.0)
  {
    exponent = 0;
    mantissa = 0;
  }
  else
  {
    double zeps = 1.0e-8;
    double zref = _refValue;

    int sign = 0;
    
    if (zref < 0.0)
    {
      sign = 128;
      zref = -zref;
    }

    // Calculate the exponent

    exponent = (int)(log(zref) * (1.0 / log(16.0)) + 64.0 + 1.0 + zeps);
    
    if (exponent < 0) exponent = 0;
    if (exponent > 127) exponent = 127;
    
    // Calculate the mantissa

    mantissa = (int)((zref / pow(16.0, exponent - 70)) + 0.5);
    
    // Make sure the mantissa value doesn't exceed 24 bits
    
    if (mantissa > 16777215)
    {
      exponent++;
    
      // Now find the closest number in GRIB format to the reference value

      mantissa = (int)((zref / pow(16.0, exponent - 70)) + 0.5);

      // Check to see that the mantissa value doesn't exceed 24 bits again

      if (mantissa > 16777215)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Bad mantissa value for reference value = " << _refValue << endl;
      
	return GRIB_FAILURE;
      }
    }

    // Add the sign bit to the exponent

    exponent += sign;
  }
  
  bds_ptr[6] = (ui08)exponent;
  _pkUnsigned3(mantissa, &(bds_ptr[7]));
  
  return GRIB_SUCCESS;
}

int
BDS::_upkFirstOrder( ui08 *bdsPtr, ui08 *bitmap ) 
{
   int    i;
   int   *firstOrderPtr;
   ui08  *packedDataPtr = bdsPtr + 11;
   fl32  *outputDataPtr = _data;
   
   //
   // If using grid point data and simple packing, there should
   // not be any additional flags
   //
   if( _additionalFlags ) {
      cout << "WARNING Should not find additional flags with simple packing" << endl << flush;
   }

   //
   // Create first order data array
   //
   _firstOrder    = new int[_numValues];
   memset(_firstOrder, 0, _numValues * sizeof(int));
   
   //
   // If the number of bits per data point is set to zero
   // the data is constant, and we're done
   //
   if( _nBitsPerDatum == 0) {
      for( i = 0; i < _numValues; i++ ) {
	 *outputDataPtr = _reference;
         outputDataPtr++;
      }
      return( GRIB_SUCCESS );
   }

   //
   // Unpack the data
   //
   if( _upkData( packedDataPtr, bitmap, _firstOrder, 
		 _nBitsPerDatum, _numValues ) != GRIB_SUCCESS ) {
      return( GRIB_FAILURE );
   }

   firstOrderPtr = _firstOrder;
   for( i = 0; i < _numValues; i++ ) {
      if( *firstOrderPtr == MISSING_INT_DATA ) {
	 *outputDataPtr = MISSING_DATA;
      } else {
	 *outputDataPtr = *firstOrderPtr * _scale + _reference;
      }
      outputDataPtr++;
      firstOrderPtr++;
   }

   return( GRIB_SUCCESS );
}

int
BDS::_pkFirstOrder( ui08 *bdsPtr, ui08 *bitmap, int &num_data_bytes )
{
  // always initialize outputs in case of early return
  num_data_bytes = 1;

  ui08  *packedDataPtr = bdsPtr + 11;
   
  //
  // If using grid point data and simple packing, there should
  // not be any additional flags
  //
  if( _additionalFlags )
    cerr << "WARNING Should not use additional flags with simple packing" << endl;

  //
  // If the number of bits per data point is set to zero
  // the data is constant, and we're done
  //
  if (_nBitsPerDatum == 0)
    return GRIB_SUCCESS;
  
  //
  // Create first order data array
  //
  unsigned int *first_order    = new unsigned int[_numValues];
  unsigned int *first_order_ptr = first_order;
  
  fl32 *data_ptr = _data;
  
  for (int i = 0; i < _numValues; ++i)
  {
    if (*data_ptr == MISSING_DATA)
      *first_order_ptr = MISSING_INT_DATA;
    else
      *first_order_ptr =
	(unsigned int)((*data_ptr - _reference) / _scale + 0.5);
    
    ++first_order_ptr;
    ++data_ptr;
  }
      
  //
  // Pack the data
  //
  if (_pkData(packedDataPtr, bitmap, first_order, 
	      _nBitsPerDatum, _numValues, num_data_bytes) != GRIB_SUCCESS)
  {
    delete [] first_order;
    return GRIB_FAILURE;
  }
  
  delete [] first_order;
  
  return GRIB_SUCCESS;
}

//
// Have never encountered a file that has used second order
// packing, so this code has not been tested yet
//
int
BDS::_upkSecondOrder( ui08 *bdsPtr, vector<int> &rowLen, ui08 *bitmap ) 
{
   ui08 *firstOrderInputPtr  = bdsPtr + _firstOrderOffset;
   ui08 *secondOrderInputPtr = bdsPtr + _secondOrderOffset;
   ui08 *bitmapPtr           = bitmap;
   ui08 *secBitmapPtr        = _secondaryBitmap;
   ui08 *secWidthPtr         = _secondOrderWidths;
   ui08  currentWidth = 0;

   int  *firstOrderPtr = 0;
   int  *secondOrderPtr = 0;
   int   currentFirstOrder;
   int   mapMask             = 128;
   int   currentNVals        = 0;
   int   bitmapByteCounter   = 0;
   int   bitmapBitOffset     = 0;
   int   secBitmapBitOffset  = 0;
   int   secOrderBitOffset   = 0;
   int   start               = 0;
   int   end                 = 0;

   int   i, j;

   fl32 *outputDataPtr = _data;

   //
   // We can't do anything if there are no first order values
   //
   if( _numFirstOrder == 0 ) {
      return( GRIB_FAILURE );
   }

   //
   // Unpack the first order values - bitmap doesn't apply to
   // these values
   //
   _firstOrder       = new int[_numFirstOrder];
   if( _upkData( firstOrderInputPtr, NULL, _firstOrder, 
		 _nBitsPerDatum, _numFirstOrder ) != GRIB_SUCCESS ) {
      return( GRIB_FAILURE );
   }

   //
   // If there is a constant width for the second order values,
   // set it now
   //
   if( _secondOrderConstWidth ) {
      currentWidth = *secWidthPtr;
   }
   
   //
   // If there is a secondary bit map, it is used to tell when each
   // section of secondary packed data begins and ends.  There should
   // be the same number of sections as there are first order values.
   // When the current bit is set to one in the secondary bit map,
   // that indicates that the corresponding second order value is
   // the start of the new section.  
   //
   if( _secondaryBitmap ) {

      //
      // The first bit in the secondary bit map should always
      // be one.  If it isn't, something's wrong.
      //
     if( (*secBitmapPtr & mapMask) != 1 ) {
	 return( GRIB_FAILURE );
      }
      
      //
      // The first bit is one, so we can now go on to the next
      // bit in the secondary bit map
      //
      mapMask = 64;

      // loop through the sub-sections (rows)
      firstOrderPtr = _firstOrder;
      for( i = 0; i < _numFirstOrder; i++ ) {
	 currentFirstOrder = *firstOrderPtr++;
         if( currentFirstOrder == MISSING_INT_DATA ) {
	    return( GRIB_FAILURE );
	 }
	 
         if( !_secondOrderConstWidth )
	    currentWidth  = *secWidthPtr++;
	 
         //
         // Find the start of the next section
         //
         while( (*secBitmapPtr & mapMask) == 0 ) {
	    if( mapMask == 1 ) {
	       secBitmapPtr++;
	       bitmapByteCounter++;
	       secBitmapBitOffset = 0;
	       mapMask = 128;
	    } else {
	       mapMask = mapMask << 1;
	       secBitmapBitOffset++;
            }
	 }
	 
         //
         // The start of the next section signals the end
         // of the current section.  The number of bits
         // in the secondary bit map between the start
         // of the current section and the start of the next
         // section indicates how many values there are
         // in the current section.
         //
         end = bitmapByteCounter*8 + secBitmapBitOffset;
         currentNVals = end - start;

         //
         // Allocate space for the second order values, this
         // needs to be redone every time, because the number
         // of values can change with each section.
         //
         if( _secondOrder ) {
	    delete[] _secondOrder;
	 }
	 _secondOrder       = new int[currentNVals];
         secondOrderPtr    = _secondOrder;
 
         //
         // If the current width is zero, all the data have the
         // same value, i.e. the scaled first order value for
         // this section.  If the width is greater than zero
         // unpack the data.  If the width is less than zero
         // there is something wrong.
         //
         if( currentWidth == 0 ) {
	    for( j = 0; j < currentNVals; j++ ) {
	       *outputDataPtr = currentFirstOrder * _scale + _reference;
	       outputDataPtr++;
	    }
         } else if( currentWidth > 0 ) {
	    
	     //
	     // Unpack the secondary values
	     //
	     if( _upkData( secondOrderInputPtr, bitmapPtr, _secondOrder,
			  currentWidth, currentNVals,
			  secOrderBitOffset, 
                          bitmapBitOffset ) != GRIB_SUCCESS ) {
		return( GRIB_FAILURE );
	     }
	 
	     //
	     // Get the data values
	     //
             secondOrderPtr = _secondOrder;
	     for( j = 0; j < currentNVals; j++ ) {
                if( *secondOrderPtr == MISSING_INT_DATA ) {
		   *outputDataPtr = MISSING_DATA;
		} else {
		   *outputDataPtr = (*secondOrderPtr + 
                      currentFirstOrder) * _scale + _reference;
                }
		outputDataPtr++;
		secondOrderPtr++;
	     }
         } else {
	    return( GRIB_FAILURE );
	 }

         //
         // Increment pointers and offsets - must be careful about 
         // the fact that the current positions should refer to
         // bits, not bytes.  This is why it is necessary to have both
         // pointers and offsets.  The offsets refer to which bit
         // is current in the bytes which the pointers reference.
         //
         secondOrderInputPtr += 
	    (secOrderBitOffset + currentNVals * currentWidth) / 8;
         secOrderBitOffset     = (int) 
	    fmod( secOrderBitOffset + currentNVals * currentWidth, 8.0 );

         if( bitmap ) {
            bitmapPtr      += (bitmapBitOffset + currentNVals) / 8;
            bitmapBitOffset = (int) fmod( bitmapBitOffset + currentNVals, 8.0 );
         }
      
         //
         // The start of the next section is one bit past the 
         // end of the current section.
         //
         start = end + 1;
	 
      }
   } else {

      // Process each section (or row) 
      for( i = 0; i < _numFirstOrder; i++ ) {
         // If we are not using the secondary bit map, the second
         // order packing is done on a row-by-row basis.  For that
         // reason, there are always the same number of second order
         // values in each section.  However, the widths of the second
         // order values may not be the same for all sections.
         delete[] _secondOrder;
         _secondOrder       = new int[rowLen[i]];
         secondOrderPtr    = _secondOrder;

	 currentFirstOrder = *firstOrderPtr++;
         if( currentFirstOrder == MISSING_INT_DATA ) {
	    return( GRIB_FAILURE );
	 }
	 
         if( !_secondOrderConstWidth )
	    currentWidth  = *secWidthPtr++;

         //
         // If the current width is zero, all the data have the
         // same value, i.e. the scaled first order value for
         // this section.  If the width is greater than zero
         // unpack the data.  If the width is less than zero
         // there is something wrong.
         //
         if( currentWidth == 0 ) {
	    for( j = 0; j < rowLen[i]; j++ ) {
	       *outputDataPtr = currentFirstOrder * _scale + _reference;
	       outputDataPtr++;
	    } 
         } else if( currentWidth > 0 ) {
	    
	     //
	     // Unpack the second order values
	     //
	     if( _upkData( secondOrderInputPtr, bitmapPtr, _secondOrder,
			  currentWidth, rowLen[i], secOrderBitOffset,
			  bitmapBitOffset ) != GRIB_SUCCESS ) {
		return( GRIB_FAILURE );
	     }

	     //
	     // Calculate the data values
	     //
             secondOrderPtr = _secondOrder;
	     for( j = 0; j < rowLen[i]; j++ ) {
                if( *secondOrderPtr == MISSING_INT_DATA ) {
		   *outputDataPtr = MISSING_DATA;
		} else {
		    *outputDataPtr = (*secondOrderPtr + 
				      currentFirstOrder) * _scale + _reference;
		}
		outputDataPtr++;
		secondOrderPtr++;
	     }
         } else {
	    return( GRIB_FAILURE );
	 }

	 //
         // Increment pointers and offsets - must be careful about 
         // the fact that the current positions should refer to
         // bits, not bytes.  This is why it is necessary to have both
         // pointers and offsets.  The offsets refer to which bit
         // is current in the bytes which the pointers reference.
         //
	 secondOrderInputPtr += 
	    (secOrderBitOffset + rowLen[i] * currentWidth) / 8;
         secOrderBitOffset     = (int)
	    fmod( secOrderBitOffset + rowLen[i] * currentWidth, 8.0 );
         
         if( bitmap ) {
            bitmapPtr      += (bitmapBitOffset + rowLen[i]) / 8;
	    bitmapBitOffset = (int) fmod( bitmapBitOffset + rowLen[i], 8.0 );
	 }

      }
      
   }
   
   return( GRIB_SUCCESS );
}

int
BDS::_upkData( ui08 *packedDataPtr, ui08 *bitmap, int *dataInt,
	       int dataWidth, int nVals, int startBit, 
	       int bitmapStartBit ) 
{
   int    *dataVal     = dataInt;

   //
   // Check offsets
   //
   if( startBit < 0 || bitmapStartBit < 0 ) {
     cerr << "    Error: startBit = " << startBit
	  << ", bitmapStartBit = " << bitmapStartBit << endl;
      return( GRIB_FAILURE );
   }

   //
   // Check data width
   //
   if( dataWidth > 31 ) {
     cerr << "    Error: dataWidth = " << dataWidth << endl;
      return( GRIB_FAILURE );
   }
     
  int mapMask = (int)pow(2.0, 7 - bitmapStartBit);
  int packed_bits_left = 8 - startBit;

  for (int i = 0; i < nVals; ++i)
  {
    // If we are using a bitmap and the current data value is missing
    // then we don't unpack anything, but we still continue on to the
    // end of the loop to increment everything. If we aren't using a
    // bitmap, always unpack the value.

    if (!bitmap ||
	(bitmap && (*bitmap & mapMask)))
    {
      // Unpack the value.

      int current_datum = 0;
      int bits_to_write = dataWidth;

      while (bits_to_write > 0)
      {
	if (bits_to_write > packed_bits_left)
	{
	  int data_mask = (int)pow(2.0, packed_bits_left) - 1;
	  current_datum = current_datum << packed_bits_left;
	  current_datum |= *packedDataPtr & data_mask;
	  bits_to_write -= packed_bits_left;
	  ++packedDataPtr;
	  packed_bits_left = 8;
	}
	else
	{
	  int bit_shift = packed_bits_left - bits_to_write;
	  int data_mask = ((int)pow(2.0, bits_to_write) - 1) << bit_shift;
	  current_datum = current_datum << bits_to_write;
	  current_datum |= (*packedDataPtr & data_mask) >> bit_shift;
	  packed_bits_left -= bits_to_write;
	  bits_to_write = 0;
	}

      }

      *dataVal++ = current_datum;
      
    } /* endif - pack the data value */
    else
    {
      *dataVal++ = MISSING_INT_DATA;
    }
    
    // Update the bitmap mask for the next round

    if (bitmap)
    {
      mapMask = mapMask >> 1;
      if (mapMask == 0)
      {
	mapMask = 128;
	++bitmap;
      }
    }
    
  } /* endfor - i */
   return( GRIB_SUCCESS );

}

int
BDS::_pkData(ui08 *packedDataPtr, ui08 *bitmap, unsigned int *dataInt,
	     int dataWidth, int nVals, int &num_data_bytes,
	     int startBit, int bitmapStartBit)
{
  static const string method_name = "BDS::_pkData()";
  
  unsigned int *dataVal = dataInt;

  //
  // Check offsets
  //
  if (startBit < 0 || bitmapStartBit < 0 ||
      startBit > 7 || bitmapStartBit > 7)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Bad start bit value -- must be 0-7" << endl;
    cerr << "start bit = " << startBit
	 << ", bitmap start bit = " << bitmapStartBit << endl;
    
    return GRIB_FAILURE;
  }
  
  //
  // Check data width
  //
  if (dataWidth > 31)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot pack data with data width greater than 31" << endl;
    cerr << "You have data width set to: " << dataWidth << endl;
    
    return GRIB_FAILURE;
  }
  
  // Loop through the values array and pack each value

  int mapMask = (int)pow(2.0, 7 - bitmapStartBit);
  int packed_bits_left = 8 - startBit;
  num_data_bytes = 1;
 
  //
  // use valid count to hold the number of valid point put in BDS
  // this value will be copoied into _numValues. so _numUnusedBits can be
  // correctly calculated after the BDS is packed
  // 
  int valid_count = 0;
  
  for (int i = 0; i < nVals; ++i)
  {
    int current_datum = *dataVal;
    
    // If we are using a bitmap and the current data value is missing
    // then we don't put anything in the packed output array, but we
    // still continue on to the end of the loop to increment everything.
    // If we aren't using a bitmap, always pack the value into the
    // output array.

    if (!bitmap ||
	(bitmap && (*bitmap & mapMask)))
    {
      // Pack the value into the output array

      ++valid_count;
      
      int bits_to_write = dataWidth;
      
      while (bits_to_write > 0)
      {
	if (bits_to_write > packed_bits_left)
	{
	  int packed_value =
	    (current_datum >> (bits_to_write - packed_bits_left)) & 255;
	  *packedDataPtr |= packed_value;
	  bits_to_write -= packed_bits_left;
	  current_datum &= (int)pow(2.0, bits_to_write) - 1;
	  ++packedDataPtr;
	  ++num_data_bytes;
	  packed_bits_left = 8;
	}
	else
	{
	  int packed_value =
	    (current_datum << (packed_bits_left - bits_to_write)) & 255;
	  *packedDataPtr |= packed_value;
	  packed_bits_left -= bits_to_write;
	  bits_to_write = 0;
	}
	
      }

    } /* endif - pack the data value */
    
    // Update the bitmap mask for the next round

    if (bitmap)
    {
      mapMask = mapMask >> 1;
      if (mapMask == 0)
      {
	mapMask = 128;
	++bitmap;
      }
    }
    
    // Move on to the next data value

    ++dataVal;
    
  } /* endfor - i */
  

  _numValues = valid_count;
      
  return GRIB_SUCCESS;
   
}

void
BDS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Binary Data Section:\n");
  fprintf(stream, "-------------------------\n");
  fprintf(stream, "BDS length %d\n", _nBytes);
  if (_gridPointData)
    fprintf(stream, "Grid point data\n");
  else
    fprintf(stream, "Spherical Harmonic Coefficients\n");

  if (_simplePacking)
    fprintf(stream, "Simple packing\n");
  else
    fprintf(stream, "Second order (Complex) Packing\n");

  if (_originalDataFloat)
    fprintf(stream, "Original data were floating point values\n");
  else
    fprintf(stream, "Original data were integer values\n");

  fprintf(stream, "Number of unused bits %d\n", _numUnusedBits);
  if (_additionalFlags) {
    fprintf(stream, "Additional flags at Octet 14\n");
    if (_singleDatum)
      fprintf(stream, "       Single datum at each grid point\n");
    else
      fprintf(stream, "       Matrix of values at each grid point\n");

    if (_noSecondaryBitmap)
      fprintf(stream, "       No secondary bit maps\n");
    else
      fprintf(stream, "       Secondary bit maps present\n");

    if (_secondOrderConstWidth)
      fprintf(stream, "       Second order values have constant width\n");
    else
      fprintf(stream, "       Second order values have different widths\n");

  }
  else
    fprintf(stream, "No Additional flags\n");

  fprintf(stream, "Binary scale factor %d\n", _binaryScale);
  fprintf(stream, "Actual scale %f\n", _scale);
  fprintf(stream, "Reference value (minimum) %f\n", _reference);
  fprintf(stream, "Number of bits per datum %d\n", _nBitsPerDatum);
  fprintf(stream, "Number of values %d\n", _numValues);
}


void BDS::printData(FILE *stream) const
{
  fprintf(stream, "Data values:\n");
  fprintf(stream, "============\n");
  
  int num_same_values = 1;
  double prev_value = _data[0];
  int num_values_printed = 0;
  
  for (int i = 1; i < _numValues; ++i)
  {
    if (_data[i] == prev_value)
    {
      ++num_same_values;
      continue;
    }
    
    if (prev_value == MISSING_DATA)
    {
      if (num_same_values == 1)
	fprintf(stream, " MISS");
      else
	fprintf(stream, " %d*MISS", num_same_values);
    }
    else
    {
      if (num_same_values == 1)
	fprintf(stream, " %.3f", prev_value);
      else
	fprintf(stream, " %d*%.3f", num_same_values, prev_value);
    }
    
    ++num_values_printed;
    
    if (num_values_printed % 7 == 0)
      fprintf(stream, "\n");

    num_same_values = 1;
    prev_value = _data[i];
    
  } /* endfor - i */

  // Print the final set of values

  if (prev_value == MISSING_DATA)
  {
    if (num_same_values == 1)
      fprintf(stream, " MISS");
    else
      fprintf(stream, " %d*MISS", num_same_values);
  }
  else
  {
    if (num_same_values == 1)
      fprintf(stream, " %.3f", prev_value);
    else
      fprintf(stream, " %d*%.3f", num_same_values, prev_value);
  }
    
}

void
BDS::print(ostream &stream) const
{
  stream << endl << endl;
  stream << "Grib Binary Data Section:" << endl;
  stream << "-------------------------" << endl;
  stream << "BDS length " <<  _nBytes << endl;
  if (_gridPointData)
    stream << "Grid point data" << endl;
  else
    stream << "Spherical Harmonic Coefficients" << endl;

  if (_simplePacking)
    stream << "Simple packing" << endl;
  else
    stream << "Second order (Complex) Packing" << endl;

  if (_originalDataFloat)
    stream << "Original data were floating point values" << endl;
  else
    stream << "Original data were integer values" << endl;

  stream << "Number of unused bits " <<  _numUnusedBits << endl;
  if (_additionalFlags) {
    stream << "Additional flags at Octet 14" << endl;
    if (_singleDatum)
      stream << "       Single datum at each grid point" << endl;
    else
      stream << "       Matrix of values at each grid point" << endl;

    if (_noSecondaryBitmap)
      stream << "       No secondary bit maps" << endl;
    else
      stream << "       Secondary bit maps present" << endl;

    if (_secondOrderConstWidth)
      stream << "       Second order values have constant width" << endl;
    else
      stream << "       Second order values have different widths" << endl;

  }
  else
    stream << "No Additional flags" << endl;

  stream << "Binary scale factor " << _binaryScale << endl;
  stream << "Actual scale " << _scale << endl;
  stream << "Reference value (minimum) " << _reference << endl;
  stream << "Number of bits per datum " << _nBitsPerDatum << endl;
  stream << "Number of values " << _numValues << endl;
}


void BDS::printData(ostream &stream) const
{
  stream << "Data values:" << endl;
  stream << "============" << endl;
  
  int num_same_values = 1;
  double prev_value = _data[0];
  int num_values_printed = 0;
  
  for (int i = 1; i < _numValues; ++i)
    {
    if (_data[i] == prev_value)
    {
      ++num_same_values;
      continue;
    }
    
    if (prev_value == MISSING_DATA)
    {
      if (num_same_values == 1)
	stream << " MISS";
      else
	stream << " " << num_same_values << "*MISS";
    }
    else
    {
      if (num_same_values == 1)
	//	stream << " %.3f", prev_value);
	stream << " " << prev_value;
      else
      //stream << " %d*%.3f", num_same_values, prev_value);
      stream << " " << num_same_values << "*" << prev_value;
    }
    
    ++num_values_printed;
    
    if (num_values_printed % 7 == 0)
      stream << endl;

    num_same_values = 1;
    prev_value = _data[i];
    
  } /* endfor - i */

  // Print the final set of values
  
  if (prev_value == MISSING_DATA)
  {
    if (num_same_values == 1)
      stream << " MISS";
    else
      stream << " " << num_same_values << "*MISS";
  }
  else
  {
    if (num_same_values == 1)
      stream << " " << prev_value;
    else
      stream << " " << num_same_values << "*" << prev_value;
  }
    
}

void
BDS::_calcDataWidthAndScale(const double min_data_value,
			    const double max_data_value,
			    const int decimal_scale,
			    const int max_bit_len)
{
  double scaled_min_value = min_data_value * pow(10.0, decimal_scale);
  double scaled_max_value = max_data_value * pow(10.0, decimal_scale);
  

  // Calculate the data width NCEP style.
  // NCEP style uses no binary scale and rounds numbers to the nearest
  // int.

  scaled_min_value = floor(scaled_min_value);
  double range = floor(scaled_max_value - scaled_min_value + 0.5);
  frexp(range, &_nBitsPerDatum);
  
  if (_nBitsPerDatum <= 16)
  {
    _binaryScale = 0;
  }
  else
  {
    _binaryScale = _nBitsPerDatum - 16;
    _nBitsPerDatum = 16;
  }
  
  _scale = pow(2.0, _binaryScale) / pow(10.0, decimal_scale);

  //
  // override _nBitsPerDatum with max_bit_len if requested
  //
  // continue to use _nBitsPerDatum if it is less than max_bit_len, to
  // keep records as small as possible
  //
  // Calculate the data width ECMWF style.
  // ECMWF style uses binary scale 
  //
  if((max_bit_len > 0) && (max_bit_len < _nBitsPerDatum)) {

    int eBits;
    range = scaled_max_value - scaled_min_value;

    if(range != 0.0) {
      frexp(range, &eBits);
      _binaryScale = eBits - max_bit_len;
      _nBitsPerDatum = max_bit_len;
      _scale = pow(2.0, -_binaryScale);
      range = floor((scaled_max_value - scaled_min_value)*_scale + 0.5);
      frexp(range, &eBits);
      if (eBits != _nBitsPerDatum) {
	_binaryScale++;
      }
      
    }
    else {
      _binaryScale = 0;
      _nBitsPerDatum = 0;
    }
    
    _scale = pow(2.0, _binaryScale) / pow(10.0, decimal_scale);
  }


}

