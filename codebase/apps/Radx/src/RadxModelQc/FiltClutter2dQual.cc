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
 * @file FiltClutter2dQual.cc
 */
#include "FiltClutter2dQual.hh"
#include <toolsa/LogStream.hh>
#include <toolsa/globals.h>
#include <Radx/RadxVol.hh>
#include <cmath>
//------------------------------------------------------------------
FiltClutter2dQual::FiltClutter2dQual(const Params::data_filter_t f,
				     const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }

  _velFieldName = P._parm_clutter_2d_qual[f.filter_index].vel_field_name;
  _widthFieldName = P._parm_clutter_2d_qual[f.filter_index].width_field_name;
  _cmdFlagFieldName = 
    P._parm_clutter_2d_qual[f.filter_index].cmdflag_field_name;
  _swShapeFactor = P._parm_clutter_2d_qual[f.filter_index].sw_shape_factor;
  _vrShapeFactor = P._parm_clutter_2d_qual[f.filter_index].vr_shape_factor;
}

//------------------------------------------------------------------
FiltClutter2dQual::~FiltClutter2dQual()
{
}

//------------------------------------------------------------------
void FiltClutter2dQual::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
bool FiltClutter2dQual::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltClutter2dQual::filter(const time_t &t, const RadxRay *ray0,
			       const RadxRay &ray, const RadxRay *ray1,
			       std::vector<RayxData> &data) const
{
  RayxData fscr, vel, width, cmdFlag;
  if (!RadxApp::retrieveRay(_f.input_field, ray, data, fscr))
  {
    return false;
  }
  if (!RadxApp::retrieveRay(_velFieldName, ray, data, vel))
  {
    return false;
  }
  if (!RadxApp::retrieveRay(_widthFieldName, ray, data, width))
  {
    return false;
  }
  if (!RadxApp::retrieveRay(_cmdFlagFieldName, ray, data, cmdFlag))
  {
    return false;
  }

  // // replace small clutter with missing value
  // clut.maskWhenLessThan(_smallClut, 0.0, true);
  
  
  // // set SCR to dbz - clutter
  // RayxData scr(dbz);
  // scr.subtract(clut);

  
  // double clutter_filter_notch = _notchNumCoeff*(2.0*_meanNyquist)/_meanNsamples;
  // clutter_filter_notch /= 2.0;

  // RayxData scr_factor(scr);
  // scr_factor.inc(_scrIntercept);
  // scr_factor.divide(_scrDenom);
  
  double sw_factor = _swShapeFactor;
  double vr_factor = _vrShapeFactor;

  RayxData arg(vel);
  arg.abs();
  arg.multiply(vr_factor);
  
  RayxData arg2(width);
  arg2.multiply(sw_factor);
  
  arg.inc(arg2, false);
  arg.qscale1(-0.69, true);


  // RayxData fcsr(scr_factor);
  // fcsr.multiply(2.0);
  // fcsr.qscale1(-0.69, true);

  // RayxData fvrsw(vel);
  // fvrsw.abs();
  // fvrsw.multiply(vr_factor);
  // RayxData ss(width);
  // ss.multiply(sw_factor);
  // fvrsw.inc(ss, false);
  // fvrsw.qscale1(-0.69, true);

  RayxData fclut(arg);
  fclut.multiply(fscr, false);
  
  fclut.maskFilterLessThan(cmdFlag, 0.5, 1.0);


  RadxApp::modifyRayForOutput(fclut, _f.output_field, _f.output_units,
                              _f.output_missing);
  data.push_back(fclut);
  return true;
}

//------------------------------------------------------------------
void FiltClutter2dQual::filterVolume(const RadxVol &vol)
{
}

//------------------------------------------------------------------
void FiltClutter2dQual::finish(void)
{
}
