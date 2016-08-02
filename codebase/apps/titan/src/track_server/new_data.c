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
/*********************************************************************
 * new_data.c
 *
 * Tests to see if new data has arrived. Returns TRUE if new data
 * has arrived, FALSE otherwise
 *
 * RAP, NCAR, Boulder CO
 *
 * March 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "track_server.h"
#include <toolsa/ldata_info.h>

static char Storm_header_file_path[MAX_PATH_LEN];
static char Track_header_file_path[MAX_PATH_LEN];
static si32 Latest_data_time;
static int Realtime_avail = FALSE;
  
int new_data(void)

{

  static int first_call = TRUE;
  static LDATA_handle_t ldata;

  /*
   * initialize latest data info handle
   */

  if (first_call) {
    LDATA_init_handle(&ldata,
		      Glob->prog_name,
		      Glob->params.debug);
    first_call = FALSE;
  }

  if (!Glob->params.realtime_avail) {
    return (FALSE);
  }
  
  /*
   * Get latest data time, set end time to this.
   * Set retry time to 10 msecs. We cannot set this to
   * 0, because then  cdata_read_index_simple() returns
   * without checking seq_num or max_valid_age.
   */
  
  if (LDATA_info_read(&ldata,
		      Glob->params.storm_data_dir,
		      Glob->params.max_realtime_valid_age)) {
    /*
     * no valid file
     */
    
    return (FALSE);
    
  } else {
    
    /*
     * new data - set header file paths
     */
    
    sprintf((char *) Storm_header_file_path, "%s%s%s.%s",
	    Glob->params.storm_data_dir, PATH_DELIM,
	    ldata.info.user_info_1, STORM_HEADER_FILE_EXT);

    sprintf((char *) Track_header_file_path, "%s%s%s.%s",
	    Glob->params.storm_data_dir, PATH_DELIM,
	    ldata.info.user_info_1, TRACK_HEADER_FILE_EXT);

    Latest_data_time = ldata.info.latest_time;

    Realtime_avail = TRUE;
    
    return (TRUE);
    
  }

}

/*********************************************************************
 * realtime_avail()
 *
 * Checks for availability of realtime data.
 *
 * Returns TRUE if available, FALSE if not.
 *
 * If TRUE, also sets other args.
 *
 *********************************************************************/

int realtime_avail(char **storm_header_file_path_p,
		   char **track_header_file_path_p,
		   si32 *latest_data_time_p)

{

  if (Realtime_avail) {
    
    *storm_header_file_path_p = (char *) Storm_header_file_path;
    *track_header_file_path_p = (char *) Track_header_file_path;
    *latest_data_time_p = Latest_data_time;
    
    return (TRUE);
    
  } else {
    
    return (FALSE);
    
  }

}
