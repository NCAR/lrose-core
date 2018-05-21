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
// DataTemp.cc : implementation for the DataTemp class 
//                  This is an abstract base class to allow
//                  objects to represent various Data
//                  templates
//                 
//
///////////////////////////////////////////////////////////

#include <float.h>
#include <grib2/DataTemp.hh>
#include <grib2/GDS.hh>
#include <grib2/DRS.hh>
#include <grib2/BMS.hh>
#include <grib2/DS.hh>

using namespace std;

namespace Grib2 {

/**************************************************************************
 * Constructor.
 */

DataTemp::DataTemp(Grib2Record::Grib2Sections_t sectionsPtr) 
{
  _sectionsPtr = sectionsPtr;
  _data = NULL;
  _lcpack = -1;
  _pdata = NULL;

}

/**************************************************************************
 * Destructor
 */

DataTemp::~DataTemp() 
{
  if(_data)
    delete [] _data;
}

void DataTemp::freeData()
{
  if(_data)
    delete [] _data;
  _data = NULL;
}

void DataTemp::_applyBitMapUnpack(fl32 *data) 
{
  si32 *bitMap = _sectionsPtr.bms->getBitMap();
  //
  // Apply the bitmap if one exists
  // Adding 0 value where bitmap = 0
  //
  if (bitMap == NULL)
    _data = data;
  else {
    si32 gridSz = _sectionsPtr.gds->getNumDataPoints();

    if(_data)
      delete [] _data;
    _data = new fl32[gridSz];
    
    int n = 0;
    for (int i = 0; i < gridSz; i++) {
      if (bitMap[i]) {
        _data[i] = data[n++];
      }
      else
        _data[i] = -9999.0;//FLT_MAX; //pow(2.0, (sizeof(fl32) * 8.0)) - 1;   //  MAX_INT;
    }
    delete [] data;
    if(n != _sectionsPtr.drs->getNumPackedDataPoints()) {
      cerr << "WARNING: Specified numPackeDataPoints in DRS != number of bitMap data points!" << endl;
    }
  }

}

fl32 *DataTemp::_applyBitMapPack(fl32 *data)
{
  si32 *bitMap = _sectionsPtr.bms->getBitMap();
  si32 gridSz = _sectionsPtr.gds->getNumDataPoints();
  //
  // Apply the bitmap if one exists
  // Removing data where bitmap = 0
  //
  fl32 *pdata;
  if(bitMap == NULL) {
    pdata = data;
    _sectionsPtr.drs->setNumPackedDataPoints(gridSz);
  } else {

    pdata = new fl32[gridSz];
    int ndpts = 0;
    for(int j = 0; j < gridSz; j++)
      if(bitMap[j]) {
	pdata[ndpts++] = data[j];
      }

    _sectionsPtr.drs->setNumPackedDataPoints(ndpts);
  }
  return pdata;
}

} // namespace Grib2

