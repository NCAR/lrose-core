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
/**
 * @file KernelData.cc
 */
#include "KernelData.hh"
#include "RepohParams.hh"
#include "KernelPoints.hh"
#include "KernelGrids.hh"
#include <toolsa/LogStream.hh>
#include <euclid/PointList.hh>

/*----------------------------------------------------------------*/
KernelData::KernelData(void)
{
  // so far not good (hasn't been tested)
  _passedTests = false;
  _kernelStatus = DataStatus("KernelFilt_DBZ_diff");
  _pidStatus = DataStatus("PID");
  _sdbzAveStatus = DataStatus("SDBZ_ave");
  _sdbzInOutStatus = DataStatus("SDBZ_in_out");
  _sdbzRangeStatus = DataStatus("SDBZ_range");
  _D0Status = DataStatus("D0");
  _corrStatus = DataStatus("corr");
}

/*----------------------------------------------------------------*/
KernelData::~KernelData()
{
}

/*----------------------------------------------------------------*/
std::string KernelData::sprintStatus(void) const
{
  string bad = " ";
  string kstat;
  if (!_passedTests)
  {
    kstat = "Bad";
    bad = "(";
    bad += _kernelStatus.sprintIfBad();
    bad += _pidStatus.sprintIfBad();
    bad += _sdbzAveStatus.sprintIfBad();
    bad += _sdbzInOutStatus.sprintIfBad();
    bad += _sdbzRangeStatus.sprintIfBad();
    bad += _D0Status.sprintIfBad();
    bad += _corrStatus.sprintIfBad();
    bad += ")";
  }
  else
  {
    kstat = "OK ";
  }
  char buf[1000];
  sprintf(buf, "KernelData:Sdbz:%6.2lf Sdbz_in:%6.2lf Sdbz_range:%6.2lf "
	  " Ddff_nremove:%d D0:%4.2lf (ZDR_ave=%4.2lf z_ave=%8.2lf) "
	  "C:%3.2lf Stat:%s ",
	  _sdbz_ave, _sdbz_ave_out, _sdbz_range,
	  _kernel_npt_removed, _D0, _ZDR_ave, _z_ave, _corr,
	  kstat.c_str());
  string ret = buf;
  if (!_passedTests)
  {
    ret += bad;
  }
  return ret;
}

/*----------------------------------------------------------------*/
std::string KernelData::sprintDebug(void) const
{
  char buf[1000];
  sprintf(buf, "%10.5lf %10.5lf %10.5lf %10.5lf %10.5lf %d %d",
	  _D0, _corr, _sdbz_ave, _kdbz_ave,
	  _sdbz_range, _kernel_npt_removed, _passedTests);
  string s = buf;
  return s;
}

/*----------------------------------------------------------------*/
void KernelData::evaluate(void)
{
  // at radar
  // fake kernel at origin

  _kernelStatus._status = true;
  _pidStatus._status = true;
  _sdbzAveStatus._status = true;
  _sdbzInOutStatus._status = true;
  _sdbzRangeStatus._status = true;
  _D0Status._status = true;
  _corrStatus._status = true;
  _kernel_npt_removed = 0;
  _sdbz_ave = 0.0;
  _sdbz_ave_out = 0;
  _sdbz_range = 0.0;
  _kdbz_ave = 0.0;
  _ZDR_ave = 0.0;
  _z_ave = 0.0;
  _D0 = 0.0;
  _corr = 0.0;
  _passedTests = true;
}

/*----------------------------------------------------------------*/
void KernelData::evaluate(const KernelGrids &grids, const RepohParams &P,
			  const KernelPoints &pts, 
			  bool enoughPoints, int nFilt)
{
  // not at radar
  // set all the status things to true and then set false as needed

  _passedTests = true;
  _kernelStatus._status = enoughPoints;
  _pidStatus._status = true;
  _sdbzAveStatus._status = true;
  _sdbzInOutStatus._status = true;
  _sdbzRangeStatus._status = true;
  _D0Status._status = true;
  _corrStatus._status = true;
  _kernel_npt_removed = nFilt;

  // evaluate pid values on pts
  _pidStatus._status = pts.cloudPts().onIntList(P.pid_weather_n,
						P._pid_weather, 0.33,
						*grids._pid);
  if (!_pidStatus._status)
  {
    _passedTests = false;
  }
  
  // compute a mean sdbz value on pts
  bool outside = false;
  _sdbz_ave = pts.meanLinearAdjusted(*grids._sdbz, outside);
  if (_sdbz_ave < P.min_mean_s_dbz)
  {
    _passedTests = false;
    _sdbzAveStatus._status = false;
  }

  // compute mean sdbz value on outPts
  outside = true;
  _sdbz_ave_out = pts.meanLinearAdjusted(*grids._sdbz, outside);

  // see how different the averages are inside and outside, bad if too small
  if (_sdbz_ave - _sdbz_ave_out < P.min_s_dbz_kernel_non_kernel_diff)
  {
    _passedTests = false;
    _sdbzInOutStatus._status = false;
  }

  // get the range of values in the sdbz data
  outside = false;
  _sdbz_range = pts.dataRange(*grids._sdbz, outside);

  if (_sdbz_range > P.max_s_dbz_kernel_diff)
  {
    _passedTests = false;
    _sdbzRangeStatus._status = false;
  }

  // get the average K band dbz from pts
  outside = false;
  _kdbz_ave = pts.meanLinearAdjusted(*grids._kdbzAdjusted, outside);

  // get the zdr aerage from pts
  _ZDR_ave = pts.meanLinearAdjusted(*grids._szdr, outside);
  if (_ZDR_ave < 0.0)
  {
    // truncate at 0.0 minimum
    if (_ZDR_ave < -0.2) // param
    {
      LOG(DEBUG_VERBOSE) << "ZDR was very negative " << _ZDR_ave;
      _passedTests = false;
    }
    _ZDR_ave = 0.00;
  }

  // take average of sdbz keeping it linear
  _z_ave = pts.meanLinear(*grids._sdbz, outside);
  if (_z_ave != 0)
  {
    double zp = pow(_z_ave, 0.37);
    double gamma = 1.81*pow(_ZDR_ave/zp, .486);
    _D0 = gamma*pow(_z_ave, 0.136);
    if (_D0 > P.max_D0)
    {
      _D0Status._status = false;
      _passedTests = false;
    }
  }
  else
  {
    _D0 = 0.0;
    _D0Status._status = false;
    _passedTests = false;
  }
  
  _corr = pts.cloudPts().correlation(*grids._sdbz, *grids._kdbzAdjusted);
  if (_corr < P.min_S_K_dbz_correlation)
  {
    _corrStatus._status = false;  
    _passedTests = false;
  }
}

/*----------------------------------------------------------------*/
double KernelData::attenuation(void) const
{
  return _sdbz_ave -_kdbz_ave;
}
  

