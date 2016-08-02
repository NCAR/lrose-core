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

#include "ridds2mom.h"
using namespace std;

short                         
get_target_elev(short *vcp_type, short *tilt_num,
                NEXRAD_vcp_set * vol_cntrl_patterns)                            
{
  NEXRAD_vcp     *vcp;                              
  NEXRAD_vcp     *tmp;
  int             i;
  float          *fixed_angle;
   
  vcp = vol_cntrl_patterns->scan_strategy;
  for (i = 0; i < vol_cntrl_patterns->vcp_total; i++) {           
    if (vcp->pattern_number == *vcp_type) {       
      if (*tilt_num - 1 > vcp->num_of_fixed_angles) {
        fprintf(stderr, "error in get_target_elev tilt_num %d > number of fixed angles %d\n",
		*tilt_num, vcp->num_of_fixed_angles);
        return (-1);
      } else {
        fixed_angle = *vcp->fixed_angle;
        return ((short) (fixed_angle[*tilt_num - 1] * 100));
      }
    }
    tmp = vcp;
    vcp = (NEXRAD_vcp *) tmp->next_pattern;
  }
  fprintf(stderr, "error in get_target_elev vol contrl pattern %d not found\n",
                *vcp_type);

  return (0);
}
