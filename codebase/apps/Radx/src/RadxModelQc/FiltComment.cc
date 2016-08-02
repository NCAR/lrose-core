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
 * @file FiltComment.cc
 */
#include "Params.hh"
#include "FiltComment.hh"
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
FiltComment::FiltComment(const Params::data_filter_t f,
				 const Params &P) : Filter(f, P)
{
  if (!ok())
  {
    return;
  }
  _doPrintInputOutput = false;  // override default
  _comment = P._parm_comment[f.filter_index].comment;
}

//------------------------------------------------------------------
FiltComment::~FiltComment()
{
}

//------------------------------------------------------------------
void FiltComment::filter_print(void) const
{
  // want total length to be 70, padded on both sides by equal # of '-'
  int n = strlen(_f.input_field);
  int pad = (70 -n)/2;
  string spad = "";
  if (pad > 0)
  {
    for (int i=0; i<pad-1; ++i)
    {
      spad += "-";
    }
  }
  
  LOG(PRINT) << spad << " " << _f.input_field << " " << spad;

  spad = "";
  pad = 0;
  if (_comment.empty())
  {
    pad = 70;
    for (int i=0; i<pad-1; ++i)
    {
      spad += "-";
    }
    LOG(PRINT) << spad;
  }
  else
  {
    n = static_cast<int>(_comment.size());
    pad = (70 -n)/2;
    if (pad > 0)
    {
      for (int i=0; i<pad-1; ++i)
      {
	spad += "-";
      }
    }
    LOG(PRINT) << spad << " " << _comment << " " << spad;
  }
}

//------------------------------------------------------------------
bool FiltComment::canThread(void) const
{
  return true;
}

//------------------------------------------------------------------
bool FiltComment::filter(const time_t &t, const RadxRay *ray0,
			 const RadxRay &ray, const RadxRay *ray1,
			 std::vector<RayxData> &data) const
{
  return true;
}

//------------------------------------------------------------------
void FiltComment::filterVolume(const RadxVol &vol)
{
}

//------------------------------------------------------------------
void FiltComment::finish(void)
{
}

