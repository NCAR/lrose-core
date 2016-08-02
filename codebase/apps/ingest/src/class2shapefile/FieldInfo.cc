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
////////////////////////////////////////////////////////////////////////////////
//
//  Field Information
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2003
//
//  $Id: FieldInfo.cc,v 1.3 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#include "Driver.hh"
using namespace std;

//
// Constructor for data fields
//
FieldInfo::FieldInfo( string className, string shapeName,
                      DBFFieldType type, int width, int precision,
                      string units, double missingVal )
{
   classFieldName  = className;
   shapeFieldName  = shapeName;
   dataType        = type;
   fieldWidth      = width;
   fieldPrecision  = precision;
   fieldUnits      = units;
   missingValue    = missingVal;

   init();
}

//
// Constructor for header fields
//
FieldInfo::FieldInfo( string shapeName,
                      DBFFieldType type, int width, int precision )
{
   shapeFieldName  = shapeName;
   dataType        = type;
   fieldWidth      = width;
   fieldPrecision  = precision;

   init();
}

void
FieldInfo::init()
{
   classColumn = -1;
   shapeColumn = -1;

   valueDbl = -999.0;
   valueInt = -999;
}
