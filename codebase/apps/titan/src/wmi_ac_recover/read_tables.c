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

/* Read in acquisition tables etc. */

#include "wmi_ac_recover.h"

/**********************************************************************/

int read_tables(void)

{

  int errflg=OKAY;
 
  /*
   * determine where swapping needed
   */
  
  switch (BE_is_big_endian()) {
  case TRUE:
    Glob->swap_data = TRUE;
    break;
  case FALSE:
    Glob->swap_data= FALSE;
    break;
  }
  if (Glob->swap_data && Glob->params.debug)   
    fprintf(stderr, "Will swap data when reading \n");

  /*
   * read in dataset information and fill in name and tag struct
   */
  
  errflg = fill_acqtbl();
  if (errflg == ERROR) {
    fprintf(stderr,
	    "Could not read aquisition table for identifier %s\n",
	    Glob->params.callsign);
    exit_str("Aquisition Table could not be read");
  }

  /*
   * read in function coefficient information
   */
  
  errflg = fill_fcoeff();
  if (errflg == ERROR) {
    fprintf(stderr,
	    "Could not read function coefficient table for identifier %s\n",
	    Glob->params.callsign);
    exit_str("Function Coefficient Table could not be read");
  }
  
  /*
   * read in calculated variable information
   */
  
  errflg = fill_cvars();
  if (errflg == ERROR) {
    fprintf(stderr,
	    "Could not read calculated variables table for identifier %s\n",
	    Glob->params.callsign);
    exit_str("Calculated Variables Table could not be read");
  }

  return(OKAY);
}

