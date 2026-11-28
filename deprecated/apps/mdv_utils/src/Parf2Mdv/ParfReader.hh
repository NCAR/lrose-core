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



#ifndef PARFREADER_HH
#define PARFREADER_HH

#include "Params.hh"
#include <vector>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <string>

#define MAX_DATALINE_LEN 2048

using namespace std;

class ParfReader {

typedef struct {

  fl32 lat;

  fl32 lon;

  vector <fl32> fieldData;

} classPt;


public:
  //
  // constructor
  //
  ParfReader(char *filename, Params &parameters);

  ~ParfReader();

  int readFile();
  
  int getTime() { return _time; }

  int getNx() { return _nx;}

  int getNy() { return _ny;}

  fl32 getMinX() { return _minx; }

  fl32 getMinY() { return _miny; }

  fl32 getDx() { return _dx; }

  fl32 getDy() { return _dy; }

  fl32 getMinValue(int i ) {return _minVals[i]; }

  int getProjType() { return _projType; }

  fl32 getProjOriginLat() { return _projOriginLat; }

  fl32 getProjOriginLon() { return _projOriginLon; }

  fl32 getNumClassPts() { return _classPts.size(); }

  int getNumFields() { return  (int)_classPts[0]->fieldData.size(); }

  fl32 getClassPtLat(int i) { return _classPts[i]->lat; }

  fl32 getClassPtLon(int i) { return _classPts[i]->lon; }

  //
  // From the ith point get the jth field data value  
  //
  fl32 getFieldData(int i, int j) {  return _classPts[i]->fieldData[j]; }

  const char *getFieldName(int i) {return _fieldNames[i];}

private:

  int _parseGridInfo(char *line);

  int _findFieldMinVals();
  
  int  _getFieldNames(FILE *fptr);

  int _getFieldData(FILE *fptr);

  //
  // Members
  //
  Params _params;

  string _parfFileStr;

  //
  // Grid parameters
  //
  int _nx;

  int _ny;

  fl32 _minx;

  fl32 _miny;

  fl32 _dx;

  fl32 _dy;

  int _projType;

  fl32 _projOriginLat;

  fl32 _projOriginLon;

  //
  // more data
  //
  time_t _time;

  vector <fl32> _minVals;

  vector <classPt*> _classPts;

  vector <char*> _fieldNames;
};

#endif
