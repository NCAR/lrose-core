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
// TrecGauge2Spdb.h: Main program object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
/////////////////////////////////////////////////////////////

#ifndef TrecGauge2Spdb_H
#define TrecGauge2Spdb_H

#include <string>

#include "Args.hh"
#include "Params.hh"
#include <Mdv/DsMdvxTimes.hh>
using namespace std;

class DsMdvTimes;

class TrecGauge2Spdb {
  
public:

  // constructor

  TrecGauge2Spdb (int argc, char **argv);

  // destructor
  
  ~TrecGauge2Spdb();

  // run 

  int Run();

  // data members

  int OK;
  
protected:
  
private:

  string _progName;
  Args _args;
  Params _params;
  DsMdvxTimes _mdvInput;

  int _processGauge(const DsMdvx &mdvx,
		    const Params::gauge_t &gauge);

  int _computeMeanVel(time_t trec_time,
		      const DsMdvx &mdvx,
		      const Params::gauge_t &gauge,
		      double xx, double yy,
		      double &u_mean, double &v_mean,
		      double &u_gauge, double &v_gauge,
		      double &wt_gauge);

  int _computeKernelDbzMean(const DsMdvx &mdvx,
			    const Params::gauge_t &gauge,
			    double xx, double yy,
			    double &dbz_mean);

  int _computeKernelVelMean(const DsMdvx &mdvx,
			    const Params::gauge_t &gauge,
			    const char *vel_label,
			    double xx, double yy,
			    double &vel_mean);

  int _computeTimeAvGaugeWind(time_t trec_time,
			      const Params::gauge_t &gauge,
			      double &time_av_gauge_u,
			      double &time_av_gauge_v);

  int _computeKernelLimits(const DsMdvx &mdvx,
			   const char *field_label,
			   double kernel_size,
			   double xx, double yy,
			   int &start_ix,
			   int &start_iy,
			   int &end_ix,
			   int &end_iy);

  int _spaceAvTrecMotion(time_t trec_time,
			 const DsMdvx &mdvx,
			 const Params::gauge_t &gauge,
			 double xx, double yy,
			 double &trec_u_mean, double &trec_v_mean);

  int _timeAvTrecMotion(time_t trec_time,
			const DsMdvx &mdvx,
			const Params::gauge_t &gauge,
			double xx, double yy,
			double &trec_u_mean, double &trec_v_mean);

};

#endif
