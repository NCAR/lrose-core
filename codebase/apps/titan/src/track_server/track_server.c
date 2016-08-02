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
/************************************************************************
 * track_server.c
 *
 * Serves the storm track data, reading from files.
 *
 * If in realtime_avail mode, uses file locking to ensure it does not
 * interfere with storm_ident and storm_track.
 * 
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * March 1992
 *
 *************************************************************************/

#define MAIN
#include "track_server.h"
#undef MAIN

#include <toolsa/sockutil.h>
#include <toolsa/smu.h>
#include <toolsa/pmu.h>

#include <signal.h>
#include <sys/wait.h>

static int Parent = TRUE;
static int Pid[MAX_CLIENTS];
static int Nchildren = 0;

int main(int argc, char **argv)

{

  char *params_file_path = NULL;

  int i, j;
  int protofd, sockfd;
  int dead_pid;
  int forever = TRUE;
  int check_params;
  int print_params;

  si32 tdata_max_children, track_max_children;
  si32 tdata_max_parents, track_max_parents;
  si32 wait_msecs;

  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * allocate global structure and other memory, initialize
   */
  
  Glob = (global_t *) umalloc((ui32) sizeof(global_t));
  memset((void *) Glob, (int) 0, sizeof(global_t));
  
  /*
   * set program name
   */

  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display ucopyright message
   */

  ucopyright(Glob->prog_name);

  /*
   * parse the command line
   */

  parse_args(argc, argv,
	     &check_params, &print_params,
	     &override,
	     &params_file_path);
  
  /*
   * load up parameters
   */
  
  Glob->table = track_server_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
			 Glob->table,
			 &Glob->params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    exit(0);
  }
  
  set_derived_params();
  
  /*
   * initialize server and process registration
   */

  register_server_init();
  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * check that the tdata_server.h values match the track.h values
   */

  tdata_max_parents = TDATA_MAX_PARENTS;
  track_max_parents = MAX_PARENTS;

  if (tdata_max_parents != track_max_parents) {

    fprintf(stderr, "Mismatch in header files for MAX_PARENTS\n");
    fprintf(stderr, "'tracks.h' value is %ld\n", (long) track_max_parents);
    fprintf(stderr, "'tdata_server.h' value is %ld\n",
	    (long) tdata_max_parents);
    fprintf(stderr, "Correct the entry in 'tdata_server.h'\n");
    return (-1);

  }

  tdata_max_children = TDATA_MAX_CHILDREN;
  track_max_children = MAX_CHILDREN;

  if (tdata_max_children != track_max_children) {

    fprintf(stderr, "Mismatch in header files for MAX_CHILDREN\n");
    fprintf(stderr, "'tracks.h' value is %ld\n", (long) track_max_children);
    fprintf(stderr, "'tdata_server.h' value is %ld\n",
	    (long) tdata_max_children);
    fprintf(stderr, "Correct the entry in 'tdata_server.h'\n");
    return (-1);

  }

  /*
   * turn process into a daemon
   */
  
  if (Glob->params.debug < DEBUG_EXTRA) {
    udaemonize();
  }
  
  /*
   * register functions to trap termination and interrupts
   */

  PORTsignal(SIGQUIT, (void (*)())tidy_and_exit);
  PORTsignal(SIGTERM, (void (*)())tidy_and_exit);
  PORTsignal(SIGINT, (void (*)())tidy_and_exit);
  PORTsignal(SIGHUP, (void (*)())tidy_and_exit);
  PORTsignal(SIGPIPE, (void (*)())SIG_IGN);

  if (!Glob->params.debug) {
    
    PORTsignal(SIGILL, (void (*)())tidy_and_exit);
    PORTsignal(SIGFPE, (void (*)())tidy_and_exit);
    PORTsignal(SIGTRAP, (void (*)())tidy_and_exit);
    PORTsignal(SIGSEGV, (void (*)())tidy_and_exit);

  }

  /*
   * open prototype server socket
   */

  if((protofd = SKU_open_server(Glob->params.port)) < 0) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Couldn't Begin Socket Operations\n");
    fprintf(stderr, "Port %ld\n", Glob->params.port);
    return(-1);
  }

  /*
   * set up message queue for children to communicate with parent
   * after each data request
   */
  
  Glob->msq_key = Glob->params.port;
  
  if((Glob->msq_id = umsg_create(Glob->msq_key, 0666)) < 0) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Couldn't create message queue\n");
    fprintf(stderr, "Key (based on port number) %d\n",
	    (int) Glob->msq_key);
    return(-1);
  }

  /*
   * set wait time for accept to time out
   */

  wait_msecs = 2000;

  /*
   * register this server
   */
      
  SMU_auto_register();
  PMU_auto_register("Initial registration");
      
  /*
   * loop waiting for connection
   */

  while (forever) {
    
    while ((sockfd = SKU_get_client_timed(protofd, wait_msecs)) < 0) {
      
      /*
       * register this server and process
       */
      
      SMU_auto_register();
      PMU_auto_register("Waiting for clients");

      /*
       * clean up after any children which have died
       */
      
      while ((dead_pid = waitpid((pid_t) -1,
				 (int *) NULL,
				 (int) (WNOHANG | WUNTRACED))) > 0) {

	if (Glob->params.debug >= DEBUG_NORM)
	  fprintf(stderr, "Child %d died\n", dead_pid);

	for (i = 0; i < Nchildren; i++) {
	  
	  if (dead_pid == Pid[i]) {
	    
	    for (j = i; j < Nchildren - 1; j++) {
	      
	      Pid[j] = Pid[j + 1];
	      
	    } /* j */
	    
	  } /* if (dead_pid = Pid[i]) */
	  
	} /* i */
	
	Nchildren--;

	if (Glob->params.debug >= DEBUG_NORM)
	  fprintf(stderr, "Number of children reduced to %d\n", Nchildren);
	
      } /* while ((dead_pid ... */

      /*
       * read the message queue to get info from children
       * for registration
       */

      read_queue();
      
    } /* while ((sockfd ... */
    
    /*
     * fork
     */

    if (Glob->params.debug >= DEBUG_EXTRA) {
    
      /*
       * debugger only - set to 1 insetad of -1
       * that allows you to debug what is normally in the 
       * child area
       */

      /*
       * child
       */

      Parent = FALSE;
      Nchildren = 0;

      /*
       * perform the server function
       */
      
      server_main(sockfd);
      SKU_close(sockfd);
      return (0);
      
    } else if ((Pid[Nchildren] = fork()) == 0) {

      /*
       * child
       */

      Parent = FALSE;
      Nchildren = 0;

      /*
       * perform the server function
       */

      server_main(sockfd);
      SKU_close(sockfd);
      return (0);
      
    } else {
      
      /*
       * parent
       */
      
      Nchildren++;
      SKU_close_no_hangup(sockfd);
      
    } /* if ((Pid = fork()) == 0) */

    /*
     * register this server with the server mapper - makes sure
     * the server is registered each time a new connection is made
     * in addition to the usual interval
     */
      
    SMU_auto_register();
      
  } /* while (forever) */

  return (0);
    
}

/***************************************************************************
 * tidy_and_exit()
 *
 * tidies up and quits - makes sure all children are terminated
 *
 ****************************************************************************/

void tidy_and_exit(int sig)

{

  int i;

  if (Parent) {

    /*
     * hang up all children
     */

    for (i = 0; i < Nchildren; i++)
      kill(Pid[i], SIGHUP);

    /*
     * unregister server
     */

    unregister_server();
    PMU_auto_unregister();

    /*
     * remove message queue
     */

    umsg_remove(Glob->msq_key);

  } else {

    /*
     * child
     */

  } /* if (Parent) */

  /*
   * dump core if required
   */

  if (sig == SIGSEGV || sig == SIGILL || sig == SIGFPE ||
      sig == SIGTRAP) {
    abort();
  }

  /*
   * exit with code sig
   */

  if (Glob->params.debug >= DEBUG_NORM)
    fprintf(stderr, "Exiting on signal %d\n", sig);

  exit(sig);

}

