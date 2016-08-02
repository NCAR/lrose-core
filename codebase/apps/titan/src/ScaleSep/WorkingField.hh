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

#ifndef WorkingField_HH
#define WorkingField_HH

#include <dataport/port_types.h>
#include <string>

using namespace std;

class WorkingField {
  
public:

  // constructor

  WorkingField(const string &nameStr,
               const string &longNameStr, 
               const string &unitsStr,
               int nx, int ny,
               double minx, double miny,
               double dx, double dy,
               bool writeOut);

  // destructor
  
  ~WorkingField();

  // set all values to 0

  void setToZero();

  // set all values to missing

  void setToMissing();

  // get methods

  const string &getName() const { return _name; }
  const string &getLongName() const { return _longName; }
  const string &getUnits() const { return _units; }
  int getNx() const { return _nx; }
  int getNy() const { return _ny; }
  int getNxy() const { return _nxy; }
  double getDx() const { return _dx; }
  double getDy() const { return _dy; }
  double getMinx() const { return _minx; }
  double getMiny() const { return _miny; }
  bool getWriteToFile() const { return _writeToFile; }
  const fl32 *getData() const { return _data; }
  fl32 *getData() { return _data; }

  // missing value

  static const fl32 missingFl32;

private:

  string _name;
  string _longName;
  string _units;

  int _nx, _ny;
  int _nxy;
  double _minx, _miny;
  double _dx, _dy;

  bool _writeToFile;

  fl32 *_data;

};

#endif
