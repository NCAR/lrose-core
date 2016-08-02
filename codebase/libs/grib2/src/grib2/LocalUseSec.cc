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
// LocalUseSec - Indicator Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html)
//
// $Id: LocalUseSec.cc,v 1.7 2016/03/03 18:38:02 dixon Exp $
////////////////////////////////////////////
#include <grib2/LocalUseSec.hh>
#include <iostream>

using namespace std;

namespace Grib2 {

LocalUseSec::LocalUseSec() :
  GribSection()
{
  _sectionLen = 0;
  _sectionNum = 2;
  _localUse = NULL;
}

LocalUseSec::LocalUseSec(si32 dataSize, ui08 *localUseData) :
  GribSection()
{
  _sectionLen = dataSize + 5;
  _sectionNum = 2;

  _localUse = new ui08[dataSize];
  for(int j = 0; j < dataSize; j++)
    _localUse[j] = localUseData[j];
}

LocalUseSec::~LocalUseSec()
{
  if(_localUse != NULL)
    delete[] _localUse;
}

void LocalUseSec::setLocalUse(const si32 dataSize, ui08 *data)
{
  _sectionLen = dataSize + 5;

  if(_localUse != NULL)
    delete[] _localUse;

  _localUse = new ui08[dataSize];
  for(int j = 0; j < dataSize; j++)
    _localUse[j] = data[j];
}

int LocalUseSec::unpack( ui08 *idPtr)
{
   
   //
   // Section number
   //
   if ((int) idPtr[4] != 2) {

     // skip this section -> no local use section
     return( GRIB_SUCCESS );
   }
   else
     _sectionNum = (int) idPtr[4];

   //
   // Section Length
   //
   _sectionLen = _upkUnsigned4(idPtr[0], idPtr[1], idPtr[2], idPtr[3]);

   _localUse = new ui08[_sectionLen - 5];

   // Local use array
    for(int j = 0; j < _sectionLen - 5; j++)
      _localUse[j] = idPtr[5+j];

   return( GRIB_SUCCESS );
}

int LocalUseSec::pack(ui08 *idPtr)
{
  if(_sectionLen > 0 && _localUse != NULL) {
    // The total number of bytes in this section 
    _pkUnsigned4(_sectionLen, &(idPtr[0]));
    
    // Section Number
    idPtr[4] = (ui08)_sectionNum;
    
    // Local use array
    for(int j = 0; j < _sectionLen - 5; j++)
      idPtr[5+j] = _localUse[j];
  }
  return GRIB_SUCCESS;
}

void
LocalUseSec::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Local Use section:\n");
  fprintf(stream, "----------------------------------------------------\n");
  if (_sectionLen > 0) {
    fprintf(stream, "Section Length %d\n", _sectionLen);
    fprintf(stream, "Section Number %d\n\n", _sectionNum);
    //fprintf(stream, "Local Use %s\n\n", _localUse);
  }
  else
    fprintf(stream, "not present\n\n");

}

} // namespace Grib2

