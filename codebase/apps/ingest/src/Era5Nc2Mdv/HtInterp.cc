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
///////////////////////////////////////////////////
// HtInterp - interpolate MDV data from pressure
// levels to ht levels
//
// Mike Dixon, EOL, NCAR, Boulder, CO
// April 2014
///////////////////////////////////////////////////

#include <math.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <physics/IcaoStdAtmos.hh>

#include "HtInterp.hh"
#include "Params.hh"
using namespace std;

HtInterp::HtInterp(const Params &params) :
        _params(params)
{
}

HtInterp::~HtInterp()
{
}

//////////////////////////////////////////
// interp vlevels onto heights msl

int HtInterp::interpVlevelsToHeight(DsMdvx &mdvx)

{

  // get the field with geopotential height

  MdvxField *htFld = mdvx.getField("height");
  if (htFld == NULL) {
    cerr << "ERROR - HtInterp::_interpVlevelsToHeight()" << endl;
    cerr << " Cannot interpolate onto height levels" << endl;
    cerr << " Cannot find 'height' field in MDV" << endl;
    return -1;
  }

  const Mdvx::field_header_t &htFhdr = htFld->getFieldHeader();
  const Mdvx::vlevel_header_t &htVhdr = htFld->getVlevelHeader();

  // create height levels vector

  vector<double> htsOut;
  if (_params.compute_heights_from_pressure_levels) {
    // create the height vector by converting pressure to
    // ht using the standard atmosphere
    IcaoStdAtmos isa;
    for (int ii = 0; ii < htFhdr.nz; ii++) {
      double htKm = isa.pres2ht(htVhdr.level[ii]) / 1000.0;
      if (htKm >= _params.min_height_from_pressure_levels) {
        htsOut.push_back(htKm);
      }
    }
  } else {
    for (int ii = 0; ii < _params.height_levels_n; ii++) {
      htsOut.push_back(_params._height_levels[ii]);
    }
  }

  if (_params.debug) {
    cerr << "Converting to following height levels:" << endl;
    for (size_t ii = 0; ii < htsOut.size(); ii++) {
      cerr << " " << htsOut[ii];
    }
    cerr << endl;
  }

  // compute array of interp points

  vector<interp_pt_t> interpPts;
  _computeInterpPts(htsOut, htFld, interpPts);

  // interpolate each field

  for (size_t ifield = 0; ifield < mdvx.getNFields(); ifield++) {
    MdvxField *fld = mdvx.getField(ifield);
    _interpField(fld, interpPts, htsOut);
  }
  
  return 0;

}

////////////////////////////////////////////////////////
// Compute interpolation struct array

void HtInterp::_computeInterpPts(const vector<double> &htsOut,
                                 const MdvxField *htFld,
                                 vector<interp_pt_t> &interpPts)
  
{

  const Mdvx::field_header_t &htFhdr = htFld->getFieldHeader();

  int nzOut = (int) htsOut.size();
  int nzIn = htFhdr.nz;
  int ny = htFhdr.ny;
  int nx = htFhdr.nx;
  int nptsXy = ny * nx;
  int nptsOut = nzOut * nptsXy;
  interpPts.resize(nptsOut);

  // load up interp pt data
  
  fl32 *htsVol = (fl32 *) htFld->getVol();
  
  int yxIndex = 0;
  vector<double> hts;
  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++, yxIndex++) {

      // load column of geopotential hts
      
      hts.clear();
      int zIndex = 0;
      for (int iz = 0; iz < nzIn; iz++, zIndex += nptsXy) {
        double ht = htsVol[zIndex + yxIndex];
        hts.push_back(ht);
      } // iz
      
      // compute interpolation weights
      
      int zzIndex = 0;
      for (int jz = 0; jz < nzOut; jz++, zzIndex += nptsXy) {

        interp_pt_t &interpPt = interpPts[zzIndex + yxIndex];
        double ht = htsOut[jz];
        interpPt.ht = ht;

        if (ht <= hts[0]) {
          // we are below the lowest ght
          interpPt.indexLower = 0;
          interpPt.indexUpper = 0;
          interpPt.ghtLower = hts[0];
          interpPt.ghtUpper = hts[0];
          interpPt.wtLower = 0.0;
          interpPt.wtUpper = 1.0;
          continue;
        } 
        
        if (ht >= hts[nzIn-1]) {
          // we are above the highest ght
          interpPt.indexLower = nzIn-1;
          interpPt.indexUpper = nzIn-1;
          interpPt.ghtLower = hts[nzIn-1];
          interpPt.ghtUpper = hts[nzIn-1];
          interpPt.wtLower = 1.0;
          interpPt.wtUpper = 0.0;
          continue;
        }

        // we are within the ght limits

        for (int iz = 1; iz < nzIn; iz++, zIndex += nptsXy) {
          double ghtLower = hts[iz-1];
          double ghtUpper = hts[iz];
          if (ht >= ghtLower && ht <= ghtUpper) {
            interpPt.indexLower = iz-1;
            interpPt.indexUpper = iz;
            interpPt.ghtLower = ghtLower;
            interpPt.ghtUpper = ghtUpper;
            interpPt.wtLower = (ghtUpper - ht) / (ghtUpper - ghtLower);
            interpPt.wtUpper = 1.0 - interpPt.wtLower;
            // break;
          }
        } // iz

      } // jz
      
    } // ix
  } // iy

}

/////////////////////////////////////////
// interpolate a field onto height levels

void HtInterp::_interpField(MdvxField *fld, 
                            const vector<interp_pt_t> &interpPts,
                            const vector<double> &htsOut)
  
{

  Mdvx::field_header_t fhdr = fld->getFieldHeader();

  int nzOut = (int) htsOut.size();
  int nzIn = fhdr.nz;
  int ny = fhdr.ny;
  int nx = fhdr.nx;
  int nptsXy = ny * nx;
  int nptsOut = nzOut * nptsXy;

  // create array for interpolated data

  fl32 *dataOut = new fl32[nptsOut];

  // interpolate

  fl32 *dataIn = (fl32 *) fld->getVol();
  
  int xyIndex = 0;
  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++, xyIndex++) {
      
      int outZIndex = 0;
      for (int iz = 0; iz < nzOut; iz++, outZIndex += nptsXy) {

        const interp_pt_t &interpPt = interpPts[outZIndex + xyIndex];

        // check for bounds
        
        if (interpPt.indexLower < nzIn && interpPt.indexUpper < nzIn) {
          
          int inzLower = interpPt.indexLower * nptsXy + xyIndex;
          int inzUpper = interpPt.indexUpper * nptsXy + xyIndex;
          
          fl32 valLower = dataIn[inzLower];
          fl32 valUpper = dataIn[inzUpper];
          
          fl32 valInterp =
            valLower * interpPt.wtLower + valUpper * interpPt.wtUpper;
          
          dataOut[outZIndex + xyIndex] = valInterp;

        }

      } // iz

    } // ix
  } // iy

  // update the field data
  
  fld->setVolData(dataOut, nzOut, ny, nx, Mdvx::ENCODING_FLOAT32);

  // update the vertical levels and type

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  for (int iz = 0; iz < nzOut; iz++) {
    vhdr.level[iz] = htsOut[iz];
    vhdr.type[iz] = Mdvx::VERT_TYPE_Z;
  }
  fld->setVlevelHeader(vhdr);
  
  // get field header again because it has changed

  fhdr = fld->getFieldHeader();
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fld->setFieldHeader(fhdr);
  
  // free up

  delete[] dataOut;

}

