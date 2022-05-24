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
///////////////////////////////////////////////////////////////
// MomentsSun.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2006
//
///////////////////////////////////////////////////////////////

#define __in_moments_cc__
#include "MomentsSun.hh"

#include <toolsa/DateTime.hh>
#include <cmath>
using namespace std;

// Constructor

MomentsSun::MomentsSun()
  
{

  init();

}

// destructor

MomentsSun::~MomentsSun()

{

}

// Set values to missing

void MomentsSun::init()
  
{

  time = missing;
  prt = missing;
  az = missing;
  el = missing;

  offsetAz = missing;
  offsetEl = missing;

  nn = 0.0;

  powerHc = missing;
  powerHx = missing;
  powerVx = missing;
  powerVc = missing;

  dbmHc = missing;
  dbmHx = missing;
  dbmVx = missing;
  dbmVc = missing;

  dbm = missing;
  dbBelowPeak = missing;

  sumRvvhh0Hc.re = 0.0;
  sumRvvhh0Hc.im = 0.0;
  sumRvvhh0Vc.re = 0.0;
  sumRvvhh0Vc.im = 0.0;
  sumRvvhh0.re = 0.0;
  sumRvvhh0.im = 0.0;

  Rvvhh0Hc.re = missing;
  Rvvhh0Hc.im = missing;
  Rvvhh0Vc.re = missing;
  Rvvhh0Vc.im = missing;
  Rvvhh0.re = missing;
  Rvvhh0.im = missing;

  corrMagHc = missing;
  corrMagVc = missing;
  corrMag = missing;

  corr00Hc = missing;
  corr00Vc = missing;
  corr00 = missing;

  arg00Hc = missing;
  arg00Vc = missing;
  arg00 = missing;
  
  ratioDbmHcHx = missing;
  ratioDbmVcVx = missing;

  ratioDbmVxHc = missing;
  ratioDbmVcHx = missing;

  ratioDbmVcHc = missing;
  ratioDbmVxHx = missing;

  S1S2 = missing;
  SS = missing;

  zdr = missing;
  phidp = missing;
  rhohv = missing;
  ncp = missing;

}

// print

void MomentsSun::print(ostream &out)

{

  out << "============ MomentsSun ==============" << endl;
  out << "  time: " << DateTime::strm((time_t) time) << endl;
  out << "  prt: " << prt << endl;
  out << "  az: " << az << endl;
  out << "  el: " << el << endl;
  out << "  offsetAz: " << offsetAz << endl;
  out << "  offsetEl: " << offsetEl << endl;

  out << "  nn: " << nn << endl;

  out << "  powerHc: " << powerHc << endl;
  out << "  powerVc: " << powerVc << endl;
  out << "  powerHx: " << powerHx << endl;
  out << "  powerVx: " << powerVx << endl;
  
  out << "  dbmHc: " << dbmHc << endl;
  out << "  dbmHx: " << dbmHx << endl;
  out << "  dbmVx: " << dbmVx << endl;
  out << "  dbmVc: " << dbmVc << endl;
  out << "  dbm: " << dbm << endl;
  out << "  dbBelowPeak: " << dbBelowPeak << endl;

  out << "  corrMagHc: " << corrMagHc << endl;
  out << "  corrMagVc: " << corrMagVc << endl;
  out << "  corrMag: " << corrMag << endl;

  out << "  corr00Hc: " << corr00Hc << endl;
  out << "  corr00Vc: " << corr00Vc << endl;
  out << "  corr00: " << corr00 << endl;

  out << "  arg00Hc: " << arg00Hc << endl;
  out << "  arg00Vc: " << arg00Vc << endl;
  out << "  arg00: " << arg00 << endl;

  out << " ratioDbmHcHx: " << ratioDbmHcHx << endl;
  out << " ratioDbmVcVx: " << ratioDbmVcVx << endl;
  out << " ratioDbmVxHc: " << ratioDbmVxHc << endl;
  out << " ratioDbmVcHx: " << ratioDbmVcHx << endl;
  out << " ratioDbmVcHc: " << ratioDbmVcHc << endl;
  out << " ratioDbmVxHx: " << ratioDbmVxHx << endl;

  out << " S1S2: " << S1S2 << endl;
  out << " SS: " << SS << endl;

  out << " zdr: " << zdr << endl;
  out << " phidp: " << phidp << endl;
  out << " rhohv: " << rhohv << endl;
  out << " ncp: " << ncp << endl;

}

