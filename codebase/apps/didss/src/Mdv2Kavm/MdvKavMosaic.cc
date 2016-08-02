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
 * MdvKavMosaic: class manipulating a Kavouras mosaic file using MDV
 *               format data.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>
#include <rapformats/kav_grid.h>
#include <toolsa/str.h>
#include <toolsa/udatetime.h>

#include "MdvKavMosaic.hh"
using namespace std;


/**********************************************************************
 * Constructor
 */

MdvKavMosaic::MdvKavMosaic() :
  KavMosaic()
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

MdvKavMosaic::~MdvKavMosaic()
{
  // Do nothing
}


/**********************************************************************
 * loadFile() - Load the Kavouras file information based on the given
 *              MDV information.
 *
 * Returns true on success, false on failure.
 */

bool MdvKavMosaic::loadFile(const Mdvx::master_header_t &master_hdr,
			    const MdvxField &field,
			    const string filename_prefix,
			    const string filename_ext)
{
  // Load the header values

  KM_header_t kav_hdr;
  
  memset(&kav_hdr, 0, sizeof(kav_hdr));
  
  MdvxPjg mdv_proj(master_hdr, field.getFieldHeader());
  
  double x_width = mdv_proj.getDx() * mdv_proj.getNx();
  double y_width = mdv_proj.getDy() * mdv_proj.getNy();
  
  double x_center = mdv_proj.getMinx() + (x_width / 2.0);
  double y_center = mdv_proj.getMiny() + (y_width / 2.0);
  
  mdv_proj.xy2latlon(x_center, y_center,
		     kav_hdr.lat0, kav_hdr.lon0);
  
  double lat_min, lon_min;
  
  mdv_proj.xy2latlon(mdv_proj.getMinx(), mdv_proj.getMiny(),
		     lat_min, lon_min);
  
  kav_hdr.lon_width = (kav_hdr.lon0 - lon_min) * 2.0;
  kav_hdr.aspect = y_width / x_width;
  
  kav_hdr.time.unix_time = master_hdr.time_centroid;
  uconvert_from_utime(&kav_hdr.time);
  
  kav_hdr.nx = mdv_proj.getNx();
  kav_hdr.ny = mdv_proj.getNy();
  
  kav_hdr.nsites = MAX_SITES;
  
  sprintf(kav_hdr.day, "%2d", kav_hdr.time.day);
  STRcopy(kav_hdr.mon, _MONTH_ARRAY[kav_hdr.time.month-1].c_str(), 4);
  if (kav_hdr.time.year >= 2000)
    sprintf(kav_hdr.year, "%2d", kav_hdr.time.year - 2000);
  else
    sprintf(kav_hdr.year, "%2d", kav_hdr.time.year - 1900);
  sprintf(kav_hdr.hour, "%2d", kav_hdr.time.hour);
  sprintf(kav_hdr.min, "%d2", kav_hdr.time.min);
  
  sprintf(kav_hdr.filename, "%s%2d%2d.%s",
	  filename_prefix.c_str(), kav_hdr.time.hour, kav_hdr.time.min,
	  filename_ext.c_str());
  
  return true;
}
