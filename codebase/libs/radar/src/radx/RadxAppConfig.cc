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
 * @file RadxAppConfig.cc
 */
#include <radar/RadxAppConfig.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
RadxAppConfig::RadxAppConfig() : RadxAppParams()
{
}

//------------------------------------------------------------------
RadxAppConfig::RadxAppConfig(const RadxAppParams &P) :
  RadxAppParams(P)
{
  bool status = true;
  if (mode != RadxAppParams::REALTIME)
  {
    max_wait_minutes = 0;
  }

  // Build up the single primary URL and the secondary URLs
  int primaryIndex = -1;
  for (int i=0; i<P.input_n; ++i)
  {
    if (i == 0)
    {
      primaryIndex = P._input[i].index;
      _primaryGroup.index = P._input[i].index;
      _primaryGroup.dir = P._input[i].path;
      _primaryGroup.fileTimeOffset = P._input[i].file_match_time_offset_sec;
      _primaryGroup.fileTimeTolerance =
	P._input[i].file_match_time_tolerance_sec;
      _primaryGroup.rayElevTolerance = 
	P._input[i].ray_match_elevation_tolerance_deg;
      _primaryGroup.rayAzTolerance =
	P._input[i].ray_match_azimuth_tolerance_deg;
      _primaryGroup.rayTimeTolerance =
	P._input[i].ray_match_time_tolerance_sec;
      _primaryGroup.isClimo = P._input[i].is_climo;
      _primaryGroup.climoFile = P._input[i].climo_file;
    }
    else
    {
      Group G;
      G.dir = P._input[i].path;
      G.index = P._input[i].index;
      G.fileTimeOffset = P._input[i].file_match_time_offset_sec;
      G.fileTimeTolerance =
	P._input[i].file_match_time_tolerance_sec;
      G.rayElevTolerance = 
	P._input[i].ray_match_elevation_tolerance_deg;
      G.rayAzTolerance =
	P._input[i].ray_match_azimuth_tolerance_deg;
      G.rayTimeTolerance =
	P._input[i].ray_match_time_tolerance_sec;
      G.isClimo = P._input[i].is_climo;
      G.climoFile = P._input[i].climo_file;
      _secondaryGroups.push_back(G);
    }
  }

  for (int i=0; i<P.field_mapping_n; ++i)
  {
    if (P._field_mapping[i].index == primaryIndex)
    {
      _primaryGroup.names.push_back(P._field_mapping[i].field);
    }
    else
    {
      bool found = false;
      for (int j=0; j<(int)_secondaryGroups.size(); ++j)
      {
	if (_secondaryGroups[j].index == P._field_mapping[i].index)
	{
	  _secondaryGroups[j].names.push_back(P._field_mapping[i].field);
	  found = true;
	  break;
	}
      }
      if (!found)
      {
	LOG(ERROR) << "Never found index " << P._field_mapping[i].index
		   << " in mappings, not used";
	status = false;
      }
    }
  }

  if (_primaryGroup.names.empty())
  {
    LOG(ERROR) << "Primary URL not used in a mapping";
    status = false;
  }
  
  if (!status)
  {
    exit(1);
  }
}

//------------------------------------------------------------------
RadxAppConfig::~RadxAppConfig()
{
}

//------------------------------------------------------------------
bool RadxAppConfig::inputsAccountedFor(const vector<string> &inputs) const
{
  bool status = true;
  for (int i=0; i<(int)inputs.size(); ++i)
  {
    string name = inputs[i];
    bool found = false;
    for (int j=0; j<(int)_primaryGroup.names.size(); ++j)
    {
      string n2 = _primaryGroup.names[j];
      if (n2 == name)
      {
	found = true;
	break;
      }
    }
    if (!found)
    {
      for (int j=0; j<(int)_secondaryGroups.size(); ++j)
      {
	for (int k=0; k<(int)_secondaryGroups[j].names.size(); ++k)
	{
	  string n2 = _secondaryGroups[j].names[k];
	  if (n2 == name)
	  {
	    found = true;
	    break;
	  }
	}
	if (found)
	{
	  break;
	}
      }
    }
    if (!found)
    {
      LOG(ERROR) << "Never found input " << name << " in indexing";
      status = false;
    }
  }
  return status;
}
