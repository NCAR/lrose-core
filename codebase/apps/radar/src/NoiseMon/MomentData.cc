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
// MomentData.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2022
//
///////////////////////////////////////////////////////////////

#include "MomentData.hh"
#include <cmath>

const double MomentData::missingVal = -9999;

// constructor

MomentData::MomentData()

{

  initialize();
  valid = true;

}

// Initialize to missing

void MomentData::initialize()

{
  
  height = missingVal;

  snr = missingVal;
  snrhc = missingVal;
  snrhx = missingVal;
  snrvc = missingVal;
  snrvx = missingVal;
  
  dbm = missingVal;
  dbmhc = missingVal;
  dbmhx = missingVal;
  dbmvc = missingVal;
  dbmvx = missingVal;

  dbz = missingVal;

  vel = missingVal;
  width = missingVal;

  zdrm = missingVal;
  ldrh = missingVal;
  ldrv = missingVal;

  phidp = missingVal;
  rhohv = missingVal;

}

// set values to zero

void MomentData::zeroOut()

{
  
  snr = 0;
  snrhc = 0;
  snrhx = 0;
  snrvc = 0;
  snrvx = 0;
  
  dbm = 0;
  dbmhc = 0;
  dbmhx = 0;
  dbmvc = 0;
  dbmvx = 0;

  dbz = 0;

  vel = 0;
  width = 0;

  zdrm = 0;
  ldrh = 0;
  ldrv = 0;

  phidp = 0;
  rhohv = 0;

}

// accumulate values

void MomentData::add(const MomentData &val)

{

  if (val.snr != missingVal) {
    snr += val.snr;
  }
  if (val.snrhc != missingVal) {
    snrhc += val.snrhc;
  }
  if (val.snrhx != missingVal) {
    snrhx += val.snrhx;
  }
  if (val.snrvc != missingVal) {
    snrvc += val.snrvc;
  }
  if (val.snrvx != missingVal) {
    snrvx += val.snrvx;
  }
  if (val.dbm != missingVal) {
    dbm += val.dbm;
  }
  if (val.dbmhc != missingVal) {
    dbmhc += val.dbmhc;
  }
  if (val.dbmhx != missingVal) {
    dbmhx += val.dbmhx;
  }
  if (val.dbmvc != missingVal) {
    dbmvc += val.dbmvc;
  }
  if (val.dbmvx != missingVal) {
    dbmvx += val.dbmvx;
  }
  if (val.dbz != missingVal) {
    dbz += val.dbz;
  }
  if (val.vel != missingVal) {
    vel += val.vel;
  }
  if (val.width != missingVal) {
    width += val.width;
  }
  if (val.zdrm != missingVal) {
    zdrm += val.zdrm;
  }
  if (val.ldrh != missingVal) {
    ldrh += val.ldrh;
  }
  if (val.ldrv != missingVal) {
    ldrv += val.ldrv;
  }
  if (val.phidp != missingVal) {
    phidp += val.phidp;
  }
  if (val.rhohv != missingVal) {
    rhohv += val.rhohv;
  }

}

// accumulate squared values

void MomentData::addSquared(const MomentData &val)

{
  
  if (val.snr != missingVal) {
    snr += val.snr * val.snr;
  }
  if (val.snrhc != missingVal) {
    snrhc += val.snrhc * val.snrhc;
  }
  if (val.snrhx != missingVal) {
    snrhx += val.snrhx * val.snrhx;
  }
  if (val.snrvc != missingVal) {
    snrvc += val.snrvc * val.snrvc;
  }
  if (val.snrvx != missingVal) {
    snrvx += val.snrvx * val.snrvx;
  }
  if (val.dbm != missingVal) {
    dbm += val.dbm * val.dbm;
  }
  if (val.dbmhc != missingVal) {
    dbmhc += val.dbmhc * val.dbmhc;
  }
  if (val.dbmhx != missingVal) {
    dbmhx += val.dbmhx * val.dbmhx;
  }
  if (val.dbmvc != missingVal) {
    dbmvc += val.dbmvc * val.dbmvc;
  }
  if (val.dbmvx != missingVal) {
    dbmvx += val.dbmvx * val.dbmvx;
  }
  if (val.dbz != missingVal) {
    dbz += val.dbz * val.dbz;
  }
  if (val.vel != missingVal) {
    vel += val.vel * val.vel;
  }
  if (val.width != missingVal) {
    width += val.width * val.width;
  }
  if (val.zdrm != missingVal) {
    zdrm += val.zdrm * val.zdrm;
  }
  if (val.ldrh != missingVal) {
    ldrh += val.ldrh * val.ldrh;
  }
  if (val.ldrv != missingVal) {
    ldrv += val.ldrv * val.ldrv;
  }
  if (val.phidp != missingVal) {
    phidp += val.phidp * val.phidp;
  }
  if (val.rhohv != missingVal) {
    rhohv += val.rhohv * val.rhohv;
  }

}

