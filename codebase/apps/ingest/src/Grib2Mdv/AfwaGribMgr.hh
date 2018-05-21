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
// AfwaGribMgr
//
////////////////////////////////////////////////////////
#ifndef _AFWA_GRIB_MGR
#define _AFWA_GRIB_MGR

#include "AvnGribMgr.hh"
using namespace std;

/**
 *  AfwaGribMgr
 *  This class converts AFWA-type grib files to MDV.
 */
class AfwaGribMgr : public AvnGribMgr {

public:

  AfwaGribMgr();
  virtual ~AfwaGribMgr();

  // we store the Afwa grib table in our own structure
  typedef struct {
    int paramId;
    const char *name;
    const char *long_name;
    const char *units;
  } AfwaParmTable;

  static const AfwaParmTable _afwaParmTable[];

  // last param index in the standard grib table
  static int _kLastStandardParam;

  // reference to ensemble map of code-to-name
  virtual Params::code_name_map_t *getEnsembleMap();
  virtual int getEnsembleMapSize();

  // override the NWS/NCEP usage
  virtual string getLongName();
  virtual string getName();
  virtual string getUnits();

};

#endif

