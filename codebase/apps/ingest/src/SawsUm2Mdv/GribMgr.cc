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
// GribMgr.cc
//
// Carl Drews, Mike Dixon
// RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2007
//
// Copied from Grib2Mdv
// Wafs-specific functionality added in from WafsGribMgr.
//
///////////////////////////////////////////////////////////////

#include <typeinfo>
#include "GribMgr.hh"

#include <grib/GribVertType.hh>
#include <Mdv/Mdvx.hh>

using namespace std;

GribMgr::GribMgr() 
{
  _forecastTime = -1;
  _generateTime = -1;
}

GribMgr::~GribMgr() 
{
}

///////////////////////////////////////////////////////////////
// inventory a record

int GribMgr::inventoryRecord(ui08 *gribPtr)
{
  ui08 *sectionPtr = gribPtr;

  // Unpack the grib indicator section
  if(_id.unpack(sectionPtr) != 0) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return(-2);
  }
  sectionPtr += _id.getSize();

  // Unpack the product definition section
  _pds.setExpectedSize(1024);
  if(_pds.unpack(sectionPtr) != 0) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return(-1);
  }
  sectionPtr += _pds.getSize();

  // try to only set time info once
  if(_forecastTime < 0) {
    _forecastTime = _pds.getForecastTime();
    _generateTime = _pds.getGenerateTime();
  }

  // Unpack the grid description section if it is present.
  if(_pds.gdsUsed()) {
    if(_gds.unpack(sectionPtr) != 0) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      _gds.print(stdout);
      return(-1);
    }

    // Equidistant Cylindrical projection must be handled specially
    if (_gds.getProjType()  == GDS::EQUIDISTANT_CYL_PROJ_ID) {
      loadEquidistantCylindrical(sectionPtr);
    }

    sectionPtr += _gds.getSize();
  } else {
    cerr << "ERROR: No gds section" << endl << flush;
    return(-1);
  }

  return(0);
}

/////////////////////////////////////////////////////////////
// unpack a record

int GribMgr::unpackRecord(ui08 *gribPtr, int nx, int ny)
{
  ui08 *sectionPtr = gribPtr;
  ui08 *bitmapPtr  = NULL;

  // Unpack the grib indicator section

  if(_id.unpack(sectionPtr) != 0) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return(-2);
  }
  sectionPtr += _id.getSize();

  // Unpack the product definition section

  if(_pds.unpack(sectionPtr) != 0) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return(-1);
  }
  sectionPtr += _pds.getSize();

  //  cout << "the record is " << _pds.getLongName() << " for level "
  //  << _pds.getLevelVal() << endl << flush;

  // Unpack the grid description section if it is present.
  // If it isn't present, see if it is a predefined grid.
  // If it is, set the grid description section to look like
  // that predefined grid.  If it isn't, we don't know
  // what the grid looks like.  Error out in this case.

  if(_pds.gdsUsed()) {
    if(_gds.unpack(sectionPtr) != 0) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      _gds.print(stdout);
      return(-1);
    }

    // Equidistant Cylindrical projection must be handled specially
    if (_gds.getProjType()  == GDS::EQUIDISTANT_CYL_PROJ_ID) {
      loadEquidistantCylindrical(sectionPtr);
    }

    sectionPtr += _gds.getSize();
  }

  // If the bit map section is present, set it

  if(_pds.bmsUsed()) {
    if(_bms.unpack(sectionPtr) != 0) {
      cerr << "ERROR: Cannot unpack bms section" << endl << flush;
      return(-1);
    }
    bitmapPtr = _bms.getBitMap();
    sectionPtr += _bms.getSize();
  }

  // for irregular grids, the number of points must be tallied up by the gds
  int nPts = nx*ny;
  if (_pds.gdsUsed()) {
    nPts = _gds.getNumGridPoints();
  }

  // determine what kind of BDS grid we have
  bool regularGrid = true;
  if (_pds.gdsUsed()) {
    regularGrid = _gds.isRegular();
  }

  // Unpack the binary data section

  int bdsUnpackResult = 0;
  if (regularGrid) {
    bdsUnpackResult = _bds.unpack(sectionPtr, nPts, _pds.getDecimalScale(),
				   nx, bitmapPtr);
  } else {
    bdsUnpackResult = _bds.unpack(sectionPtr, nPts, _pds.getDecimalScale(),
				   _gds.getNumPtsPerRow(), bitmapPtr);
  }

  // did we get it right?
  if (bdsUnpackResult != 0) {
    cerr << "ERROR: Cannot unpack data" << endl << flush;
    return(-1);
  }

  return(0);
}


int GribMgr::loadEquidistantCylindrical(ui08 *sectionPtr)
{

  // unpack into GDS

  if (_gds.unpack(sectionPtr) != 0) {
    cerr << "ERROR: Cannot unpack EquidistantCylind gds section" << endl;
    _gds.print(stderr);
    return(-1);
  }
  
  return 0;

}

///////////////////////////////////////////
// change data orientation to S-N

void GribMgr::swapGridOrientationNS_2_SN()

