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
/***************************************
 * smu_last_data : smu last data regsitration
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Sept 1996
 *
 */

#include <dataport/bigend.h>
#include <toolsa/globals.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/smu.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

static SERVMAP_info_t Info;
static int Init_done = FALSE;
static char *Servmap_host;
static char *Servmap_host2;

/******************************************
 * SMU_last_data_init()
 *
 * Sets up statics for last registration.
 *
 * Fill info is a function use to fill in some of the the data 
 * needed for registration. If NULL, defaults are used.
 *
 */

void SMU_last_data_init(char *type,
			char *subtype,
			char *instance,
			char *dir)
     
{

  memset(&Info, 0, sizeof(Info));
  STRncopy(Info.server_type, type, SERVMAP_NAME_MAX);
  STRncopy(Info.server_subtype, subtype, SERVMAP_NAME_MAX);
  STRncopy(Info.instance, instance, SERVMAP_INSTANCE_MAX);
  STRncopy(Info.dir, dir, SERVMAP_DIR_MAX);

  Servmap_host = getenv("SERVMAP_HOST");
  Servmap_host2 = getenv("SERVMAP_HOST2");

  Init_done = TRUE;
  return;
  
}

/******************************************
 * SMU_reg_last_data()
 *
 * Registers last data time
 */

void SMU_reg_last_data(si32 last_data_time)
     
{
  
  if (Init_done) {
    SMU_last_data(Servmap_host, Servmap_host2,
		  Info.server_type, Info.server_subtype,
		  Info.instance, Info.dir, last_data_time);
  }
  
  return;

}

