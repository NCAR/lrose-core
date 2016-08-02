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

/****************************************************************************
 * output.c
 *
 * SPDB output module
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307-3000
 *
 * Sept 1997
 *
 ****************************************************************************/

#include "tstorms2spdb.h"
#include <symprod/spdb.h>
#include <symprod/spdb_products.h>

static spdb_handle_t Db;
static int Init_done = FALSE;

/***************
 * output_init()
 *
 * Initialize SPDB handle
 */

int output_init(void)
     
{
  
  if (SPDB_init(&Db, SPDB_TSTORMS_LABEL, SPDB_TSTORMS_ID,
		Glob->params.output_spdb_dir)) {
    return (-1);
  } else {
    Init_done = TRUE;
    return (0);
  }

}

/****************
 * output_write()
 *
 * Write output message to SPDB
 *
 * Returns 0 on success, -1 on failure
 */

int output_write(si32 valid_time, si32 expire_time,
		 ui08 *message, int messlen)
     
{

  if (Init_done) {
    if(SPDB_store_over(&Db, SPDB_TSTORMS_PROD_TYPE,
		       valid_time, expire_time,
		       message, messlen)) {
      return (-1);
    } else {
      return (0);
    }
  } else {
    fprintf(stderr, "ERROR - %s:output_write\n", Glob->prog_name);
    fprintf(stderr, "Init not done\n");
    return (-1);
  }

}

/****************
 * output_close()
 *
 * Free up SPDB handle
 */

void output_close(void)

{
  if (Init_done) {
    SPDB_close(&Db);
    Init_done = FALSE;
  }
}