{

  // check for correct projection
  if (_gds.getProjType() != GDS::EQUIDISTANT_CYL_PROJ_ID) {
    return;
  }

  EquidistantCylind *equiDist = (EquidistantCylind *) &_gds;
  
  // store grid data in a buffer
  fl32 *dataPtr = _bds.getData();
  MemBuf *gribData = new MemBuf();
  int numX = _gds.getNx();
  int numY = _gds.getNy();
  fl32 *bufPtr =
    (fl32 *)gribData->load((void *)dataPtr, numX * numY * sizeof (fl32));

  // swap while copying back into data
  int j = 0;
  for (int y = numY - 1; y >= 0; y--) {
    for (int x = 0;  x < numX; x++) {
      dataPtr[j] = bufPtr[(y * numX) + x];
      j++;
    }
  }

  // switch the beginning and end latitude to reflect reordering
  double oldStartLat = equiDist->getStartLat();
  equiDist->setStartLat(equiDist->getEndLat());
  equiDist->setEndLat(oldStartLat);

  gribData->free();

  // swap the projection
  Pjg projection(*getProjection());
  projection.setGridMins(projection.getMinx(), equiDist->getStartLat(), projection.getMinz());
  setProjection(projection);
}

////////////////////////////////////////
// find first record, after GRIB keyword

void GribMgr::findFirstRecord(FILE *fp)
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

/////////////////////////////////////////////
// map Quasi-rectangular grid to rectangular

void GribMgr::mapQuasiToRegular()
{
  if (!_pds.gdsUsed())
    return;
  if (_gds.isRegular())
    return;

  // construct the parameters we need to call qlin()

  // same number of rows in input and output
  int numberOfRows = _gds.getNy();

  // convert row lengths to row starts
  int *rowStarts = new int[numberOfRows + 1];
  rowStarts[0] = 0;

  vector<int> &ptsPerRow = _gds.getNumPtsPerRow();
  int startIndex = 0;
  int maxRowLength = 0;
  for (int ri = 0; ri < numberOfRows; ri++) {
    startIndex += ptsPerRow[ri];
    rowStarts[ri + 1] = startIndex;
    if (ptsPerRow[ri] > maxRowLength) {
      maxRowLength = ptsPerRow[ri];
    }
  }

  float *quasiData = _bds.getData();

  // allocate space for the regular grid
  int numRegularPoints = numberOfRows * maxRowLength;
  fl32 *regularData = new fl32[numRegularPoints];

  // do the convert
  _qlin(numberOfRows, rowStarts, quasiData, maxRowLength, numberOfRows, regularData);

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
  _bds.setData(regularData, numRegularPoints, minValue, maxValue, _pds.getDecimalScale());
  _gds.setRegular(numberOfRows, maxRowLength);

  // clean up allocated storage
  delete [] rowStarts;
}

////////////////////////////////////////////////////////////////////////////
// to regrid data from one equally-spaced grid to a different equally-spaced
// grid.

void GribMgr::_qlin(int nrows, int ix[], fl32 *idat,
		    int ni, int nj, fl32 *odat)
{
  // allocate temporary local storage
  double *c = new double[ni];
  fl32 *row2 = new fl32[ni];
  
  // precompute interpolation coefficients
  for (int i=0; i < ni; i++)
    c[i] = (double) (ni - i - 1) / (ni - 1);
  
  fl32 *outp = odat;
  for (int j=0; j < nj; j++) {
                                /* Does jth output row correspond to one of
                                   input rows, or is it between two?  */
    int inrow = j*(nrows-1)/(nj-1);		// input row to use
    if (inrow * (nj-1) == j*(nrows-1)) { /* one row */
                                /* interpolate inrow row to ni points */
      _linear(&idat[ix[inrow]],/* input row */
	      ix[inrow+1] - ix[inrow], /* number of points in row */
	      outp,        /* where to put new output row */
	      ni,          /* number of output values to compute */
	      c);          /* precomputed interpolation coefficients */
      outp += ni;
    } else {                /* between two rows */
                                /* interpolate rows inrow, inrow+1 to ni
                                   points  */
      _linear(&idat[ix[inrow]],/* first input row */
	      ix[inrow+1] - ix[inrow], /* # of points in row */
	      outp,        /* where to put new output row */
	      ni,          /* number of output values to compute */
	      c);          /* precomputed interpolation coefficients */
      _linear(&idat[ix[inrow+1]],/* second input row */
	      ix[inrow+2] - ix[inrow+1], /* # of points in row */
	      row2,        /* where to put new output row */
	      ni,          /* number of output values to compute */
	      c);          /* precomputed interpolation coefficients */
      for (int ii=0; ii < ni; ii++) {
	double c1 = 1.0 - (j*(nrows - 1.0)/(nj - 1.0) - inrow);
	double c2 = 1.0 - c1;
	*outp = c1 * *outp + c2*row2[ii];
	outp++;
      }
    }
  }
  delete [] row2;
  delete [] c;
}

void GribMgr::_linear(fl32 y[], int n, fl32 v[], int m, double c[])
{
  int j = 0;
  int k = 0;
  for (int i=0; i<m; i++) {
    v[i] = c[j]*y[k] + c[m-j-1]*y[k+1];
    j += n-1;
    int skip = (j-1)/(m-1);
    if (skip > 0) {
      k +=  skip;
      j = j - (m-1) * skip;
    }
  }
}