// compute mean and standard deviation,
// given sum and sum-squared objects

void MomentData::computeMeanSdev(double count,
                                 const MomentData &sum,
                                 const MomentData &sum2,
                                 MomentData &mean,
                                 MomentData &sdev)

{
  
  _computeMeanSdev(count, sum.snr, sum2.snr, mean.snr, sdev.snr);
  _computeMeanSdev(count, sum.snrhc, sum2.snrhc, mean.snrhc, sdev.snrhc);
  _computeMeanSdev(count, sum.snrhx, sum2.snrhx, mean.snrhx, sdev.snrhx);
  _computeMeanSdev(count, sum.snrvc, sum2.snrvc, mean.snrvc, sdev.snrvc);
  _computeMeanSdev(count, sum.snrvx, sum2.snrvx, mean.snrvx, sdev.snrvx);
  _computeMeanSdev(count, sum.dbm, sum2.dbm, mean.dbm, sdev.dbm);
  _computeMeanSdev(count, sum.dbmhc, sum2.dbmhc, mean.dbmhc, sdev.dbmhc);
  _computeMeanSdev(count, sum.dbmhx, sum2.dbmhx, mean.dbmhx, sdev.dbmhx);
  _computeMeanSdev(count, sum.dbmvc, sum2.dbmvc, mean.dbmvc, sdev.dbmvc);
  _computeMeanSdev(count, sum.dbmvx, sum2.dbmvx, mean.dbmvx, sdev.dbmvx);
  _computeMeanSdev(count, sum.dbz, sum2.dbz, mean.dbz, sdev.dbz);
  _computeMeanSdev(count, sum.vel, sum2.vel, mean.vel, sdev.vel);
  _computeMeanSdev(count, sum.width, sum2.width, mean.width, sdev.width);
  _computeMeanSdev(count, sum.zdrm, sum2.zdrm, mean.zdrm, sdev.zdrm);
  _computeMeanSdev(count, sum.ldrh, sum2.ldrh, mean.ldrh, sdev.ldrh);
  _computeMeanSdev(count, sum.ldrv, sum2.ldrv, mean.ldrv, sdev.ldrv);
  _computeMeanSdev(count, sum.phidp, sum2.phidp, mean.phidp, sdev.phidp);
  _computeMeanSdev(count, sum.rhohv, sum2.rhohv, mean.rhohv, sdev.rhohv);

}

// compute mean and sdev

void MomentData::_computeMeanSdev(double count,
                                  double sum,
                                  double sum2,
                                  double &mean,
                                  double &sdev)
  
{

  mean = missingVal;
  sdev = missingVal;

  if (count > 0) {
    mean = sum / count;
  }
  
  if (count > 2) {
    double var = (sum2 - (sum * sum) / count) / (count - 1.0);
    if (var >= 0.0) {
      sdev = sqrt(var);
    } else {
      sdev = 0.0;
    }
  }

}

// print

void MomentData::print(ostream &out) const

{

  out << "================ moment data =================" << endl;
  out << "snr: " << snr << endl;
  out << "snrhc: " << snrhc << endl;
  out << "snrhx: " << snrhx << endl;
  out << "snrvc: " << snrvc << endl;
  out << "snrvx: " << snrvx << endl;
  out << "dbm: " << dbm << endl;
  out << "dbmhc: " << dbmhc << endl;
  out << "dbmhx: " << dbmhx << endl;
  out << "dbmvc: " << dbmvc << endl;
  out << "dbmvx: " << dbmvx << endl;
  out << "dbz: " << dbz << endl;
  out << "vel: " << vel << endl;
  out << "width: " << width << endl;
  out << "zdrm: " << zdrm << endl;
  out << "ldrh: " << ldrh << endl;
  out << "ldrv: " << ldrv << endl;
  out << "phidp: " << phidp << endl;
  out << "rhohv: " << rhohv << endl;
  out << "==============================================" << endl;

}

