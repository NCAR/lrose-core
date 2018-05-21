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
 * @file FiltInfoInput.cc
 */
#include <FiltAlg/FiltInfoInput.hh>
#include <FiltAlg/Filter.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
FiltInfoInput::FiltInfoInput(void) : 
  _has_vlevels(false), _gin(NULL), _filter(NULL), _gout(NULL), 
  _vIndex(-1), _vlevel(-1), _in(NULL), _out(NULL)
{
  
}

//------------------------------------------------------------------
FiltInfoInput::FiltInfoInput(const VlevelSlice *gi,
			     const std::vector<double> &vlevels,
			     const Filter *f,
			     const Data *g, const int index,
			     const double vlevel, const GridProj &gp) :
  _has_vlevels(true), _vlevels(vlevels), _gin(gi), _filter(f), _gout(g),
  _vIndex(index), _vlevel(vlevel), _gp(gp), _in(NULL), _out(NULL)
{
  
}

//------------------------------------------------------------------
FiltInfoInput::FiltInfoInput(const Data *in, const Data *out) :
  _has_vlevels(false), _gin(NULL), _filter(NULL), _gout(NULL), 
  _vIndex(-1), _vlevel(-1), _in(in), _out(out)
{
  
}

//------------------------------------------------------------------
FiltInfoInput::~FiltInfoInput()
{
}

//------------------------------------------------------------------
void FiltInfoInput::printPtrs(void) const
{
  if (_has_vlevels)
  {
    printf("slice=%p\n", _gin);
    printf("filter=%p\n", _filter);
    printf("Data=%p\n", _gout);
    printf("_vindex = %d\n", _vIndex);
    printf("Vlevel=%lf\n", _vlevel);
  }
  else
  {
    printf("in=%p\n", _in);
    printf("out=%p\n", _out);
  }
}


//------------------------------------------------------------------
void FiltInfoInput::printFilter(const bool isStart, const bool debug) const
{
  if (_has_vlevels)
  {
    if (isStart)
    {
      _filter->filter_print(_vlevel);
      if (debug)
      {
	LOG(DEBUG) << "========> Filtering:    " << _vlevel << " " << _vIndex;
      }
    }
    else
    {
      if (debug)
      {
	LOG(DEBUG) << "========> Done Filtering:" << _vlevel << " " << _vIndex;
      }
    }
  }
  else
  {
    if (isStart)
    {
      _filter->filter_print();
    }
  }
}

