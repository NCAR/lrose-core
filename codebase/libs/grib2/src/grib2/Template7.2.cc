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
// Template7_pt_2 Complex Packing Format
// Complex packing with spatial differencing format
//
// Jason Craig,  Aug 2006
//
//////////////////////////////////////////////////

#include <cmath>
#include <stdlib.h>

#include <grib2/Template7.2.hh>
#include <grib2/DS.hh>
#include <grib2/GDS.hh>
#include <grib2/DRS.hh>
#include <grib2/DataRepTemp.hh>
#include <grib2/Template5.2.hh>
#include <grib2/Template5.3.hh>

using namespace std;

namespace Grib2 {

Template7_pt_2::Template7_pt_2(Grib2Record::Grib2Sections_t sectionsPtr)
: DataTemp(sectionsPtr)
{

}


Template7_pt_2::~Template7_pt_2 () 
{
  if(_pdata)
    delete[] _pdata;
}

void Template7_pt_2::print(FILE *stream) const
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

int Template7_pt_2::pack (fl32 *dataPtr)
{
// SUBPROGRAM:    compack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-11-07
//
// ABSTRACT: This subroutine packs up a data field using a complex
//   packing algorithm as defined in the GRIB2 documention.  It
//   supports GRIB2 complex packing templates with or without
//   spatial differences (i.e. DRS 5.2 and 5.3).
//   It also fills in GRIB2 Data Representation Template 5.2 or 5.3 
//   with the appropriate values.
//
// PROGRAM HISTORY LOG:
// 2002-11-07  Gilbert


  // To be fully implemented implement the pack_gp() function
  // that is commented out below.
  cerr << "ERROR: Template7_pt_2::pack()" << endl;
  cerr << "Complex packing not fully implemented." << endl;
  return GRIB_FAILURE;

  fl32 *pdataPtr = _applyBitMapPack(dataPtr);
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();

  if(_pdata)
    delete[] _pdata;
  _pdata = new fl32[gridSz];

  si32 zero = 0;
  si32 simple_alg = 0;
  fl32 alog2 = 0.69314718;       //  ln(2.0)
  si32 one = 1;

  si32 nbitsd = 0, nbitsgwidth = 0, ngwidthref = 0;
  si32 nglenref = 0, nglenlast = 0, nbitsglen = 0;
  si32 ival1, ival2, nbitsgref;
  si32 iofst, ngroups;

  fl32 bscale = pow(2.0, -drsConstants.binaryScaleFactor);
  fl32 dscale = pow(10.0, drsConstants.decimalScaleFactor);

  //
  //  Find max and min values in the data
  //
  fl32 rmax = pdataPtr[0];
  fl32 rmin = pdataPtr[0];
  for (int j = 1; j < gridSz; j++) {
    if (pdataPtr[j] > rmax) rmax = pdataPtr[j];
    if (pdataPtr[j] < rmin) rmin = pdataPtr[j];
  }

  //
  //  If max and min values are not equal, pack up field.
  //  If they are equal, we have a constant field, and the reference
  //  value (rmin) is the value for each point in the field and
  //  set nbits to 0.
  //
  if (rmin != rmax) {
    iofst = 0;
    si32 *ifld = (si32 *)calloc(gridSz, sizeof(si32));
    si32 *gref = (si32 *)calloc(gridSz, sizeof(si32));
    si32 *gwidth = (si32 *)calloc(gridSz, sizeof(si32));
    si32 *glen = (si32 *)calloc(gridSz, sizeof(si32));
    //
    //  Scale original data
    //
    if (drsConstants.binaryScaleFactor == 0) {        //  No binary scaling
      si32 imin = (int)(rmin*dscale + .5);
      //imax = (int)(rmax*dscale + .5);
      rmin = (float)imin;
      for (int j = 0; j < gridSz; j++) 
	ifld[j] = (int)(pdataPtr[j]*dscale + .5)-imin;
    }
    else {                             //  Use binary scaling factor
      rmin = rmin*dscale;
      //rmax = rmax*dscale;
      for (int j = 0; j < gridSz; j++) 
	ifld[j] = (int)(((pdataPtr[j]*dscale)-rmin)*bscale + .5);
    }
    //
    //  Calculate Spatial differences, if using DRS Template 5.3
    //
    if (drsConstants.templateNumber == 3) {        // spatial differences
      Template5_pt_3 *template5_3 = (Template5_pt_3 *) _sectionsPtr.drs;

      if (template5_3->_spatialDifferenceOrder != 1 &&
	  template5_3->_spatialDifferenceOrder != 2) 
	template5_3->_spatialDifferenceOrder = 1;

      if (template5_3->_spatialDifferenceOrder == 1) {      // first order
	ival1 = ifld[0];
	for (int j = gridSz-1; j > 0; j--) 
	  ifld[j] = ifld[j]-ifld[j-1];
	ifld[0] = 0;
      }
      else if (template5_3->_spatialDifferenceOrder == 2) {      // second order
	ival1 = ifld[0];
	ival2 = ifld[1];
	for (int j = gridSz-1; j > 1; j--) 
	  ifld[j] = ifld[j] - (2*ifld[j-1]) + ifld[j-2];
	ifld[0] = 0;
	ifld[1] = 0;
      }
      //
      //  subtract min value from spatial diff field
      //
      si32 isd = template5_3->_spatialDifferenceOrder;
      si32 minsd = ifld[isd];
      for (int j = isd; j < gridSz; j++)  
	if ( ifld[j] < minsd ) 
	  minsd = ifld[j];
      for (int j = isd; j<gridSz; j++)  
	ifld[j] = ifld[j] - minsd;
      //
      //   find num of bits need to store minsd and add 1 extra bit
      //   to indicate sign
      //
      fl32 temp = log((double)(abs((double)minsd)+1))/alog2;
      nbitsd = (si32)ceil(temp)+1;
      //
      //   find num of bits need to store ifld[0] ( and ifld[1]
      //   if using 2nd order differencing )
      //
      si32 maxorig = ival1;
      if (template5_3->_spatialDifferenceOrder == 2 && ival2 > ival1) 
	maxorig = ival2;
      temp = log((double)(maxorig+1))/alog2;
      si32 nbitorig = (si32)ceil(temp)+1;
      if (nbitorig > nbitsd) 
	nbitsd = nbitorig;
      //   increase number of bits to even multiple of 8 ( octet )
      if ( (nbitsd%8) != 0) 
	nbitsd = nbitsd+(8-(nbitsd%8));
      //
      //  Store extra spatial differencing info into the packed
      //  data section.
      //
      if (nbitsd != 0) {
	//   pack first original value
	if (ival1 >= 0) {
	  DS::sbit((ui08 *)_pdata,&ival1,iofst,nbitsd);
	  iofst = iofst+nbitsd;
	}
	else {
	  DS::sbit((ui08 *)_pdata,&one,iofst,1);
	  iofst = iofst+1;
	  si32 itemp = abs(ival1);
	  DS::sbit((ui08 *)_pdata,&itemp,iofst,nbitsd-1);
	  iofst = iofst+nbitsd-1;
	}
	if (template5_3->_spatialDifferenceOrder == 2) {
	  //  pack second original value
	  if (ival2 >= 0) {
	    DS::sbit((ui08 *)_pdata,&ival2,iofst,nbitsd);
	    iofst = iofst+nbitsd;
	  }
	  else {
	    DS::sbit((ui08 *)_pdata,&one,iofst,1);
	    iofst = iofst+1;
	    si32 itemp = abs(ival2);
	    DS::sbit((ui08 *)_pdata,&itemp,iofst,nbitsd-1);
	    iofst = iofst+nbitsd-1;
	  }
	}
	//  pack overall min of spatial differences
	if (minsd >= 0) {
	  DS::sbit((ui08 *)_pdata,&minsd,iofst,nbitsd);
	  iofst = iofst+nbitsd;
	}
	else {
	  DS::sbit((ui08 *)_pdata,&one,iofst,1);
	  iofst = iofst+1;
	  si32 itemp = abs(minsd);
	  DS::sbit((ui08 *)_pdata,&itemp,iofst,nbitsd-1);
	  iofst = iofst+nbitsd-1;
	}
      }
      
    }     //  end of spatial diff section
    //
    //   Determine Groups to be used.
    //
    if ( simple_alg == 1 ) {
      //  set group length to 10;  calculate number of groups
      //  and length of last group
      si32 ngroups = gridSz/10;
      for (int j=0; j < ngroups;j++)
	glen[j] = 10;
      si32 itemp = gridSz%10;
      if (itemp != 0) {
	ngroups = ngroups+1;
	glen[ngroups-1] = itemp;
      }
    }
    else {
      // Use Dr. Glahn's algorithm for determining grouping.
      //
//      si32 kfildo = 6;
      si32 minpk = 10;
//      si32 inc = 1;
      si32 maxgrps = (gridSz/minpk)+1;
      si32 *jmin = (si32 *)calloc(maxgrps,sizeof(si32));
      si32 *jmax = (si32 *)calloc(maxgrps,sizeof(si32));
      si32 *lbit = (si32 *)calloc(maxgrps,sizeof(si32));
//      si32 missopt=0;
//      si32 ibit, jbit, kbit, lbitref, miss1, miss2, ier;
      si32 novref;
      //pack_gp(&kfildo,ifld,&gridSz,&missopt,&minpk,&inc,&miss1,&miss2,
      //      jmin,jmax,lbit,glen,&maxgrps,&ngroups,&ibit,&jbit,
      //      &kbit,&novref,&lbitref,&ier);

      for (int ng = 0; ng < ngroups; ng++) 
	glen[ng] = glen[ng]+novref;
      free(jmin);
      free(jmax);
      free(lbit);
    }
    //  
    //  For each group, find the group's reference value
    //  and the number of bits needed to hold the remaining values
    //
    int n = 0;
    for (int ng = 0; ng < ngroups; ng++) {
      //    find max and min values of group
      gref[ng] = ifld[n];
      si32 imax = ifld[n];
      int j = n+1;
      for (int lg = 1; lg < glen[ng]; lg++) {
	if (ifld[j] < gref[ng]) 
	  gref[ng] = ifld[j]; 
	if (ifld[j] > imax)
	  imax = ifld[j];
	j++;
      }
      //   calc num of bits needed to hold data
      if ( gref[ng] != imax ) {
	fl32 temp = log((double)(imax-gref[ng]+1))/alog2;
	gwidth[ng] = (si32)ceil(temp);
      }
      else 
	gwidth[ng] = 0;
      //   Subtract min from data
      j = n;
      for (int lg = 0; lg < glen[ng]; lg++) {
	ifld[j] = ifld[j]-gref[ng];
	j++;
      }
      //   increment pdataPtr array counter
      n = n+glen[ng];
    }
    //  
    //  Find max of the group references and calc num of bits needed 
    //  to pack each groups reference value, then
    //  pack up group reference values
    //
    si32 igmax = gref[0];
    for (int j = 1; j < ngroups; j++) 
      if (gref[j] > igmax) 
	igmax = gref[j];
    if (igmax != 0) {
      fl32 temp = log((double)(igmax+1))/alog2;
      nbitsgref = (si32)ceil(temp);
      DS::sbits((ui08 *)_pdata,gref,iofst,nbitsgref,0,ngroups);
      si32 itemp = nbitsgref*ngroups;
      iofst = iofst+itemp;
      //         Pad last octet with Zeros, if necessary,
      if ( (itemp%8) != 0) {
	si32 left = 8-(itemp%8);
	DS::sbit((ui08 *)_pdata,&zero,iofst,left);
	iofst=iofst+left;
      }
    }
    else
      nbitsgref=0;
    //
    //  Find max/min of the group widths and calc num of bits needed
    //  to pack each groups width value, then
    //  pack up group width values
    //
    si32 iwmax = gwidth[0];
    ngwidthref = gwidth[0];
    for (int j = 1; j < ngroups; j++) {
      if (gwidth[j] > iwmax) 
	iwmax=gwidth[j];
      if (gwidth[j] < ngwidthref) 
	ngwidthref=gwidth[j];
    }
    if (iwmax != ngwidthref) {
      fl32 temp = log((double)(iwmax-ngwidthref+1))/alog2;
      nbitsgwidth = (si32)ceil(temp);
      for (int i = 0; i < ngroups; i++) 
	gwidth[i] = gwidth[i]-ngwidthref;
      DS::sbits((ui08 *)_pdata,gwidth,iofst,nbitsgwidth,0,ngroups);
      si32 itemp = nbitsgwidth*ngroups;
      iofst = iofst+itemp;
      //         Pad last octet with Zeros, if necessary,
      if ( (itemp%8) != 0) {
	si32 left = 8-(itemp%8);
	DS::sbit((ui08 *)_pdata,&zero,iofst,left);
	iofst = iofst+left;
      }
    }
    else {
      nbitsgwidth = 0;
      for (int i = 0; i < ngroups; i++) 
	gwidth[i] = 0;
    }
    //
    //  Find max/min of the group lengths and calc num of bits needed
    //  to pack each groups length value, then
    //  pack up group length values
    //
    si32 ilmax = glen[0];
    nglenref = glen[0];
    for (int j = 1; j < ngroups-1; j++) {
      if (glen[j] > ilmax) 
	ilmax = glen[j];
      if (glen[j] < nglenref) 
	nglenref = glen[j];
    }
    nglenlast = glen[ngroups-1];
    if (ilmax != nglenref) {
      fl32 temp = log((double)(ilmax-nglenref+1))/alog2;
      nbitsglen = (si32)ceil(temp);
      for (int i = 0; i < ngroups-1; i++)  
	glen[i] = glen[i]-nglenref;
      DS::sbits((ui08 *)_pdata,glen,iofst,nbitsglen,0,ngroups);
      si32 itemp = nbitsglen*ngroups;
      iofst = iofst+itemp;
      //         Pad last octet with Zeros, if necessary,
      if ( (itemp%8) != 0) {
	si32 left = 8-(itemp%8);
	DS::sbit((ui08 *)_pdata,&zero,iofst,left);
	iofst = iofst+left;
      }
    }
    else {
      nbitsglen=0;
      for (int i = 0; i < ngroups; i++) glen[i]=0;
    }
    //
    //  For each group, pack data values
    //
    n = 0;
    for (int ng = 0; ng < ngroups; ng++) {
      si32 glength = glen[ng]+nglenref;
      if (ng == (ngroups-1) ) glength=nglenlast;
      si32 grpwidth = gwidth[ng] + ngwidthref;
      if ( grpwidth != 0 ) {
	DS::sbits((ui08 *)_pdata,ifld+n,iofst,grpwidth,0,glength);
	iofst = iofst+(grpwidth*glength);
      }
      n = n + glength;
    }
    //         Pad last octet with Zeros, if necessary,
    if ( (iofst%8) != 0) {
      si32 left = 8-(iofst%8);
      DS::sbit((ui08 *)_pdata,&zero,iofst,left);
      iofst = iofst+left;
    }
    _lcpack = iofst/8;
    //
    if ( ifld!=0 ) free(ifld);
    if ( gref!=0 ) free(gref);
    if ( gwidth!=0 ) free(gwidth);
    if ( glen!=0 ) free(glen);
  }
  else {          //   Constant field ( max = min )
    _lcpack = 0;
    nbitsgref = 0;
    ngroups = 0;
  }

  //
  //  Fill in ref value and number of bits in Template 5.2
  //
  drsConstants.referenceValue = rmin;
  drsConstants.numberOfBits = nbitsgref;
  //drsConstants.origFieldTypes = 0;              // original data were reals
  if(drsConstants.templateNumber == 2) {
    Template5_pt_2 *template5_2 = (Template5_pt_2 *) _sectionsPtr.drs;

    template5_2->_splittingMethod = 1;           // general group splitting
    template5_2->_missingType = 0;               // No internal missing values
    template5_2->_primaryMissingVal = 0;         // Primary missing value
    template5_2->_secondaryMissingVal = 0;       // secondary missing value
    template5_2->_numberGroups = ngroups;        // Number of groups
    template5_2->_groupWidths = ngwidthref;      // reference for group widths
    template5_2->_groupWidthsBits = nbitsgwidth; // num bits used for group widths
    template5_2->_groupLength = nglenref;        // Reference for group lengths
    template5_2->_lengthIncrement = 1;           // length increment for group lengths
    template5_2->_lengthOfLastGroup = nglenlast; // True length of last group
    template5_2->_groupLengthsBits = nbitsglen;  // num bits used for group lengths

  } else if(drsConstants.templateNumber == 3) {
    Template5_pt_3 *template5_3 = (Template5_pt_3 *) _sectionsPtr.drs;

    template5_3->_splittingMethod = 1;           // general group splitting
    template5_3->_missingType = 0;               // No internal missing values
    template5_3->_primaryMissingVal = 0;         // Primary missing value
    template5_3->_secondaryMissingVal = 0;       // secondary missing value
    template5_3->_numberGroups = ngroups;        // Number of groups
    template5_3->_groupWidths = ngwidthref;      // reference for group widths
    template5_3->_groupWidthsBits = nbitsgwidth; // num bits used for group widths
    template5_3->_groupLength = nglenref;        // Reference for group lengths
    template5_3->_lengthIncrement = 1;           // length increment for group lengths
    template5_3->_lengthOfLastGroup = nglenlast; // True length of last group
    template5_3->_groupLengthsBits = nbitsglen;  // num bits used for group lengths
    template5_3->_octetsRequired = nbitsd/8;     // num bits used for extra spatial
                                                 // differencing values
  }
  _sectionsPtr.drs->setDrsConstants(drsConstants);

  if(pdataPtr != dataPtr)
    delete [] pdataPtr;

  return GRIB_SUCCESS;
}

int Template7_pt_2::unpack (ui08 *dataPtr) 
{
  si32 gridSz = _sectionsPtr.drs->getNumPackedDataPoints();
  fl32 *outputData = new fl32[gridSz];
  DataRepTemp::data_representation_t drsConstants = _sectionsPtr.drs->getDrsConstants();
// SUBPROGRAM:    comunpack
//   PRGMMR: Gilbert          ORG: W/NP11    DATE: 2002-10-29
//
// ABSTRACT: This subroutine unpacks a data field that was packed using a
//   complex packing algorithm as defined in the GRIB2 documention,
//   using info from the GRIB2 Data Representation Template 5.2 or 5.3.
//   Supports GRIB2 complex packing templates with or without
//   spatial differences (i.e. DRTs 5.2 and 5.3).
//
// PROGRAM HISTORY LOG:
// 2002-10-29  Gilbert
// 2004-12-16  Gilbert  -  Added test ( provided by Arthur Taylor/MDL )
//                         to verify that group widths and lengths are
//                         consistent with section length.

  si32 nbitsd, iofst, j, k, l, n, non = 0;
  si32 isign, ival1, ival2, minsd, totBit, totLen;
  si32 nbitsgref, misType, nbitsgwidth, nbitsglen;
  si32 gwidths, spatialOrder, itemp;
  si32 lengthIncrement;
  si32 ngroups, glength, lengthLast;
  fl32 msng1, msng2;
  fl32 bscale, dscale, reference, rmiss1 = 0.0, rmiss2 = 0.0;
  
  bscale = pow(2.0, drsConstants.binaryScaleFactor);
  dscale = pow(10.0, -drsConstants.decimalScaleFactor);
  reference = drsConstants.referenceValue;
  
  nbitsgref = drsConstants.numberOfBits;
    
  if(drsConstants.templateNumber == 2) {
    Template5_pt_2 *template5_2 = (Template5_pt_2 *) _sectionsPtr.drs->getDrsTemplate();
      
    misType = template5_2->_missingType;
    ngroups = template5_2->_numberGroups;
    gwidths = template5_2->_groupWidths;
    nbitsgwidth = template5_2->_groupWidthsBits;
    glength = template5_2->_groupLength;
    lengthIncrement = template5_2->_lengthIncrement;
    lengthLast = template5_2->_lengthOfLastGroup;
    nbitsglen = template5_2->_groupLengthsBits;
    
    if ( misType == 1 ) {
      rmiss1 = template5_2->_primaryMissingVal;
      rmiss2 = 0;
    } else if ( misType == 2 ) {
      rmiss1 = template5_2->_primaryMissingVal;
      rmiss2 = template5_2->_secondaryMissingVal;
    }
    nbitsd = 0;
    spatialOrder = 0;
    
  } else if(drsConstants.templateNumber == 3) {
    Template5_pt_3 *template5_3 = (Template5_pt_3 *) _sectionsPtr.drs->getDrsTemplate();
    
    misType = template5_3->_missingType;
    ngroups = template5_3->_numberGroups;
    gwidths = template5_3->_groupWidths;
    nbitsgwidth = template5_3->_groupWidthsBits;
    glength = template5_3->_groupLength;
    lengthIncrement = template5_3->_lengthIncrement;
    lengthLast = template5_3->_lengthOfLastGroup;
    nbitsglen = template5_3->_groupLengthsBits;
    
    if ( misType == 1 ) {
      rmiss1 = template5_3->_primaryMissingVal;
      rmiss2 = 0;
    } else if ( misType == 2 ) {
      rmiss1 = template5_3->_primaryMissingVal;
      rmiss2 = template5_3->_secondaryMissingVal;
    }
    nbitsd = template5_3->_octetsRequired*8;
    spatialOrder = template5_3->_spatialDifferenceOrder;
    
  } else {
    cerr << "ERROR: Template7_pt_2::unpack()" << endl;
    cerr << "Complex packing only used for templates 2 and 3 not template " << drsConstants.templateNumber << endl;
    delete[] outputData;
    return GRIB_FAILURE;
  }
  
  
  //   Constant field
  if (ngroups == 0) {
    for (j=0; j< gridSz; j++) 
      outputData[j] = reference;
    _applyBitMapUnpack(outputData);
    return GRIB_SUCCESS;
  }
  

  si32 *ifld = new si32 [gridSz];
  si32 *gref = new si32 [ngroups];
  si32 *gwidth = new si32 [ngroups];
  si32 *ifldmiss = NULL;

  iofst=0;      
  
  // 
  //  Extract Spatial differencing values, if using DRS Template 5.3
  //
  if(drsConstants.templateNumber == 3) {
    if (nbitsd != 0) {
      DS::gbit(dataPtr, &isign, iofst, 1);
      iofst = iofst+1;
      DS::gbit(dataPtr, &ival1, iofst, nbitsd-1);
      iofst = iofst + nbitsd - 1;
      if (isign == 1) ival1 = -ival1;
      if (spatialOrder == 2) {
	DS::gbit(dataPtr, &isign, iofst, 1);
	iofst = iofst+1;
	DS::gbit(dataPtr, &ival2, iofst, nbitsd-1);
	iofst = iofst + nbitsd - 1;
	if (isign == 1) ival2 = -ival2;
      }
      DS::gbit(dataPtr, &isign, iofst, 1);
      iofst = iofst+1;
      DS::gbit(dataPtr,&minsd,iofst,nbitsd-1);
      iofst = iofst + nbitsd - 1;
      if (isign == 1) minsd = -minsd;
    }
    else {
      ival1=0;
      ival2=0;
      minsd=0;
    }
    //printf("SDu %ld %ld %ld %ld \n",ival1,ival2,minsd,nbitsd);
  }

  //
  //  Extract Each Group's reference value
  //
  //printf("SAG1: %ld %ld %ld \n",nbitsgref,ngroups,iofst);
  if (nbitsgref != 0) {
    DS::gbits(dataPtr, gref+0, iofst, nbitsgref, 0, ngroups);
    itemp = nbitsgref*ngroups;
    iofst = iofst+itemp;
    if (itemp%8 != 0) 
      iofst = iofst+(8-(itemp%8));
  }
  else {
    for (j=0; j<ngroups; j++)
      gref[j]=0;
  }

  //
  //  Extract Each Group's bit width
  //
  //printf("SAG2: %ld %ld %ld %ld \n",nbitsgwidth,ngroups,iofst,idrstmpl[10]);
  if (nbitsgwidth != 0) {
    DS::gbits(dataPtr, gwidth+0, iofst, nbitsgwidth, 0, ngroups);
    itemp = nbitsgwidth*ngroups;
    iofst = iofst+itemp;
    if (itemp%8 != 0) 
      iofst = iofst+(8-(itemp%8));
  }
  else {
    for (j=0; j<ngroups; j++)
      gwidth[j]=0;
  }
  
  for (j=0; j<ngroups; j++)
    gwidth[j] = gwidth[j]+gwidths;
  
  //
  //  Extract Each Group's length (number of values in each group)
  //
  si32 *glen = new si32 [ngroups];

  //printf("ALLOC glen: %d %x\n",(int)ngroups,glen);
  //printf("SAG3: %ld %ld %ld %ld %ld \n",nbitsglen,ngroups,iofst,idrstmpl[13],idrstmpl[12]);
  if (nbitsglen != 0) {
    DS::gbits(dataPtr, glen, iofst, nbitsglen, 0, ngroups);
    itemp = nbitsglen*ngroups;
    iofst = iofst+itemp;
    if (itemp%8 != 0) 
      iofst = iofst+(8-(itemp%8));
  }
  else {
    for (j=0; j<ngroups; j++)
      glen[j]=0;
  }
  for (j=0; j<ngroups; j++) 
    glen[j] = (glen[j] * lengthIncrement) + glength;
  glen[ngroups-1] = lengthLast;
  //
  //  Test to see if the group widths and lengths are consistent with number of
  //  values, and length of section 7.
  //
  totBit = 0;
  totLen = 0;
  for (j=0; j<ngroups; j++) {
    totBit += (gwidth[j]*glen[j]);
    totLen += glen[j];
  }

  if (totLen != gridSz || totBit / 8. > _sectionsPtr.ds->_sectionLen) {
    cerr << "ERROR: Template7_pt_2::unpack()" << endl;
    cerr << "Complex unpacking failed " << endl;
    delete[] ifld;
    delete[] glen;
    delete[] gref;
    delete[] gwidth;
    delete[] outputData;
    return GRIB_FAILURE;
  }

  //
  //  For each group, unpack data values
  //
  if ( misType == 0 ) {        // no missing values
    n=0;
    for (j=0; j<ngroups; j++) {
      if (gwidth[j] != 0) {
	DS::gbits(dataPtr, ifld+n, iofst, gwidth[j], 0, glen[j]);
	for (k=0; k<glen[j]; k++) {
	  ifld[n] = ifld[n]+gref[j];
	  n=n+1;
	}
      }
      else {
	for (l=n; l<n+glen[j]; l++)
	  ifld[l] = gref[j];
	n = n+glen[j];
      }
      iofst = iofst+(gwidth[j]*glen[j]);
    }
  }
  else if ( misType == 1 || misType == 2 ) {
    // missing values included
    ifldmiss = new si32[gridSz];
    
    //printf("ALLOC ifldmiss: %d %x\n",(int)gridSz,ifldmiss);
    //for (j=0;j < gridSz;j++) ifldmiss[j]=0;
    n=0;
    non=0;
    for (j=0; j<ngroups; j++) {
      //printf(" SAGNGP %d %d %d %d\n",j,gwidth[j],glen[j],gref[j]);
      if (gwidth[j] != 0) {
	msng1 = pow(2.0, gwidth[j])-1;
	msng2 = msng1-1;
	DS::gbits(dataPtr, ifld+n ,iofst, gwidth[j], 0, glen[j]);
	iofst = iofst+(gwidth[j]*glen[j]);
	for (k=0; k<glen[j]; k++) {
	  if (ifld[n] == msng1) {
	    ifldmiss[n]=1;
	    //ifld[n]=0;
	  }
	  else if (misType == 2 && ifld[n] == msng2) {
	    ifldmiss[n]=2;
	    //ifld[n]=0;
	  }
	  else {
	    ifldmiss[n]=0;
	    ifld[non++] = ifld[n]+gref[j];
	  }
	  n++;
	}
      }
      else {
	msng1 = pow(2.0, nbitsgref)-1;
	msng2 = msng1-1;
	if (gref[j] == msng1) {
	  for (l=n; l<n+glen[j]; l++)
	    ifldmiss[l] = 1;
	}
	else if (misType == 2 && gref[j] == msng2) {
	  for (l=n; l<n+glen[j]; l++) 
	    ifldmiss[l] = 2;
	}
	else {
	  for (l=n; l<n+glen[j]; l++) 
	    ifldmiss[l] = 0;
	  for (l=non; l<non+glen[j]; l++) 
	    ifld[l] = gref[j];
	  non += glen[j];
	}
	n=n+glen[j];
      }
    }
  }

  delete[] gref;
  delete[] gwidth;
  delete[] glen;
  //
  //  If using spatial differences, add overall min value, and
  //  sum up recursively
  //
  if (drsConstants.templateNumber == 3) {         // spatial differencing
    if (spatialOrder == 1) {      // first order
      ifld[0] = ival1;
      if ( misType == 0 )
	itemp = gridSz;        // no missing values
      else  
	itemp = non;
      for (n=1; n<itemp; n++) {
	ifld[n] = ifld[n]+minsd;
	ifld[n] = ifld[n]+ifld[n-1];
      }
    }
    else if (spatialOrder == 2) {    // second order
      ifld[0] = ival1;
      ifld[1] = ival2;
      if ( misType == 0 ) 
	itemp = gridSz;        // no missing values
      else  
	itemp = non;
      for (n=2; n<itemp; n++) {
	ifld[n] = ifld[n]+minsd;
	ifld[n] = ifld[n]+(2*ifld[n-1])-ifld[n-2];
      }
    }
  }
  //
  //  Scale data back to original form
  //
  //printf("SAGT: %f %f %f\n",reference,bscale,dscale);
  if ( misType == 0 ) {        // no missing values
    for (n=0;n < gridSz;n++) {
      outputData[n]=(( (fl32) ifld[n] * bscale) + reference) * dscale;
    }
  }
  else if ( misType == 1 || misType == 2 ) {
    // missing values included
    non=0;
    for (n=0; n < gridSz; n++) {
      if ( ifldmiss[n] == 0 ) {
	outputData[n] = (( (fl32) ifld[non++] * bscale) + reference) * dscale;
	//printf(" SAG %d %f %d %f %f %f\n",n,fld[n],ifld[non-1],bscale,reference,dscale);
      }
      else if ( ifldmiss[n] == 1 ) 
	outputData[n] = rmiss1;
      else if ( ifldmiss[n] == 2 ) 
	outputData[n] = rmiss2;
    }
    delete[] ifldmiss;
  }
  
  delete[] ifld;

  _applyBitMapUnpack(outputData);
  
  return GRIB_SUCCESS;
}

} // namespace Grib2

