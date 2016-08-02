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
/////////////////////////////////////////////////////////
// GribFile
//
//
////////////////////////////////////////////////////////
#ifndef _GRIB_FILE_
#define _GRIB_FILE_

#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <toolsa/MemBuf.hh>

#include "GFSrecord.hh"
#include <grib/IdSec.hh>
#include "Params.hh"
using namespace std;


class GribFile {

public:

  typedef struct {
    int   Id;
    string LongName;
    string Name;
    string Units;
    int LevelType;
    int ZlevelNum;
    int RecLocation;
    int ForecastTime;
  } inventory_t;

  GribFile (Params &params);
  ~GribFile();
   

  inline bool eof() { return (_eofFound); }
  inline void reInit() { _eofFound = FALSE; _gribLen = 0; }
  inline void setEof(bool val) { _eofFound = val; }
  inline int getId() { return (_lastId); }
  inline void setId(int val) { _lastId = val; }
  inline int InventorySize() { return (_inventory.size()); }
  inline GFSrecord* getRecord (int location) { return (_inventory[location]); }

  list <Params::out_field_t> getFieldList();

  int addToInventory (FILE *, int);
  
  // to order all grib record vertical planes call sort after everything 
  // has be added to the inventory
  void sortInventory ();

  int findGribRec (const int id, const int levelType, const int levelNum);
  int getNumZlevels (const int id, const int levelType);
  void clearInventory ();
  void printInventory (FILE *);
  void printSummary ();

  Params::param_id_t cnvtParamId2enum(const int paramType);
  //Params::level_id_t cnvtLevelId2enum(const int levelType);

  string *cnvtLvl2String (const int);
  string uniqueParamNm (const string, const int);


// _vertType.print(stream); from PDS
private:
  //
  // Parameters
  //
  Params *_paramsPtr;

  // Create a list of all grib records
  // found in grib file in "de-gribbed"
  // state
  vector <GFSrecord *> _inventory;
  vector <GFSrecord *>::iterator _inv;

  inventory_t _gribParam;

  int _lastId;
  int _gribLen;
  int _Z_level;

  bool _eofFound;

  MemBuf *_gribRec;

};

bool compareFunc (GFSrecord *lhs, GFSrecord *rhs);

#endif
