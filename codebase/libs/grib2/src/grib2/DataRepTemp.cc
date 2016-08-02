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
///////////////////////////////////////////////////////////
// DataRepTemp.cc : implementation for the DataRepTemp class 
//                  This is an abstract base class to allow
//                  objects to represent various Data
//                  Representation templates
//                 
//
///////////////////////////////////////////////////////////

#include <grib2/DataRepTemp.hh>

using namespace std;

namespace Grib2 {

/**************************************************************************
 * Constructor.
 */

DataRepTemp::DataRepTemp() 
{

}

DataRepTemp::DataRepTemp(Grib2Record::Grib2Sections_t sectionsPtr) 
{
  setSectionsPtr(sectionsPtr);
}

/**************************************************************************
 * Destructor
 */

DataRepTemp::~DataRepTemp() 
{
  // do nothing
}

void DataRepTemp::setSectionsPtr(Grib2Record::Grib2Sections_t sectionsPtr)
{
  _sectionsPtr = sectionsPtr;
}

} // namespace Grib2

