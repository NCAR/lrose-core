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
 * @file BeamBlock.cc
 */
#include "BeamBlock.hh"
#include <toolsa/LogMsg.hh>

//----------------------------------------------------------------
BeamBlock::BeamBlock(const Parms &params) : Data(params)
{
  // don't set up grids yet
  LOG(LogMsg::DEBUG, "Reading beam blockage file");
  vector<string> fields;
  fields.push_back(params.beam_block_field_name);
  fields.push_back(params.beam_peak_field_name);
  _dataIsOK = readLite(params.beam_block_path, fields, true);

  for (size_t i=0; i<_sweeps.size(); ++i)
  {
    _beamElevations.push_back(_sweeps[i].elev());
  }
  LOG(LogMsg::DEBUG, "Done Reading beam blockage file");
}

//----------------------------------------------------------------
BeamBlock::~BeamBlock(void)
{
}

//----------------------------------------------------------------
bool BeamBlock::align(const Data &data)
{
  _sweeps.clear();
  _elevIndex.clear();

  // return true;
  if (mismatch(data))
  {
    return false;
  }
  bool ret = true;
#ifdef CPP11
  for (auto &scan : data)
  {
#else
  for (size_t i=0; i<data.size(); ++i)
  {
    const Sweep &scan = data[i];
#endif
    int ind = closestElevIndex(scan.elev());
    _elevIndex.push_back(ind);
    if (ind < 0)
    {
      ret =  false;
    }
  }
  if (ret)
  {
    const vector<RadxSweep *> sw = _vol.getSweeps();
    const vector<RadxRay *> rays = _vol.getRays();
    // for each sweep referenced to by an elevation index
    for (size_t i=0; i<_elevIndex.size(); ++i)
    {
      int ii = _elevIndex[i];
      // create a Grids object
      _sweeps.push_back(Sweep(_beamElevations[ii], *sw[ii], rays, *this));
    }  
    ret = true;
    for (size_t i=0; i<_sweeps.size(); ++i)
    {
      if (!_sweeps[i]._isOK)
      {
	_sweeps.clear();
	ret = false;
      }
    }  
  }
  return ret;
}


// //----------------------------------------------------------------
// std::vector<double> BeamBlock::blockVertValues(double gate, double az,
// 					       const std::string &name) const
// {
//   // find indices to use for gate and azimuth
//   int ig, ia;
//   if (Geom::closestGateIndex(gate, ig) &&
//       Geom::closestOutputAzimuthIndex(az, ia))
//   {
//     return vertValues(ig, ia, name);
//   }
//   else
//   {
//     std::vector<double> ret;
//     return ret;
//   }
// }

//----------------------------------------------------------------
int BeamBlock::closestElevIndex(double elev) const
{
  if (_beamElevations.empty())
  {
    return -1;
  }

  if (elev < _beamElevations[0])
  {
    return 0;
  }
  if (elev >= _beamElevations[_beamElevations.size()-1])
  {
    return _beamElevations.size()-1;
  }

  for (size_t i=1; i<_beamElevations.size(); ++i)
  {
    if (elev >= _beamElevations[i-1] && elev <= _beamElevations[i])
    {
      if (elev - _beamElevations[i-1] <= _beamElevations[i] - elev)
      {
	return i-1;
      }
      else
      {
	return i;
      }
    }
  }
  LOG(LogMsg::ERROR, "Logic error should not get to here");
  return -1;
}