// copy

void MomentsSun::copy(const MomentsSun &orig)

{

  time = orig.time;
  prt = orig.prt;
  az = orig.az;
  el = orig.el;
  offsetAz = orig.offsetAz;
  offsetEl = orig.offsetEl;

  nn = orig.nn;

  powerHc = orig.powerHc;
  powerHx = orig.powerHx;
  powerVx = orig.powerVx;
  powerVc = orig.powerVc;

  dbmHc = orig.dbmHc;
  dbmHx = orig.dbmHx;
  dbmVx = orig.dbmVx;
  dbmVc = orig.dbmVc;

  dbm = orig.dbm;
  dbBelowPeak = orig.dbBelowPeak;

  sumRvvhh0Hc = orig.sumRvvhh0Hc;
  sumRvvhh0Vc = orig.sumRvvhh0Vc;
  sumRvvhh0 = orig.sumRvvhh0;

  Rvvhh0Hc = orig.Rvvhh0Hc;
  Rvvhh0Vc = orig.Rvvhh0Vc;
  Rvvhh0 = orig.Rvvhh0;

  corrMagHc = orig.corrMagHc;
  corrMagVc = orig.corrMagVc;
  corrMag = orig.corrMag;

  corr00Hc = orig.corr00Hc;
  corr00Vc = orig.corr00Vc;
  corr00 = orig.corr00;

  arg00Hc = orig.arg00Hc;
  arg00Vc = orig.arg00Vc;
  arg00 = orig.arg00;
  
  ratioDbmHcHx = orig.ratioDbmHcHx;
  ratioDbmVcVx = orig.ratioDbmVcVx;
  ratioDbmVxHc = orig.ratioDbmVxHc;
  ratioDbmVcHx = orig.ratioDbmVcHx;
  ratioDbmVcHc = orig.ratioDbmVcHc;
  ratioDbmVxHx = orig.ratioDbmVxHx;

  S1S2 = orig.S1S2;
  SS = orig.SS;

  zdr = orig.zdr;
  phidp = orig.phidp;
  rhohv = orig.rhohv;
  ncp = orig.ncp;

}

// interpolate

void MomentsSun::interp(const MomentsSun &m1,
                        const MomentsSun &m2,
                        double wt1,
                        double wt2)

