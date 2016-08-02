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

/*
 * Module: free_clump_info.c
 *
 * Author: Gerry Wiener
 *
 * Date:   6/26/96
 *
 * Description:
 *     Free memory allocated in clump_info structure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <euclid/clump.h>
#include <euclid/boundary.h>

/************************************************************************

Function Name: 	EG_free_clump_info

Description:  	free memory allocated in clump_info structure
    
Returns:    	void

Notes:

************************************************************************/
void EG_free_clump_info(Clump_info *ci)
{
  if (ci->interval_order != NULL)
    {
      free(ci->interval_order);
      ci->interval_order = NULL;
    }
  if (ci->clump_order != NULL)
    {
      free(ci->clump_order);
      ci->clump_order = NULL;
    }
  if (ci->intervals != NULL)
    {
      free(ci->intervals);
      ci->intervals = NULL;
    }
  if (ci->row_hdr != NULL)
    {
      free(ci->row_hdr);
      ci->row_hdr = NULL;
    }
}

/************************************************************************

Function Name: 	EG_init_clump_info

Description:  	Initialize clump_info structure
    
Returns:    	void

Notes:

************************************************************************/
void EG_init_clump_info(Clump_info *ci)
{
  ci->interval_order = NULL;
  ci->clump_order = NULL;
  ci->intervals = NULL;
  ci->row_hdr = NULL;
}

