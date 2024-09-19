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
/**************************************************************************
 * test_mapper.c :
 *
 * Mike Dixon
 *
 * RAP NCAR Boulder Colorado USA
 *
 * Feb 1996
 *
 **************************************************************************/

#include "test_procmap.hh"
#include <toolsa/sockutil.h>
#include <sys/times.h>
#include <dataport/port_types.h>

#ifdef NOT_NOW
static int connection_good()

{

  int fd;
  
  fprintf(stderr, "Trying .....\n");
  
  if ((fd = SKU_open_client(Glob->procmap_host, 5433)) >= 0) {
    fprintf(stderr, "Connection to %s, port %d, established\n",
	    Glob->procmap_host, 5433);
    SKU_close(fd);
    return (TRUE);
  } else {
    fprintf(stderr, "Unable to open connection to %s, port 5433\n",
	    Glob->procmap_host);
    return (FALSE);
  }

}
#endif

static void pr_times(clock_t real, struct tms *start, struct tms *end)

{

  static long clktck = 0;
  static double dtck;

  if (clktck == 0) {
    if ((clktck = sysconf(_SC_CLK_TCK)) < 0) {
      fprintf(stderr, "Error getting clock tick constant\n");
      clktck = 1;
    }
    dtck = (double) clktck;
  }

  fprintf(stdout, "  real:  %10.5f\n", real / dtck);
  fprintf(stdout, "  user:  %10.5f\n",
	  (end->tms_utime - start->tms_utime) / dtck);
  fprintf(stdout, "   sys:  %10.5f\n",
	  (end->tms_stime - start->tms_stime) / dtck);

}

static void register_proc(void)

{

  /* auto_init does the reg for us */

  PMU_auto_init(Glob->name, Glob->instance, Glob->max_reg_int);
  
}

/*
 * get_info : get the proc info from the proc mapper
 *
 */

static int get_info(PROCMAP_info_t **return_info_p)
     
{

  int i, retval;
  clock_t start, end;
  struct tms tms_start, tms_end;
  int nprocs;
  long uptime;
  PROCMAP_request_t request;
  PROCMAP_info_t *return_info;

  /*
   * init times
   */

  if ((start = times(&tms_start)) == -1) {
    fprintf(stderr, "times error");
  }

  /*
   * load up request struct
   */
  
  memset ((void *) &request,
	  (int) 0, (size_t) sizeof(PROCMAP_request_t));
  
  strncpy(request.name, Glob->name, PROCMAP_NAME_MAX);
  strncpy(request.instance, Glob->instance,
	  PROCMAP_INSTANCE_MAX);
  
  nprocs = 0;
  
  retval = PMU_requestInfo(&request, &nprocs, &uptime,
			   return_info_p, Glob->procmap_host, "none");

  if (retval == 0) {
    fprintf(stdout, "No procs found\n");
    SKU_set_headers_to_new();
    return (-1);
  }

  if (nprocs == 0) {
    SKU_set_headers_to_new();
    fprintf(stdout, "No proc mapper found\n");
    return (-1);
  }

  fprintf(stdout, "Request successful, %d procs\n", nprocs);
  
  return_info = *return_info_p;

  for (i = 0; i < nprocs; i++) {
    
    fprintf(stdout, "Proc %d: %s %d %s %s %s\n",
	    i,
	    return_info[i].host,
	    return_info[i].pid,
	    return_info[i].name,
	    return_info[i].instance,
	    return_info[i].status_str);
    
  }

  fflush(stdout);
  
  if ((end = times(&tms_end)) == -1) {
    fprintf(stderr, "times error");
  }
  
  fprintf(stdout, "QUERYING INFO: TIMES\n");
  pr_times(end-start, &tms_start, &tms_end);
  fprintf(stdout, "\n");

  /*
   * set sockutil headers to new in case they were set to old by
   *  the proc mapper
   */
  
  SKU_set_headers_to_new();

  return (0);

}

int test_mapper(void)

/*
 * returns 0 on success, -1 on failure
 */
     
{

  int retval;
  PROCMAP_info_t *return_info;

  /*
   * first test the connection
   */
#ifdef NOT_NOW
  if (connection_good()) {
#endif

    /*
     * register test proc
     */

    if (Glob->do_register) {
      fprintf(stdout, "Registering ...\n");
      register_proc();
    }
    
    /*
     * give the process mapper a little time to register
     * the process
     */

    sleep(1);
    
    /*
     * then get test proc's info
     */

    retval = get_info(&return_info);

    if (retval) {
      return (-1);
    }
    
    return (0);

#ifdef NOT_NOW
  } else {

    return (-1);

  } /* if (connection_good ... */
#endif

}
