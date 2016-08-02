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
///////////////////////////////////////////////
// DtraGribMgr
//////////////////////////////////////////////


#include "DtraGribMgr.hh"
#include "Quasi.hh"
using namespace std;

#define GRIB_INDICATOR	"GRIB"

// Last standard parameter ID in the NWS/NCEP table.
int DtraGribMgr::_kLastStandardParam = 127;

/*
 * parameter table for the Air Force Weather Agency (AFWA).
 * Fields are: param ID, short name, long name, units.
 * 0 should be the first param ID, and -1 marks the last.
 * Beyond that, the params don't have to be in any order,
 * but it's convenient if they are.
 */
const DtraGribMgr::DtraParmTable DtraGribMgr::_dtraParmTable[] = {
          { 0, "var0", "undefined", "none"},
          { Params::PRESS, "PRESS", "Pressure", "hPa"},
          { Params::PBLHGT, "PBLHGT", "Planetary Boundary Layer (PBL) Height", "m"},
          { Params::NLAT, "NLAT", "North latitude (-90 to +90)", "deg"},
          { Params::ELON, "ELON", "East longitude (0-360)", "deg"},
          { Params::PSTAR, "PSTAR", "Pstar (sfc pressure - model top pressure)", "cbar"},
          { Params::PRSPRT, "PRSPRT", "Pressure perturbation", "Pa"},
          { Params::PBLREG, "PBLREG", "PBL Regime", "category"},
          { Params::FRIVEL, "FRIVEL", "Friction velocity", "m/s"},
          { Params::MAPSCL, "MAPSCL", "Map scale factor", "non-dim"},
          { Params::TB, "TB", "Turbulence (Intensity)", "category"},
          { Params::LAT, "LAT", "Latitude", "deg"},
          { Params::LON, "LON", "Longitude", "deg"},
          { Params::TER_HT, "TER_HT", "Model terrain height", "m"},
          { Params::TOTACP, "TOTACP", "Total accumulated precipitation", "kg/m2"},
          { Params::LNDUSE, "LNDUSE", "Land-use", "category"},
          { Params::PCPTYPE, "PCPTYPE", "Precipitation type", "category"},
          { -1, "null", "null name", "none"}
};


DtraGribMgr::DtraGribMgr()
:AvnGribMgr()
{
}

DtraGribMgr::~DtraGribMgr() 
{
}


void DtraGribMgr::findFirstRecord(FILE *fp)
{
  string gribIndicator(GRIB_INDICATOR);
  bool gribFound = false;

  // look for the grib keyword
  char oneChar;
  while (!gribFound && !feof(fp)) {

    // look for the first character
    fread(&oneChar, sizeof(oneChar), 1, fp);
    if (feof(fp))
      continue;

    if (oneChar == gribIndicator[0]) {
      // look for the rest of the indicator
      int si = 1;
      while (!feof(fp) && si < (int) gribIndicator.length()) {
        fread(&oneChar, sizeof(oneChar), 1, fp);
        if (oneChar != gribIndicator[si])
          break;

        si++;
      }

      // did we find the full indicator?
      if (si >= (int) gribIndicator.length())
        gribFound = true;
    }
  }

  if (gribFound) {
    // back off, to the beginning of the indicator
    fseek(fp, -gribIndicator.length(), SEEK_CUR);
  }
}


string DtraGribMgr::getLongName()
{
  int paramId = _pds->getParameterId();
  if (paramId <= _kLastStandardParam) {
    return( AvnGribMgr::getLongName() );
  }

  // look for the param in the dtra table
  for (int pid = 0; _dtraParmTable[pid].paramId >= 0; pid++) {
    if (_dtraParmTable[pid].paramId == paramId) {
      return _dtraParmTable[pid].long_name;
    }
  }

  return _dtraParmTable[0].long_name;
}


string DtraGribMgr::getName()
{
  int paramId = _pds->getParameterId();
  if (paramId <= _kLastStandardParam) {
    return( AvnGribMgr::getName() );
  }

  // look for the param in the dtra table
  for (int pid = 0; _dtraParmTable[pid].paramId >= 0; pid++) {
    if (_dtraParmTable[pid].paramId == paramId) {
      return _dtraParmTable[pid].name;
    }
  }

  return _dtraParmTable[0].name;
}


string DtraGribMgr::getUnits()
{
  int paramId = _pds->getParameterId();
  if (paramId <= _kLastStandardParam) {
    return( AvnGribMgr::getUnits() );
  }

  // look for the param in the dtra table
  for (int pid = 0; _dtraParmTable[pid].paramId >= 0; pid++) {
    if (_dtraParmTable[pid].paramId == paramId) {
      return _dtraParmTable[pid].units;
    }
  }

  return _dtraParmTable[0].units;
}

