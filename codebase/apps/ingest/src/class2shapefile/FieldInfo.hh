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
//  Field information
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2003
//
//  $Id: FieldInfo.hh,v 1.4 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _FIELDINFO_INC_
#define _FIELDINFO_INC_

#include <toolsa/DateTime.hh>
#include <shapelib/shapefil.h>
using namespace std;


class FieldInfo
{
public:
   //
   // Constructor for data fields
   //
   FieldInfo( string classFieldName, string shapeFieldName, 
              DBFFieldType dataType, int width, int precision, 
              string units, double missingValue );

   //
   // Constructor for header fields
   //
   FieldInfo( string shapeName,
              DBFFieldType dataType, int width, int precision );

  ~FieldInfo(){};

   //
   // Members set by user parameters
   // For simplicity, all members are public -- YIKES!
   //
   string         classFieldName;
   string         shapeFieldName;
   DBFFieldType   dataType;
   int            fieldWidth;
   int            fieldPrecision;
   string         fieldUnits;
   double         missingValue;

   //
   // Members derived by application
   //
   int        classColumn;
   int        shapeColumn;

   double     valueDbl;
   long       valueInt;
   string     valueStr;

private:

   void       init();

};

#endif
