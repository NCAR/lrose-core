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
 * convert_utils.cc
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1997
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "tstorms_spdb2symprod.h"

/*********************************************************************
 * convert_capstyle_param() - Convert the TDRP capstyle parameter to
 *                            the matching symprod value.
 */

int convert_capstyle_param(int capstyle)
{
  switch(capstyle)
  {
  case CAPSTYLE_BUTT :
    return(SYMPROD_CAPSTYLE_BUTT);
    
  case CAPSTYLE_NOT_LAST :
    return(SYMPROD_CAPSTYLE_NOT_LAST);
    
  case CAPSTYLE_PROJECTING :
    return(SYMPROD_CAPSTYLE_PROJECTING);

  case CAPSTYLE_ROUND :
    return(SYMPROD_CAPSTYLE_ROUND);
  }
  
  return(SYMPROD_CAPSTYLE_BUTT);
}


/*********************************************************************
 * convert_joinstyle_param() - Convert the TDRP joinstyle parameter to
 *                             the matching symprod value.
 */

int convert_joinstyle_param(int joinstyle)
{
  switch(joinstyle)
  {
  case JOINSTYLE_BEVEL :
    return(SYMPROD_JOINSTYLE_BEVEL);
    
  case JOINSTYLE_MITER :
    return(SYMPROD_JOINSTYLE_MITER);
    
  case JOINSTYLE_ROUND :
    return(SYMPROD_JOINSTYLE_ROUND);
  }
  
  return(SYMPROD_JOINSTYLE_BEVEL);
}


/*********************************************************************
 * convert_line_type_param() - Convert the TDRP line type parameter to
 *                             the matching symprod value.
 */

int convert_line_type_param(int line_type)
{
  switch(line_type)
  {
  case LINETYPE_SOLID :
    return(SYMPROD_LINETYPE_SOLID);
    
  case LINETYPE_DASH :
    return(SYMPROD_LINETYPE_DASH);
    
  case LINETYPE_DOT_DASH :
    return(SYMPROD_LINETYPE_DOT_DASH);
  }
  
  return(SYMPROD_LINETYPE_SOLID);
}
