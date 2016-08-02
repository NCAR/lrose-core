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
 * @file FiltAlgParms.cc
 */
#include <FiltAlg/FiltAlgParms.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
FiltAlgParms::FiltAlgParms() : FiltAlgParams()
{
}

//------------------------------------------------------------------
FiltAlgParms::FiltAlgParms(const FiltAlgParams &P) : FiltAlgParams(P)
{
  if (!realtime)
  {
    max_wait_minutes = 0;
  }
}

//------------------------------------------------------------------
FiltAlgParms::~FiltAlgParms()
{
}

//------------------------------------------------------------------
int FiltAlgParms::max_fuzzy_function_index(void) const
{
  // SEE THE paramdef file!!!
  return 14;
}

//------------------------------------------------------------------
const FiltAlgParams::fuzzy_t *FiltAlgParms::fuzzy_params(const int index,
							 int &nf) const
{
  if (index < 0 || index >= 11)
  {
    LOG(ERROR) << "Fuzzy index out of range [0,11]  value=" << index;
    nf = 0;
    return NULL;
  }

  const FiltAlgParams::fuzzy_t *ret;
  switch (index)
  {
  case 0:
    nf = fuzzy0_n;
    ret = _fuzzy0;
    break;
  case 1:
    nf = fuzzy1_n;
    ret = _fuzzy1;
    break;
  case 2:
    nf = fuzzy2_n;
    ret = _fuzzy2;
    break;
  case 3:
    nf = fuzzy3_n;
    ret = _fuzzy3;
    break;
  case 4:
    nf = fuzzy4_n;
    ret = _fuzzy4;
    break;
  case 5:
    nf = fuzzy5_n;
    ret = _fuzzy5;
    break;
  case 6:
    nf = fuzzy6_n;
    ret = _fuzzy6;
    break;
  case 7:
    nf = fuzzy7_n;
    ret = _fuzzy7;
    break;
  case 8:
    nf = fuzzy8_n;
    ret = _fuzzy8;
    break;
  case 9:
    nf = fuzzy9_n;
    ret = _fuzzy9;
    break;
  case 10:
    nf = fuzzy10_n;
    ret = _fuzzy10;
    break;
  case 11:
    nf = fuzzy11_n;
    ret = _fuzzy11;
    break;

  case 12:
    nf = fuzzy12_n;
    ret = _fuzzy12;
    break;

  case 13:
    nf = fuzzy13_n;
    ret = _fuzzy13;
    break;

  case 14:
  default:
    nf = fuzzy14_n;
    ret = _fuzzy14;
    break;

  }
  if (nf == 0)
  {
    LOG(ERROR) << index << "'th fuzzy function is empty yet accessed";
    return NULL;
  }
  else
  {
    return ret;
  }
}

