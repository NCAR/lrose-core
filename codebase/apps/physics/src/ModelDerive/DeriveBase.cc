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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.3 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * DeriveBase.cc: File containing base class for all derive classes
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#include "DeriveBase.hh"

int DeriveBase::numberOfInputs = 0;
int DeriveBase::numberOfOutputs = 0;
vector<string> DeriveBase::output_short_names;
vector<string> DeriveBase::output_long_names;
vector<string> DeriveBase::output_units;

DeriveBase::DeriveBase(const float missing, const float bad, int nx, int ny, int nz) 
  : missing(missing), bad(bad)
{
  this->nx = nx;
  this->ny = ny;
  this->nz = nz;
  this->nPts = nx * ny * nz;
  this->numberOfInputs = 0;
  this->numberOfOutputs = 0;
}

DeriveBase::~DeriveBase() 
{
  this->output_short_names.clear();
  this->output_long_names.clear();
  this->output_units.clear();
  this->numberOfInputs = 0;
  this->numberOfOutputs = 0;
  
}

int DeriveBase::derive(vector<float*> *inputs, vector<float*> *outputs) 
{
  if(inputs->size() != this->numberOfInputs) {
    return -2;
  }
  return 0;
}
