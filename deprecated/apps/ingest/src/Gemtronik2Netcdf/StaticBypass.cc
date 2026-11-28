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
/////////////////////////////////////////////////////////////
//  StaticBypass:  Stores static BypassMaps for known radars.
//
//  Jason Craig, RAP, NCAR, Boulder, CO, 80307, USA
//  March 2012
//
//  $Id: StaticBypass.cc,v 1.5 2016/03/07 01:23:00 dixon Exp $
//
/////////////////////////////////////////////////////////////

#include "StaticBypass.hh"

BypassSegment_t *getBypassSegment(char radarName[5], float fixedAngle)
{


  if(radarName[0] == 'R' && radarName[1] == 'C' && radarName[2] == 'C' && radarName[3] == 'G') 
  {
    if(fixedAngle < 1.38)
      return &RCCG_1_38;
    else
      if(fixedAngle < 2.38)
	return &RCCG_2_38;
      else
	if(fixedAngle < 4.28)
	  return &RCCG_4_28;
	else
	  return &RCCG_4_30;
  }
  if(radarName[0] == 'R' && radarName[1] == 'C' && radarName[2] == 'K' && radarName[3] == 'T') 
  {
    if(fixedAngle < 3.38)
      return &RCKT_3_38;
    else
      if(fixedAngle < 9.88)
	return &RCKT_9_88;
      else
	return &RCKT_9_90;
  }
  if(radarName[0] == 'R' && radarName[1] == 'C' && radarName[2] == 'H' && radarName[3] == 'L') 
  {
    if(fixedAngle < 2.38)
      return &RCHL_2_38;
    else
      if(fixedAngle < 5.98)
	return &RCHL_5_98;
      else
	if(fixedAngle < 14.58)
	  return &RCHL_14_58;
	else
	  return &RCHL_14_60;
  }
  return NULL;
}
