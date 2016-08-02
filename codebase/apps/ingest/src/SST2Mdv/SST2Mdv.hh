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
// Class SST2Mdv
//
// Author:  Curtis Caravone
// Date:    6/20/2003
//
// Takes an SSTData and writes to a MDV file.
//
//

#ifndef SST2MDV_H
#define SST2MDV_H

#include <stdio.h>
#include <dataport/port_types.h>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>

#include "SSTData.hh"

/********
 * Writes the given Sea Surface Temperature data to MDV format.
 */
int sst2mdv(SSTData &data, Mdvx &mdvFile, si32 dataStartTime, si32 dataEndTime);

// Sets up all the common elements in the field header
// After this call the following header fields still need to be set:
//
// field_code
// scale
// bias
//
Mdvx::field_header_t _createFieldHeader(si32 time_centroid, char *field_name, char *units);

#endif