{

  time = (wt1 * m1.time) + (wt2 * m2.time);
  prt = (wt1 * m1.prt) + (wt2 * m2.prt);
  az = (wt1 * m1.az) + (wt2 * m2.az);
  el = (wt1 * m1.el) + (wt2 * m2.el);
  offsetAz = (wt1 * m1.offsetAz) + (wt2 * m2.offsetAz);
  offsetEl = (wt1 * m1.offsetEl) + (wt2 * m2.offsetEl);

  nn = (wt1 * m1.nn) + (wt2 * m2.nn);

  powerHc = (wt1 * m1.powerHc) + (wt2 * m2.powerHc);
  powerVc = (wt1 * m1.powerVc) + (wt2 * m2.powerVc);
  powerHx = (wt1 * m1.powerHx) + (wt2 * m2.powerHx);
  powerVx = (wt1 * m1.powerVx) + (wt2 * m2.powerVx);

  dbmHc = (wt1 * m1.dbmHc) + (wt2 * m2.dbmHc);
  dbmHx = (wt1 * m1.dbmHx) + (wt2 * m2.dbmHx);
  dbmVx = (wt1 * m1.dbmVx) + (wt2 * m2.dbmVx);
  dbmVc = (wt1 * m1.dbmVc) + (wt2 * m2.dbmVc);

  dbm = (wt1 * m1.dbm) + (wt2 * m2.dbm);
  dbBelowPeak = (wt1 * m1.dbBelowPeak) + (wt2 * m2.dbBelowPeak);

  sumRvvhh0Hc.re = (wt1 * m1.sumRvvhh0Hc.re) + (wt2 * m2.sumRvvhh0Hc.re);
  sumRvvhh0Hc.im = (wt1 * m1.sumRvvhh0Hc.im) + (wt2 * m2.sumRvvhh0Hc.im);
  sumRvvhh0Vc.re = (wt1 * m1.sumRvvhh0Vc.re) + (wt2 * m2.sumRvvhh0Vc.re);
  sumRvvhh0Vc.im = (wt1 * m1.sumRvvhh0Vc.im) + (wt2 * m2.sumRvvhh0Vc.im);
  sumRvvhh0.re = (wt1 * m1.sumRvvhh0.re) + (wt2 * m2.sumRvvhh0.re);
  sumRvvhh0.im = (wt1 * m1.sumRvvhh0.im) + (wt2 * m2.sumRvvhh0.im);

  Rvvhh0Hc.re = (wt1 * m1.Rvvhh0Hc.re) + (wt2 * m2.Rvvhh0Hc.re);
  Rvvhh0Hc.im = (wt1 * m1.Rvvhh0Hc.im) + (wt2 * m2.Rvvhh0Hc.im);
  Rvvhh0Vc.re = (wt1 * m1.Rvvhh0Vc.re) + (wt2 * m2.Rvvhh0Vc.re);
  Rvvhh0Vc.im = (wt1 * m1.Rvvhh0Vc.im) + (wt2 * m2.Rvvhh0Vc.im);
  Rvvhh0.re = (wt1 * m1.Rvvhh0.re) + (wt2 * m2.Rvvhh0.re);
  Rvvhh0.im = (wt1 * m1.Rvvhh0.im) + (wt2 * m2.Rvvhh0.im);

  corrMagHc = (wt1 * m1.corrMagHc) + (wt2 * m2.corrMagHc);
  corrMagVc = (wt1 * m1.corrMagVc) + (wt2 * m2.corrMagVc);
  corrMag = (wt1 * m1.corrMag) + (wt2 * m2.corrMag);

  corr00Hc = (wt1 * m1.corr00Hc) + (wt2 * m2.corr00Hc);
  corr00Vc = (wt1 * m1.corr00Vc) + (wt2 * m2.corr00Vc);
  corr00 = (wt1 * m1.corr00) + (wt2 * m2.corr00);

  arg00Hc = (wt1 * m1.arg00Hc) + (wt2 * m2.arg00Hc);
  arg00Vc = (wt1 * m1.arg00Vc) + (wt2 * m2.arg00Vc);
  arg00 = (wt1 * m1.arg00) + (wt2 * m2.arg00);

  ratioDbmHcHx = (wt1 * m1.ratioDbmHcHx) + (wt2 * m2.ratioDbmHcHx);
  ratioDbmVcVx = (wt1 * m1.ratioDbmVcVx) + (wt2 * m2.ratioDbmVcVx);
  ratioDbmVxHc = (wt1 * m1.ratioDbmVxHc) + (wt2 * m2.ratioDbmVxHc);
  ratioDbmVcHx = (wt1 * m1.ratioDbmVcHx) + (wt2 * m2.ratioDbmVcHx);
  ratioDbmVcHc = (wt1 * m1.ratioDbmVcHc) + (wt2 * m2.ratioDbmVcHc);
  ratioDbmVxHx = (wt1 * m1.ratioDbmVxHx) + (wt2 * m2.ratioDbmVxHx);

  S1S2 = (wt1 * m1.S1S2) + (wt2 * m2.S1S2);
  SS = (wt1 * m1.SS) + (wt2 * m2.SS);

  zdr = (wt1 * m1.zdr) + (wt2 * m2.zdr);
  phidp = (wt1 * m1.phidp) + (wt2 * m2.phidp);
  rhohv = (wt1 * m1.rhohv) + (wt2 * m2.rhohv);
  ncp = (wt1 * m1.ncp) + (wt2 * m2.ncp);

}

