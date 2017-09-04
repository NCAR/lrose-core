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
// IdSec - Indicator Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
////////////////////////////////////////////
#include <grib/constants.h>
#include <grib/IdSec.hh>
#include <iostream>

using namespace std;

const int IdSec::IS_NUM_BYTES_PACKED = 8;

IdSec::IdSec() :
  GribSection(),
  _numMsgBytes(0),
  _editionNum(1)
{
  _nBytes = IS_NUM_BYTES_PACKED;
}

int
IdSec::unpack( ui08 *idPtr)
{
   
   //
   // If the "GRIB" keyword is not there,
   // there is a problem
   //
   if( idPtr[0] != 'G' || idPtr[1] != 'R' || 
       idPtr[2] != 'I' || idPtr[3] != 'B' ) {
      cerr << "Didn't find GRIB keyword" << endl << flush;
      return( GRIB_FAILURE );
   }
   
   //
   // The total number of bytes in the entire message
   //
   _numMsgBytes = _upkUnsigned3( idPtr[4], idPtr[5], idPtr[6] );
   
   //
   // Edition number
   //
   _editionNum = (int) idPtr[7];

   if(_editionNum != 1) {
     cerr << "Edition number not 1." << endl << flush;
     return( GRIB_FAILURE );
  }

   return( GRIB_SUCCESS );
}

int IdSec::pack(ui08 *idPtr)
{
  // Pack the "GRIB" keyword into the buffer
  
  idPtr[0] = 'G';
  idPtr[1] = 'R';
  idPtr[2] = 'I';
  idPtr[3] = 'B';
   
  // The total number of bytes in the entire message

  _pkUnsigned3(_numMsgBytes, &(idPtr[4]));
   
  // Edition number

  idPtr[7] = (ui08)_editionNum;

  return GRIB_SUCCESS;
}

void
IdSec::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Indicator section:\n");
  fprintf(stream, "--------------------------\n");
  fprintf(stream, "File Length %d\n", _numMsgBytes);
  // even thought _editionNum is inherited, it is part of the
  // Indicator section
  fprintf(stream, "Edition %d\n\n", _editionNum);

}

void
IdSec::print(ostream &stream) const
{
  stream << endl << endl;
  stream << "Grib Indicator section:" << endl;
  stream << "--------------------------" << endl;
  stream << "File Length " <<  _numMsgBytes << endl;
  // even thought _editionNum is inherited, it is part of the
  // Indicator section
  stream << "Edition " << _editionNum << endl << endl;

}
