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

HtInterp::HtInterp(Params *params) :
        _paramsPtr(params)
{
}

HtInterp::~HtInterp()
{
}

//////////////////////////////////////////
// interp vlevels onto heights msl

int HtInterp::interpVlevelsToHeight(DsMdvx *mdvObj)

{

  // get the field with geopotential height

  MdvxField *ghtFld = mdvObj->getField(_paramsPtr->height_field_mdv_name);
  if (ghtFld == NULL) {
    cerr << "ERROR - HtInterp::_interpVlevelsToHeight()" << endl;
    cerr << " Cannot interpolate onto height levels" << endl;
    cerr << " Cannot find geopot height field in MDV: "
         << _paramsPtr->height_field_mdv_name << endl;
    return -1;
  }

  const Mdvx::field_header_t &ghtFhdr = ghtFld->getFieldHeader();
  const Mdvx::vlevel_header_t &ghtVhdr = ghtFld->getVlevelHeader();

  // check we have the correct type of vertical units

  if (_paramsPtr->compute_heights_from_pressure_levels &&
      ghtFhdr.vlevel_type != Mdvx::VERT_TYPE_PRESSURE) {
    cerr << "ERROR - HtInterp::_interpVlevelsToHeight()" << endl;
    cerr << "  Cannot interpolate onto height levels" << endl;
    cerr << "  Cannot compute height levels from vert levels" << endl;
    cerr << "  Vert levels must be of type PRESSURE" << endl;
    cerr << "  Vert levels are of type: "
         << Mdvx::vertType2Str(ghtFhdr.vlevel_type) << endl;
    return -1;
  }

  // add a pressure field to be interpolated

  _addPressureField(mdvObj, ghtFld);

  // create height levels vector

  vector<double> htsOut;
  if (_paramsPtr->compute_heights_from_pressure_levels) {
    // create the height vector by converting pressure to
    // ht using the standard atmosphere
    IcaoStdAtmos isa;
    for (int ii = 0; ii < ghtFhdr.nz; ii++) {
      double htKm = isa.pres2ht(ghtVhdr.level[ii]) / 1000.0;
      if (htKm >= _paramsPtr->min_height_from_pressure_levels) {
        htsOut.push_back(htKm);
      }
    }
  } else {
    for (int ii = 0; ii < _paramsPtr->height_levels_n; ii++) {
      htsOut.push_back(_paramsPtr->_height_levels[ii]);
    }
  }

  if (_paramsPtr->debug) {
    cerr << "Converting to following height levels:" << endl;
    for (size_t ii = 0; ii < htsOut.size(); ii++) {
      cerr << " " << htsOut[ii];
    }
    cerr << endl;
  }

  // compute array of interp points

  vector<interp_pt_t> interpPts;
  _computeInterpPts(htsOut, ghtFld, interpPts);

  // interpolate each field

  for (int ifield = 0; ifield < mdvObj->getNFields(); ifield++) {
    MdvxField *fld = mdvObj->getField(ifield);
    _interpField(fld, interpPts, htsOut);
  }
  
  return 0;

}

////////////////////////////////////////////////////////
// Add a pressure field which will then be interpolated

void HtInterp::_addPressureField(DsMdvx *mdvObj,
                                 const MdvxField *ghtFld)

{
  
  // copy the geopotential height field
  // rename

  MdvxField *presFld = new MdvxField(*ghtFld);
  presFld->convertType(Mdvx::ENCODING_FLOAT32,
                       Mdvx::COMPRESSION_NONE);
  presFld->setFieldName("Pressure");
  presFld->setFieldNameLong("Pressure");
  presFld->setUnits("hPa");

  // set the data to the vertical level

  const Mdvx::field_header_t &fhdr = presFld->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = presFld->getVlevelHeader();

  int nz = fhdr.nz;
  int ny = fhdr.ny;
  int nx = fhdr.nx;
  int nptsXy = ny * nx;

  fl32 *pp = (fl32 *) presFld->getVol();
  for (int iz = 0; iz < nz; iz++) {
    double pres = vhdr.level[iz];
    for (int ipt = 0; ipt < nptsXy; ipt++, pp++) {
      *pp = pres;
    }
  }

  mdvObj->addField(presFld);

}

////////////////////////////////////////////////////////
// Compute interpolation struct array

void HtInterp::_computeInterpPts(const vector<double> &htsOut,
                                 const MdvxField *ghtFld,
                                 vector<interp_pt_t> &interpPts)
  
{

  const Mdvx::field_header_t &ghtFhdr = ghtFld->getFieldHeader();

  int nzOut = (int) htsOut.size();
  int nzIn = ghtFhdr.nz;
  int ny = ghtFhdr.ny;
  int nx = ghtFhdr.nx;
  int nptsXy = ny * nx;
  int nptsOut = nzOut * nptsXy;
  interpPts.resize(nptsOut);
  
  // load up interp pt data
  
  fl32 *ghtsVol = (fl32 *) ghtFld->getVol();
  
  int yxIndex = 0;
  vector<double> ghts;
  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++, yxIndex++) {

      // compute column of geopotential hts
      
      ghts.clear();
      int zIndex = 0;
      for (int iz = 0; iz < nzIn; iz++, zIndex += nptsXy) {
        double ght = ghtsVol[zIndex + yxIndex];
        ghts.push_back(ght);
      } // iz
      
      // compute interpolation weights
      
      int zzIndex = 0;
      for (int jz = 0; jz < nzOut; jz++, zzIndex += nptsXy) {

        interp_pt_t &interpPt = interpPts[zzIndex + yxIndex];
        double ht = htsOut[jz];
        interpPt.ht = ht;

        if (ht <= ghts[0]) {
          // we are below the lowest ght
          interpPt.indexLower = 0;
          interpPt.indexUpper = 0;
          interpPt.ghtLower = ghts[0];
          interpPt.ghtUpper = ghts[0];
          interpPt.wtLower = 0.0;
          interpPt.wtUpper = 1.0;
          continue;
        } 
        
        if (ht >= ghts[nzIn-1]) {
          // we are above the highest ght
          interpPt.indexLower = nzIn-1;
          interpPt.indexUpper = nzIn-1;
          interpPt.ghtLower = ghts[nzIn-1];
          interpPt.ghtUpper = ghts[nzIn-1];
          interpPt.wtLower = 1.0;
          interpPt.wtUpper = 0.0;
          continue;
        }

        // we are within the ght limits

        for (int iz = 1; iz < nzIn; iz++, zIndex += nptsXy) {
          double ghtLower = ghts[iz-1];
          double ghtUpper = ghts[iz];
          if (ht >= ghtLower && ht <= ghtUpper) {
            interpPt.indexLower = iz-1;
            interpPt.indexUpper = iz;
            interpPt.ghtLower = ghtLower;
            interpPt.ghtUpper = ghtUpper;
            interpPt.wtLower = (ghtUpper - ht) / (ghtUpper - ghtLower);
            interpPt.wtUpper = 1.0 - interpPt.wtLower;
            break;
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
        
        int inzLower = interpPt.indexLower * nptsXy + xyIndex;
        int inzUpper = interpPt.indexUpper * nptsXy + xyIndex;

        fl32 valLower = dataIn[inzLower];
        fl32 valUpper = dataIn[inzUpper];

        fl32 valInterp =
          valLower * interpPt.wtLower + valUpper * interpPt.wtUpper;

        dataOut[outZIndex + xyIndex] = valInterp;

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

