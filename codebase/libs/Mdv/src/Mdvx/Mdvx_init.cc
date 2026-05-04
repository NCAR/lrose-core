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
//////////////////////////////////////////////////////////
// Mdvx_init.cc
//
// Initialize Mdvx structs etc.
//
// Mike Dixon, EOL, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2026
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
using namespace std;

void Mdvx::init(Mdvx::master_header_t &mhdr)
{
  memset(&mhdr, 0, sizeof(Mdvx::master_header_t));
}

void Mdvx::init(Mdvx::field_header_t &fhdr)
{
  memset(&fhdr, 0, sizeof(Mdvx::field_header_t));
}

void Mdvx::init(Mdvx::vlevel_header_t &vhdr)
{
  memset(&vhdr, 0, sizeof(Mdvx::vlevel_header_t));
}

void Mdvx::init(Mdvx::coord_t &coord)
{
  memset(&coord, 0, sizeof(Mdvx::coord_t));
}

