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
// Template7_pt_0 Simple Packing Format
//
// Jason Craig,  Aug 2006
//
//////////////////////////////////////////////////

#include <cmath>
#include <stdlib.h>

#include <grib2/Template7.0.hh>
#include <grib2/DS.hh>
#include <grib2/GDS.hh>
#include <grib2/DRS.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/Template5.0.hh>

using namespace std;

namespace Grib2 {

Template7_pt_0::Template7_pt_0(Grib2Record::Grib2Sections_t sectionsPtr)
: DataTemp(sectionsPtr)
{

}


Template7_pt_0::~Template7_pt_0 () 
{
  if(_pdata)
    delete[] _pdata;
}

void Template7_pt_0::print(FILE *stream) const
{
  si32 gridSz = _sectionsPtr.gds->getNumDataPoints();
  fprintf(stream, "DS length: %d\n", gridSz);
  if(_data) {
    for (int i = 0; i < gridSz; i++) {
      fprintf(stream, "%f ", _data[i]);
    }
    fprintf(stream, "\n");
  }
}

int Template7_pt_0::pack (fl32 *dataPtr)
{
// SUBPROGRAM:    simpack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-11-06
//
// ABSTRACT: This subroutine packs up a data field using the simple
//   packing algorithm as defined in the GRIB2 documention.  It
//   also fills in GRIB2 Data Representation Template 5.0 with the
//   appropriate values.
//
// PROGRAM HISTORY LOG:
// 2002-11-06  Gilbert

  fl32 *pdataPtr = _applyBitMapPack(dataPtr);
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();

  if(_pdata)
    delete[] _pdata;
  _pdata = new fl32[gridSz];

  si32 zero = 0;
  fl32 alog2 = 0.69314718;  //  ln(2.0)

  si32 nbits,imin,imax,maxdif,left;    
  fl32 bscale = pow(2.0, -drsConstants.binaryScaleFactor);
  fl32 dscale = pow(10.0, drsConstants.decimalScaleFactor);

  if (drsConstants.numberOfBits <= 0 || drsConstants.numberOfBits > 31)
    nbits = 0;
  else
    nbits = drsConstants.numberOfBits;
  //
  //  Find max and min values in the data
  //
  fl32 rmax = pdataPtr[0];
  fl32 rmin = pdataPtr[0];
  for (int j = 1; j < gridSz; j++) {
    if (pdataPtr[j] > rmax) rmax = pdataPtr[j];
    if (pdataPtr[j] < rmin) rmin = pdataPtr[j];
  }
     
  si32 *ifld = (si32 *)calloc(gridSz,sizeof(si32));
  //
  //  If max and min values are not equal, pack up field.
  //  If they are equal, we have a constant field, and the reference
  //  value (rmin) is the value for each point in the field and
  //  set nbits to 0.
  //
  if (rmin != rmax) {
    //
    //  Determine which algorithm to use based on user-supplied 
    //  binary scale factor and number of bits.
    //
    if (nbits == 0 && drsConstants.binaryScaleFactor == 0) {
      //
      //  No binary scaling and calculate minumum number of 
      //  bits in which the data will fit.
      //
      imin = (int)(rmin*dscale + .5);
      imax = (int)(rmax*dscale + .5);
      maxdif = imax-imin;
      fl32 temp = log((double)(maxdif+1))/alog2;
      nbits = (int)ceil(temp);
      rmin = (float)imin;
      //   scale data
      for(int j = 0; j < gridSz; j++)
	ifld[j] = (int)(pdataPtr[j]*dscale + .5)-imin;
    }
    else if (nbits != 0 && drsConstants.binaryScaleFactor == 0) {
      //
      //  Use minimum number of bits specified by user and
      //  adjust binary scaling factor to accomodate data.
      //
      rmin = rmin*dscale;
      rmax = rmax*dscale;
      double maxnum = pow(2.0,nbits)-1;
      fl32 temp = log(maxnum/(rmax-rmin))/alog2;
      drsConstants.binaryScaleFactor = (int)ceil(-1.0*temp);
      bscale = pow(2.0,-drsConstants.binaryScaleFactor);
      //   scale data
      for (int j = 0; j < gridSz; j++)
	ifld[j] = (int)(((pdataPtr[j]*dscale)-rmin)*bscale + .5);
    }
    else if (nbits == 0 && drsConstants.binaryScaleFactor != 0) {
      //
      //  Use binary scaling factor and calculate minumum number of 
      //  bits in which the data will fit.
      //
      rmin = rmin*dscale;
      rmax = rmax*dscale;
      maxdif = (int)((rmax-rmin)*bscale + .5);
      fl32 temp = log((double)(maxdif+1))/alog2;
      nbits = (int)ceil(temp);
      //   scale data
      for (int j = 0; j < gridSz; j++)
	ifld[j] = (int)(((pdataPtr[j]*dscale)-rmin)*bscale  + .5);
    }
    else if (nbits != 0 && drsConstants.binaryScaleFactor != 0) {
      //
      //  Use binary scaling factor and use minumum number of 
      //  bits specified by user.   Dangerous - may loose
      //  information if binary scale factor and nbits not set
      //  properly by user.
      //
      rmin = rmin*dscale;
      //   scale data
      for (int j = 0;j<gridSz;j++)
	ifld[j] = (int)(((pdataPtr[j]*dscale)-rmin)*bscale + .5);
    }
    //
    //  Pack data, Pad last octet with Zeros, if necessary,
    //  and calculate the length of the packed data in bytes
    //
    DS::sbits((ui08 *)_pdata,ifld+0,0,nbits,0,gridSz);
    si32 nbittot = nbits*gridSz;
    left = 8-(nbittot%8);
    if (left != 8) {
      DS::sbit((ui08 *)_pdata,&zero,nbittot,left);   // Pad with zeros to fill Octet
      nbittot = nbittot+left;
    }
    _lcpack = nbittot/8;
  }
  else {
    nbits = 0;
    _lcpack = 0;
  }

  //
  //  Fill in ref value and number of bits in Template 5.0
  //
  drsConstants.referenceValue = rmin;
  drsConstants.numberOfBits = nbits;
  //drsConstants.origFieldTypes = 0;      // original data were reals

  _sectionsPtr.drs->setDrsConstants(drsConstants);

  free(ifld);
  if(pdataPtr != dataPtr)
    delete [] pdataPtr;

  return GRIB_SUCCESS;
}


int Template7_pt_0::unpack (ui08 *dataPtr) 
{
// SUBPROGRAM:    simunpack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-10-29
//
// ABSTRACT: This subroutine unpacks a data field that was packed using a
//   simple packing algorithm as defined in the GRIB2 documention,
//   using info from the GRIB2 Data Representation Template 5.0.
//
// PROGRAM HISTORY LOG:
// 2002-10-29  Gilbert
//
//   INPUT ARGUMENT LIST:
//     dataPtr     - pointer to the packed data field.
//
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();
  fl32 *outputData = new fl32[gridSz];
  si32  j;
  fl32 bscale, dscale, reference;

  bscale = pow(2.0, drsConstants.binaryScaleFactor);
  dscale = pow(10.0, -drsConstants.decimalScaleFactor);
  reference = drsConstants.referenceValue;
 
  si32 *tmp_data = new si32 [gridSz];

//
//  if drsConstants.numberOfBits equals 0, we have a constant field where the reference value
//  is the data value at each gridpoint
//
  if (drsConstants.numberOfBits != 0) {
    DS::gbits (dataPtr, tmp_data, 0, drsConstants.numberOfBits, 0, gridSz);
    for (j = 0; j < gridSz; j++) {
      outputData[j] = (((fl32) tmp_data[j] * bscale) 
		       + reference) * dscale;
    }
  }
  else {
    for (j=0; j < gridSz; j++) {
      outputData[j] = reference;
    }
  }
  delete [] tmp_data;
  
  _applyBitMapUnpack(outputData);
  
  return GRIB_SUCCESS;
}

} // namespace Grib2

