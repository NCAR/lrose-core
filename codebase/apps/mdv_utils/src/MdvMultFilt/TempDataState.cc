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
#include <toolsa/copyright.h>

/************************************************************************

Module:	TempDataState

Author:	Dave Albo

Date:	Fri Jan 27 12:12:54 2006

Description:   Temporary data storage class, all data.

************************************************************************/



/* System include files / Local include files */
#include <cstdio>
#include "TempDataState.hh"
using namespace std;

/* Constant definitions / Macro definitions / Type definitions */

/* External global variables / Non-static global variables / Static globals */

/* External functions / Internal global functions / Internal static functions */

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
TempDataState::TempDataState(void)
{
}

/*----------------------------------------------------------------*/
TempDataState::~TempDataState()
{
  _state.clear();
}

/*----------------------------------------------------------------*/
void TempDataState::print(void) const
{    
  map<string, TempData>::const_iterator i;
  for (i=_state.begin(); i!=_state.end(); ++i)
  {
    printf("TEMPDATA[%s]=\n", i->first.c_str());
    i->second.print();
  }
}

/*----------------------------------------------------------------*/
void TempDataState::init(const Params &P)
{
  // clear out any previous state.
  _state.clear();

  // for sure need that second input field name.
  for (int i=0; i<P.MdvCombine_n; ++i)
    _add(P._MdvCombine[i].SecondInpFieldName);

  // save all upstream inputs that are not immediately preceding the
  // current one.
  for (int i=0; i<P.FiltParm_n; ++i)
  {
    if (strcmp(P._FiltParm[i].InFieldName, P.InFieldName) == 0 && i > 0)
      // the main input is the input field (but its not the 0th filter..keep.
      _add(P._FiltParm[i].InFieldName);
    else
    {
      if (i == 0)
	continue;
      if (strcmp(P._FiltParm[i].InFieldName, P._FiltParm[i-1].OutFieldName) != 0)
	// the main input is not the previous output...keep
	_add(P._FiltParm[i].InFieldName);
    }
  }
}

/*----------------------------------------------------------------*/
void TempDataState::clear(void)
{
  map<string, TempData>::iterator i;

  for (i=_state.begin(); i!=_state.end(); ++i)
    i->second.clear();
}

/*----------------------------------------------------------------*/
void TempDataState::store_if_needed(string &name, Data &D)
{
  if (_state.find(name) != _state.end())
    // do need it so store.
    _state[name].store(D);
}

/*----------------------------------------------------------------*/
bool TempDataState::extract(string &name, Data &D)
{
  if (_state.find(name) == _state.end())
    return false;
  return _state[name].extract(D);
}

/*----------------------------------------------------------------*/
bool TempDataState::has_name(string &name) const
{
  return (_state.find(name) != _state.end());
}

/*----------------------------------------------------------------*/
fl32 *TempDataState::getVals(string &name, fl32 &missing, fl32 &bad, int &num)
{
  if (_state.find(name) == _state.end())
  {
    cerr << "Didn't find tempdata for " << name << endl;
    return NULL;
  }

  fl32 *f = _state[name].getVals(missing, bad, num);
  if (f == NULL)
  {
    cerr << "Didn't find allocated data for " << name << endl;
    return NULL;
  }
  return f;
}

/*----------------------------------------------------------------*/
void TempDataState::putVals(string &name, fl32 missing, fl32 bad)
{
  if (_state.find(name) == _state.end())
  {
    cerr << "Didn't find tempdata for " << name << endl;
    return;
  }
  _state[name].putVals(missing, bad);
}


/*----------------------------------------------------------------*/
void TempDataState::_add(const char *name)
{
  string s = name;
  if (_state.find(s) == _state.end())
  {
    // not already in there so add it now.
    TempData t;
    _state[s] = t;
  }
}
