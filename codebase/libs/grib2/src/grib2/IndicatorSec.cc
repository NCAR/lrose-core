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
////////////////////////////////////////////
// IndicatorSec - Indicator Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
// $Id: IndicatorSec.cc,v 1.10 2018/10/13 22:29:11 dixon Exp $
////////////////////////////////////////////
#include <grib2/IndicatorSec.hh>
#include <iostream>

using namespace std;

namespace Grib2 {

IndicatorSec::IndicatorSec() :
  GribSection()
{
  _numMsgBytes = 0;
  _sectionNum = 0;
  _sectionLen = 16;
}

int IndicatorSec::unpack( ui08 *idPtr)
{
   
   //
   // If the "GRIB" keyword is not there,
   // there is a problem
   //
   if( idPtr[0] != 'G' || idPtr[1] != 'R' || 
       idPtr[2] != 'I' || idPtr[3] != 'B' ) {
     cerr << "ERROR: IndicatorSec::unpack()" << endl;
      cerr << "Didn't find GRIB keyword" << endl << flush;
      return( GRIB_FAILURE );
   }
   
   //
   // Discipline - GRIB Master Table Number
   //
   _disciplineNum = (int) idPtr[6];

   //
   // Edition number
   //
   _editionNum = (int) idPtr[7];

   //
   // The total number of bytes in the entire message
   //
   //_numMsgBytes = _upkUnsigned3 (idPtr[4], idPtr[5], idPtr[6]);
   _numMsgBytes = 
      _upkUnsigned8 (idPtr[8], idPtr[9], idPtr[10], idPtr[11], idPtr[12], idPtr[13], idPtr[14], idPtr[15]);
   
   return( GRIB_SUCCESS );
}

int IndicatorSec::pack(ui08 *idPtr)
{
  // Pack the "GRIB" keyword into the buffer
  
  idPtr[0] = 'G';
  idPtr[1] = 'R';
  idPtr[2] = 'I';
  idPtr[3] = 'B';
   
  // The total number of bytes in the entire message
  _pkUnsigned4(0, &(idPtr[8]));
  _pkUnsigned4(_numMsgBytes, &(idPtr[12]));
   
  // Discipline - GRIB Master Table Number
  idPtr[6] = (ui08)_disciplineNum;

  // Edition number
  idPtr[7] = (ui08)_editionNum;

  return GRIB_SUCCESS;
}

void
IndicatorSec::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Indicator section:\n");
  fprintf(stream, "----------------------------------------------------\n");
  fprintf(stream, "Message Length %lu\n", _numMsgBytes);
  fprintf(stream, "Discipline Number %d\n", _disciplineNum);
  fprintf(stream, "Edition %d\n\n", _editionNum);

}

} // namespace Grib2

