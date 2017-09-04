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
/////////////////////////////////////////////
// ES - End Section
//
////////////////////////////////////////////

#include <iostream>

#include <grib/constants.h>
#include <grib/ES.hh>

using namespace std;

const int ES::ES_NUM_BYTES_PACKED = 4;

ES::ES() :
  GribSection()
{
  _nBytes = ES_NUM_BYTES_PACKED;
}

ES::~ES()
{
}

int ES::unpack(ui08 *esPtr)
{
  // If the "7777" keyword is not there, there is a problem

  if (esPtr[0] != '7' || esPtr[1] != '7' || 
      esPtr[2] != '7' || esPtr[3] != '7')
  {
    cerr << "Didn't find 7777 keyword" << endl;
    return GRIB_FAILURE;
  }
   
  return GRIB_SUCCESS;
}

int ES::pack(ui08 *esPtr)
{
  // Pack the "7777" keyword

  esPtr[0] = '7';
  esPtr[1] = '7';
  esPtr[2] = '7';
  esPtr[3] = '7';

   return GRIB_SUCCESS;
}

void ES::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib End Section:\n");
  fprintf(stream, "--------------------------\n");
  fprintf(stream, "7777\n");
}

void ES::print(ostream &stream) const
{
  stream << endl << endl;
  stream << "Grib End Section" << endl;
  stream << "--------------------------" << endl;
  stream << "7777" << endl;
}
