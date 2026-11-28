/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/***************************************************************************
 * convert_pararms.c
 *
 * converts parameter file values to internal defines
 *
 * Nancy Rehak
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * October 1997
 *
 ****************************************************************************/

#include <stdio.h>

#include <mdv/mdv_macros.h>

#include "mdv_update_data.h"

void convert_params(void)
{
  /*
   * data type
   */

  switch (Glob->params.data_type)
  {
  case DATA_INT8 :
    Glob->mdv_data_type = MDV_INT8;
    break;
    
  case DATA_INT16 :
    Glob->mdv_data_type = MDV_INT16;
    break;
    
  case DATA_FLOAT32 :
    Glob->mdv_data_type = MDV_FLOAT32;
    break;
    
  } /* endswitch - Glob->params.data_type */
  
  /*
   * data type
   */

  switch (Glob->params.output_type)
  {
  case OUTPUT_INT8 :
    Glob->mdv_output_type = MDV_INT8;
    break;
    
  case OUTPUT_INT16 :
    Glob->mdv_output_type = MDV_INT16;
    break;
    
  case OUTPUT_FLOAT32 :
    Glob->mdv_output_type = MDV_FLOAT32;
    break;
    
  case OUTPUT_PLANE_RLE8 :
    Glob->mdv_output_type = MDV_PLANE_RLE8;
    break;
    
  } /* endswitch - Glob->params.output_type */
  
  return;
}

