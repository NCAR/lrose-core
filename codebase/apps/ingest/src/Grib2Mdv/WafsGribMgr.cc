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
// WafsGribMgr
//////////////////////////////////////////////


#include "WafsGribMgr.hh"
#include "Quasi.hh"
using namespace std;

#define GRIB_INDICATOR	"GRIB"


WafsGribMgr::WafsGribMgr()
:GribMgr()
{
}

WafsGribMgr::~WafsGribMgr() 
{
}


void WafsGribMgr::findFirstRecord(FILE *fp)
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


void WafsGribMgr::mapQuasiToRegular()
{
  if (!_pds->gdsUsed())
    return;
  if (_gds->isRegular())
    return;

  // construct the parameters we need to call Quasi::qlin()

  // same number of rows in input and output
  int numberOfRows = _gds->getNy();

  // convert row lengths to row starts
  int *rowStarts = new int[numberOfRows + 1];
  rowStarts[0] = 0;

  vector<int> &ptsPerRow = _gds->getNumPtsPerRow();
  int startIndex = 0;
  int maxRowLength = 0;
  for (int ri = 0; ri < numberOfRows; ri++) {
    startIndex += ptsPerRow[ri];
    rowStarts[ri + 1] = startIndex;
    if (ptsPerRow[ri] > maxRowLength) {
      maxRowLength = ptsPerRow[ri];
    }
  }

  float *quasiData = _bds->getData();

  // allocate space for the regular grid
  int numRegularPoints = numberOfRows * maxRowLength;
  fl32 *regularData = new fl32[numRegularPoints];

  // do the convert
  Quasi::qlin(numberOfRows, rowStarts, quasiData, maxRowLength, numberOfRows, regularData);

  // find the new min and max values
  double minValue = regularData[0];
  double maxValue = regularData[0];
  for (int vi = 1; vi < numRegularPoints; vi++) {
    if (regularData[vi] < minValue)
      minValue = regularData[vi];
    if (regularData[vi] > maxValue)
      maxValue = regularData[vi];
  }

  // assign regular data back into the GDS and BDS
  _bds->setData(regularData, numRegularPoints, minValue, maxValue, _pds->getDecimalScale());
  _gds->setRegular(numberOfRows, maxRowLength);

  // clean up allocated storage
  delete [] rowStarts;
}

