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
/*********************************************************************
 * MdvPrivate.cc: MDV object code.  This object manipulates MDV
 *                information.  This file contains the miscellaneous
 *                private member functions.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cerrno>
#include <cassert>

#include <mdv/mdv_file.h>
#include <mdv/mdv_user.h>
#include <toolsa/mem.h>

#include <mdv/Mdv.h>
using namespace std;

/*
 * Global variables
 */


/*********************************************************************
 * _createInitialMasterHdr() - Creates an initial master header with
 *                             default values for the dataset.
 */

MDV_master_header_t *Mdv::_createInitialMasterHdr(void)
{
  MDV_master_header_t *master_hdr;
  
  master_hdr = (MDV_master_header_t *)umalloc(sizeof(MDV_master_header_t));
  
  MDV_init_master_header(master_hdr);
  
  return(master_hdr);
}
