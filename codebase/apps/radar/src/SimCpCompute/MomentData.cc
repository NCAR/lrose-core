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
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2006
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
  
  corr_dbm = missingVal;
  corr_dbmhc = missingVal;
  corr_dbmhx = missingVal;
  corr_dbmvc = missingVal;
  corr_dbmvx = missingVal;

  phc = missingVal;
  phx = missingVal;
  pvc = missingVal;
  pvx = missingVal;

  dbz = missingVal;
  dbzhc = missingVal;
  dbzhx = missingVal;
  dbzvc = missingVal;
  dbzvx = missingVal;

  vel = missingVal;
  width = missingVal;

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

  corr_dbm = 0;
  corr_dbmhc = 0;
  corr_dbmhx = 0;
  corr_dbmvc = 0;
  corr_dbmvx = 0;

  dbz = 0;
  dbzhc = 0;
  dbzhx = 0;
  dbzvc = 0;
  dbzvx = 0;

  phc = 0;
  phx = 0;
  pvc = 0;
  pvx = 0;

  vel = 0;
  width = 0;

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

  if (val.corr_dbm != missingVal) {
    corr_dbm += val.corr_dbm;
  }
  if (val.corr_dbmhc != missingVal) {
    corr_dbmhc += val.corr_dbmhc;
  }
  if (val.corr_dbmhx != missingVal) {
    corr_dbmhx += val.corr_dbmhx;
  }
  if (val.corr_dbmvc != missingVal) {
    corr_dbmvc += val.corr_dbmvc;
  }
  if (val.corr_dbmvx != missingVal) {
    corr_dbmvx += val.corr_dbmvx;
  }

  if (val.dbz != missingVal) {
    dbz += val.dbz;
  }
  if (val.dbzhc != missingVal) {
    dbzhc += val.dbzhc;
  }
  if (val.dbzhx != missingVal) {
    dbzhx += val.dbzhx;
  }
  if (val.dbzvc != missingVal) {
    dbzvc += val.dbzvc;
  }
  if (val.dbzvx != missingVal) {
    dbzvx += val.dbzvx;
  }

  if (val.vel != missingVal) {
    vel += val.vel;
  }
  if (val.width != missingVal) {
    width += val.width;
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

  if (val.corr_dbm != missingVal) {
    corr_dbm += val.corr_dbm * val.corr_dbm;
  }
  if (val.corr_dbmhc != missingVal) {
    corr_dbmhc += val.corr_dbmhc * val.corr_dbmhc;
  }
  if (val.corr_dbmhx != missingVal) {
    corr_dbmhx += val.corr_dbmhx * val.corr_dbmhx;
  }
  if (val.corr_dbmvc != missingVal) {
    corr_dbmvc += val.corr_dbmvc * val.corr_dbmvc;
  }
  if (val.corr_dbmvx != missingVal) {
    corr_dbmvx += val.corr_dbmvx * val.corr_dbmvx;
  }

  if (val.dbz != missingVal) {
    dbz += val.dbz * val.dbz;
  }
  if (val.dbzhc != missingVal) {
    dbzhc += val.dbzhc * val.dbzhc;
  }
  if (val.dbzhx != missingVal) {
    dbzhx += val.dbzhx * val.dbzhx;
  }
  if (val.dbzvc != missingVal) {
    dbzvc += val.dbzvc * val.dbzvc;
  }
  if (val.dbzvx != missingVal) {
    dbzvx += val.dbzvx * val.dbzvx;
  }

  if (val.vel != missingVal) {
    vel += val.vel * val.vel;
  }
  if (val.width != missingVal) {
    width += val.width * val.width;
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

  _computeMeanSdev(count, sum.corr_dbm, sum2.corr_dbm,
                   mean.corr_dbm, sdev.corr_dbm);
  _computeMeanSdev(count, sum.corr_dbmhc, sum2.corr_dbmhc,
                   mean.corr_dbmhc, sdev.corr_dbmhc);
  _computeMeanSdev(count, sum.corr_dbmhx, sum2.corr_dbmhx,
                   mean.corr_dbmhx, sdev.corr_dbmhx);
  _computeMeanSdev(count, sum.corr_dbmvc, sum2.corr_dbmvc,
                   mean.corr_dbmvc, sdev.corr_dbmvc);
  _computeMeanSdev(count, sum.corr_dbmvx, sum2.corr_dbmvx,
                   mean.corr_dbmvx, sdev.corr_dbmvx);

  _computeMeanSdev(count, sum.dbz, sum2.dbz, mean.dbz, sdev.dbz);
  _computeMeanSdev(count, sum.dbzhc, sum2.dbzhc, mean.dbzhc, sdev.dbzhc);
  _computeMeanSdev(count, sum.dbzhx, sum2.dbzhx, mean.dbzhx, sdev.dbzhx);
  _computeMeanSdev(count, sum.dbzvc, sum2.dbzvc, mean.dbzvc, sdev.dbzvc);
  _computeMeanSdev(count, sum.dbzvx, sum2.dbzvx, mean.dbzvx, sdev.dbzvx);

  _computeMeanSdev(count, sum.vel, sum2.vel, mean.vel, sdev.vel);
  _computeMeanSdev(count, sum.width, sum2.width, mean.width, sdev.width);

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