// adjust powers for noise

void MomentsSun::adjustForNoise(double noisePowerHc,
                                double noisePowerHx,
                                double noisePowerVc,
                                double noisePowerVx)

{

  if (powerHc != missing) {
    powerHc -= noisePowerHc;
    if (powerHc <= 0) {
      powerHc = 1.0e-12;
    }
    dbmHc = 10.0 * log10(powerHc);
  }

  if (powerVc != missing) {
    powerVc -= noisePowerVc;
    if (powerVc <= 0) {
      powerVc = 1.0e-12;
    }
    dbmVc = 10.0 * log10(powerVc);
  }

  if (powerHx != missing) {
    powerHx -= noisePowerHx;
    if (powerHx <= 0) {
      powerHx = 1.0e-12;
    }
    dbmHx = 10.0 * log10(powerHx);
  }

  if (powerVx != missing) {
    powerVx -= noisePowerVx;
    if (powerVx <= 0) {
      powerVx = 1.0e-12;
    }
    dbmVx = 10.0 * log10(powerVx);
  }

}

// compute ratios

void MomentsSun::computeRatios()

{

  if (dbmHc != missing && dbmHx != missing) {
    ratioDbmHcHx = dbmHc - dbmHx;
  }
  if (dbmVc != missing && dbmVx != missing) {
    ratioDbmVcVx = dbmVc - dbmVx;
  }
  if (dbmVx != missing && dbmHc != missing) {
    ratioDbmVxHc = dbmVx - dbmHc;
  }
  if (dbmVc != missing && dbmHx != missing) {
    ratioDbmVcHx = dbmVc - dbmHx;
  }
  if (dbmVc != missing && dbmHc != missing) {
    ratioDbmVcHc = dbmVc - dbmHc;
  }
  if (dbmVx != missing && dbmHx != missing) {
    ratioDbmVxHx = dbmVx - dbmHx;
  }

  if (ratioDbmVcHc != missing && ratioDbmVxHx != missing) {
    S1S2 = ratioDbmVcHc + ratioDbmVxHx;
  }

  if (ratioDbmVcHc != missing) {
    SS = 2 * ratioDbmVcHc;
  }

}

// print

void MomentsSun::print(ostream &out) const

{

  out << "========== MomentsSun data ========================" << endl;
  out << "  time: " << time << endl;
  out << "  prt: " << prt << endl;
  out << "  az: " << az << endl;
  out << "  el: " << el << endl;
  out << "  offsetAz: " << offsetAz << endl;
  out << "  offsetEl: " << offsetEl << endl;
  out << "  nn: " << nn << endl;
  out << "  sumRvvhh0.re: " << sumRvvhh0.re << endl;
  out << "  sumRvvhh0.im: " << sumRvvhh0.im << endl;
  out << "  dbm: " << dbm << endl;
  out << "  dbBelowPeak: " << dbBelowPeak << endl;
  out << "  dbmHc: " << dbmHc << endl;
  out << "  dbmHx: " << dbmHx << endl;
  out << "  dbmVx: " << dbmVx << endl;
  out << "  dbmVc: " << dbmVc << endl;
  out << "  Rvvhh0.re: " << Rvvhh0.re << endl;
  out << "  Rvvhh0.im: " << Rvvhh0.im << endl;
  out << "  corrMag: " << corrMag << endl;
  out << "  corr00: " << corr00 << endl;
  out << "  arg00: " << arg00 << endl;
  out << "  ratioDbmHcHx: " << ratioDbmHcHx << endl;
  out << "  ratioDbmVcVx: " << ratioDbmVcVx << endl;
  out << "  ratioDbmVxHc: " << ratioDbmVxHc << endl;
  out << "  ratioDbmVcHx: " << ratioDbmVcHx << endl;
  out << "  ratioDbmVcHc: " << ratioDbmVcHc << endl;
  out << "  ratioDbmVxHx: " << ratioDbmVxHx << endl;
  out << "  S1S2: " << S1S2 << endl;
  out << "  SS: " << SS << endl;

}


