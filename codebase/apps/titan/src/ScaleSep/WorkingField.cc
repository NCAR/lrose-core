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
/////////////////////////////////////////////////////////////
// WorkingField.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2014
//
///////////////////////////////////////////////////////////////

#include "WorkingField.hh"

const fl32 WorkingField::missingFl32 = -9999.0F;

///////////////////////////////////////////////////////
// constructor

WorkingField::WorkingField(const string &nameStr,
                           const string &longNameStr, 
                           const string &unitsStr,
                           int nx, int ny,
                           double minx, double miny,
                           double dx, double dy,
                           bool writeOut) :
        _name(nameStr),
        _longName(longNameStr),
        _units(unitsStr),
        _nx(nx),
        _ny(ny),
        _minx(minx),
        _miny(miny),
        _dx(dx),
        _dy(dy),
        _writeToFile(writeOut),
        _data(NULL)
        
{
  
  _nxy = _nx * _ny;
  _data = new fl32[_nxy];
  
  for (int ii = 0; ii < _nxy; ii++) {
    _data[ii] = missingFl32;
  }
  
}

/////////////////
// destructor

WorkingField::~WorkingField()

{
  
  if (_data) {
    delete[] _data;
  }
  
}

//////////////////////////
// set all values to zero

void WorkingField::setToZero()
{
  for (int ii = 0; ii < _nxy; ii++) {
    _data[ii] = 0.0;
  }
}

//////////////////////////
// set all values to missing

void WorkingField::setToMissing()
{
  for (int ii = 0; ii < _nxy; ii++) {
    _data[ii] = missingFl32;
  }
}

