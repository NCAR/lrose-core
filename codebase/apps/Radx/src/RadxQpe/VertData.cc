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
 * @file VertData.cc
 */
#include "VertData.hh"
#include "Data.hh"
#include "BeamBlock.hh"
#include "Out.hh"
#include "BadValue.hh"

//----------------------------------------------------------------
static double _setValue(int igt, int iaz, const std::string &name,
			const Sweep &d)
{
  double v;
  if (d.value(igt, iaz, name, v))
  {
    return v;
  }
  else
  {
    return BadValue::value();
  }
}

//----------------------------------------------------------------
VertData::VertData(const Data &data, const BeamBlock &block, 
		   int igt, int iaz, const Parms &params)
{

  double az = data.ithOutputAz(iaz);
  double gt = data.ithGtMeters(igt);

  int ig, ia;
  bool blockOk = (block.closestGateIndex(gt, ig) &&
		  block.closestOutputAzimuthIndex(az, ia));

#ifdef CPP11
  for (auto & sweep : data)
  {
#else
  for (size_t i=0; i<data.size(); ++i)
  {
    const Sweep &sweep = data[i];
#endif
    double rangeKm;
    double htKm = sweep.getBeamHtKmMsl(igt, data, rangeKm);
    double elevDeg = sweep.elev();

    double blockage;
    double terrainheight;
    if (blockOk)
    {    
      // note that blockage input assumed to have same # of elevations
      size_t j = data.index(sweep);
      blockage = _setValue(igt, iaz, params.beam_block_field_name, block[j]);
      terrainheight = _setValue(igt, iaz, params.beam_peak_field_name, block[j]);
    }
    else
    {
      blockage = 0.0;
      terrainheight = -9999;
    }

    _data.push_back(VertData1(_setValue(igt, iaz, params.PID_field_name, sweep),
                              blockage, terrainheight, elevDeg, htKm, rangeKm,
                              _setValue(igt, iaz, params.SNR_field_name, sweep)));
  
  } // i

}


//----------------------------------------------------------------
VertData::~VertData(void)
{
}
